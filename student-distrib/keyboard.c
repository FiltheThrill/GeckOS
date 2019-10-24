/* keyboard.c - Functions to handle terminal io and typing
Author - Ian Goodwin
https://wiki.osdev.org/PS2_Keyboard
*/

#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"
#include "terminal.h"
//globals
//static vars
//lowercase translational table with numbers
static char scancode_lowercase[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//uppercase translational table with special chars
static char scancode_shift[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//uppercase translational table with numbers
static char scancode_caps[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//lowercase translational table with special chars
static char scancode_shiftcaps[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//dynamic vars (made volatile to survive running)
//all keyboard uses for each possible terminal are stored here
volatile unsigned int term;                   //global for current terminal 0,(1,2,3,4 for multiterm)
volatile unsigned int cmd_len[TNUM];          //global for each command length
volatile char cmd_buf[TNUM][BUFMAX];          //global cmd store for any terminal
volatile unsigned int cursorX[TNUM];          //cursor x val
volatile unsigned int cursorY[TNUM];          //cursor y val(line num)
volatile unsigned int cmdX[TNUM];             //Xpos of command
volatile unsigned int cmdY[TNUM];             //Ypos of command
volatile uint8_t attr[TNUM];                  //attributes for console
volatile uint8_t address[TNUM];              //vid mem pointers
//[x,x,x,x,alt,caps,ctrl,shift]
volatile unsigned char ops;                   //char to store modifiers
//boolean using n or y
volatile unsigned char termRead[TNUM];        //allow typed term read
//extra credit!!
volatile char cmd_hist[TNUM][HNUM][BUFMAX];   //allows for cmd history up to a cnt
/*
* keyboard_init
*   DESCRIPTION: enables irq1 to allow keyboard interrupts
*     also sets up the global vars for keyboard use
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: clears screen
*/
void keyboard_init(){
  //no protect needed, interrupt not enabled
  //set global trackers and commands
  //have to init_terminal first in order to run this
  int i;
  int j;
  for(i=0;i<TNUM;i++){
    termRead[i] = 'n';
    cmd_len[i] = 0;
    cursorX[i] = 0;
    cursorY[i] = 0;
    cmdX[i] = 0;
    cmdY[i] = 0;
    //change for color?
    attr[i] = 0x07;
    address[i] = term_addr(i);
  }
  //set global cmd bufs
  for(i=0;i<TNUM;i++){
    for(j=0;j<BUFMAX;j++){
      //fill with non vals
      cmd_buf[i][j] = 0;
    }
  }
  //set term num to first term (0)
  term = 0;
  ops = 0;
  //enable req and clear the screen
  enable_irq(1);
  clear();
  return;
}
//update the current terminal
void update_term(unsigned int t){
  term = t;
}
//handles system terminal writes
int32_t term_write(int32_t fd, const char * buf, int32_t nbytes){
  int bytecnt;
  int check;
  unsigned int idx;
  idx = fetch_process();
  bytecnt = 0;
  check = 0;
  //kill if excedes term buffer size
  while(bytecnt < nbytes && bytecnt < BUFMAX){
    term_putc(idx,buf[bytecnt]);
    bytecnt++;
  }
  return bytecnt;
}

//handles system terminal reads, deletes typed cmd after use
int32_t term_read(int32_t fd, char * buf, int32_t nbytes){
  //start at the cmd
  unsigned int idx = fetch_process();
  int32_t i;
  char c;
  //set cmdloc to cursor
  cmdX[idx] = cursorX[idx];
  cmdY[idx] = cursorY[idx];
  //wait for command completion (spam?)
  while(1){
    if(termRead[idx] == 'y'){
      break;
    }
  }
  //save entered command
  if(USEHIST == 1){
    history_write();
  }
  //clean up and read
  c = cmd_buf[idx][0];
  i = 0;
  term_clear(term,0);
  while(i<nbytes && c != 0){
    c = cmd_buf[idx][i];
    buf[i] = c;
    cmd_buf[idx][i] = 0;
    i++;
  }
  //reset vals
  cmd_len[idx] = 0;
  termRead[idx] = 'n';
  return i;
}
/*
* keyboard_handler
*   DESCRIPTION: putc a lowercase character or number to screen from keyboard
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: echo's keypress to screen
*/
void keyboard_handler(){
  uint8_t scan;           //scancode
  uint8_t stat;           //statcode
  int i;                  //loop int

  cli();                  //disable interrupts for now
  asm volatile(
    "pushal\n"
    :
    :);
  stat = inb(STATREG);
  //take all from keyboard buffer
  //loop allows for fast typing to fill keyboard buffer
  while(ANDFULL & stat){
    scan = inb(DATAPORT);          //get data from key press
    parse_input(scan);             //parse the input
    //update cursor pos
    cursorX[term] = cmdX[term];
    cursorY[term] = cmdY[term];
    i=0;
    while(i<cmd_len[term]){        //print whole keyboard buffer
      term_putc(term,cmd_buf[term][i]);
      i++;
    }
    stat = inb(STATREG);           //eval buffer
  }

  sti();                           // reenable interrupts
  send_eoi(1);                     //send end of interrupt signal to irq2
  asm volatile(
    "popal\n"
    :
    :);
  asm("leave");
  asm("iret");                     // interrupt return
}

//moves the cursor forward by arg
void move_cursor(unsigned int cursor){
  uint32_t loc = cmdY[term] * XMAX; //iterate the pos down rows
  loc = loc + cmdX[term] + cursor;      //add in x vals plus added chars
  unsigned char write = loc&0xFF;       //mask all bits outside of char val
  //write new low cursor
  outb(CURSORLOW,CURSORLOC);
  outb(write,CURSORLOC);
  //shift bits and write new high cursor
  write = loc;
  write = write>>BYTE;
  write = write&0xFF;
  outb(CURSORHIGH,CURSORLOC);
  outb(write,CURSORLOC);
}

int parse_input(uint8_t scancode){
  int ret;
  char c;
  //update the ops
  ret = update_ops(scancode);
  if(ret == 1){
    //was a flag, ret
    return ret;
  }
  c = generate_char(scancode);
  ret = process_char(c);
  if(ret == 1){
    //was a special char, handled
    return ret;
  }
  //needs to be written, insert
  insert_char(c,cursorX[term]);
  return ret;
}
int process_char(char c){
  int i;
  switch(c)
  {
    case 13:  //enter
      termRead[term] = 'y';
      return 1;
    case 127: //delete
      cursorX[term]--;
      cmd_buf[term][cmd_len[term]-1]=0;
      cmd_len[term]--;
      return 1;
    case 8: //backspace
      cursorX[term]--;
      cmd_buf[term][cmd_len[term]-1]=0;
      cmd_len[term]--;
      return 1;
    default:
      break;
  }
  //check for useless non chars
  for(i=0;i<32;i++){
    if(c == i){
      return 1;
    }
  }
  //is a printable char
  return 0;
}
void insert_char(char c, int idx){
  char temp;
  int len;

  len = cmd_len[term];
  cmd_len[term]++;
  cursorX[term]++;
  //push back the rest of the cmd buf
  idx++;
  while(idx < len){
    temp = cmd_buf[term][len];
    len--;
    cmd_buf[term][len] = temp;
  }
  idx--;
  cmd_buf[term][idx] = c;
}

//func to update the ops based on scancode
//[x,x,x,x,alt,caps,ctrl,shift]
int update_ops(uint8_t scancode){
  //check for indicator of media keys
  //next value will be the actual key
  if(scancode == 0xE0){
    scancode = inb(DATAPORT);
  }
  switch(scancode)
  {
  //pressed cases
    case 0x36: //right shift
      ops = ops | 0x01;
      return 1;
    case 0x2A: //left shift
      ops = ops | 0x01;
      return 1;
    case 0x1D: //ctrl
      ops = ops | 0x02;
      return 1;
    case 0x3A: //caps
      ops = ops | 0x04;
      return 1;
    case 0x38: //alt
      ops = ops | 0x08;
      return 1;
  //released cased
    case 0xB6: //right shift
      ops = ops & 0x0E;
      return 1;
    case 0xAA: //left shift
      ops = ops & 0x0E;
      return 1;
    case 0x9D: //ctrl
      ops = ops & 0x0D;
      return 1;
    case 0xBA: //caps
      ops = ops & 0x0B;
      return 1;
    case 0xB9: //alt
      ops = ops | 0x07;
      return 1;
    default:   //no ops
      return 0;
  }
}
//picks correct char based on flags, will return in bottom bits with flags
//returns 0 if no char is needed based on flags
char generate_char(uint8_t scancode){
  char c;

  c = 0; //filler val
  switch(ops)
  {
    case 0:        //no flags
      c = scancode_lowercase[scancode];
      return c;
    case 1:        //shift only
      c = scancode_shift[scancode];
      return c;
    case 2:       //ctrl only
      return c;
    case 3:       //shift and ctrl
      return c;
    case 4:       //just caps
      c = scancode_caps[scancode];
      return c;
    case 5:       //caps and shift
      c = scancode_shiftcaps[scancode];
      return c;
    case 6:       //caps and ctrl
      return c;
    case 7:       //caps, ctrl, shift
      return c;
    case 8:       //alt only
      return c;
    case 9:       //alt and shift
      return c;
    case 10:      //alt and ctrl
      return c;
    case 11:      //alt, ctrl, and shift
      return c;
    case 12:      //alt and caps
      return c;
    case 13:      //alt, ctrl, and shift
      return c;
    case 14:      //alt caps, and ctrl
      return c;
    case 15:      //alt, caps, ctrl, and shift
      return c;
    default:
      return c;
  }
}
//change over current term buffer to history and displays
void history_fetch(int idx){
  if(USEHIST == 0){
    //do nothing
    return;
  }
  unsigned int i, size;
  size = 0;
  //move command to active array and set helpers accordingly
  for(i=0;i<BUFMAX;i++){
    unsigned char c = cmd_hist[term][i][idx];
    cmd_buf[term][i] = c;
    if(c != 0){
      size++;
    }
  }
  cmd_len[term] = size;
}
//write the entered command to history
void history_write(){
  int i,j;
  if(USEHIST == 0){
    //do nothing
    return;
  }
  for(i=0;i<HNUM-1;i++){
    //dont waste effort on usless transfer
    if(cmd_hist[term][i][0] != 0){
      j=0;
      while(j<BUFMAX && cmd_hist[term][i][j] != 0){
        cmd_hist[term][i+1][j] = cmd_hist[term][i][j];
        j++;
      }
    }
  }
  //store most recent cmd
  for(i=0;i<BUFMAX;i++){
    cmd_hist[term][0][i] = cmd_buf[term][i];
  }
  return;
}
//function to get the process id in order to do sys calls
unsigned int fetch_process(){
  unsigned int id;

  id = term;
  if(id<0 || id>TNUM){      //invalid process, make guess
    return term;
  }
  //return the term for now
  return id;
}
extern void term_putc(unsigned int t, uint8_t c){
  //do same as putc
  if(USEPAGE == 0){
    //write like normal
    putc(c);
    return;
  }
  if(c == '\n' || c == '\r') {
      cursorY[t]++;
      cursorX[t] = 0;
  }
  else {
      *(uint8_t *)(address[t] + ((XMAX * cursorY[t] + cursorX[t]) << 1)) = c;
      *(uint8_t *)(address[t] + ((XMAX * cursorY[t] + cursorX[t]) << 1) + 1) = attr[t];
  }
  return;
}
//clear and reset the terminal for use
void term_clear(unsigned int t, int op){
  int cnt, i;

  cursorX[t] = 0;
  cursorY[t] = 0;
  //do reg clear if not paging
  if(USEPAGE == 0){
    clear();
  }
  //modify given clear
  else{
    for (i = 0; i < YMAX * XMAX; i++) {
        *(uint8_t *)(address[t] + (i << 1)) = ' ';
        *(uint8_t *)(address[t] + (i << 1) + 1) = attr[t];
    }
  }
  //caller chose to add buffer to screen
  if(op == 1){
    cnt = 0;
    while(cnt < BUFMAX && cmd_buf[t][cnt] != 0){
      term_putc(t,cmd_buf[t][cnt]);
      cursorX[t]++;
      if(cursorX[t] == XMAX){
        cursorX[t] = 0;
        cursorY[t]++;
      }
      cnt++;
    }
  }
  return;
}
