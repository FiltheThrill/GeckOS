/* keyboard.c - Functions to handle terminal io and typing
Author - Ian Goodwin
https://wiki.osdev.org/PS2_Keyboard
 */

#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
//globals
//static vars
//lowercase translational table
static char scancode_lowercase[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//uppercase translational table
static char scancode_shift[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//special case translational table
static char scancode_shiftcaps[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//dynamic vars
unsigned int term;                   //global for current terminal 0,(1,2,3,4 for multiterm)
unsigned int cmd_len[TNUM];          //global for each command length
char cmd_buf[TNUM][BUFMAX];          //global cmd store for any terminal
unsigned int cursorX[TNUM];          //cursor x val
unsigned int cursorY[TNUM];          //cursor y val(line num)
unsigned int cmdX[TNUM];             //Xpos of command
unsigned int cmdY[TNUM];             //Ypos of command
unsigned int cursor[TNUM];           //cursor cnt/iterator
//[x,x,x,x,alt,caps,ctrl,shift]
unsigned char ops;                   //char to store modifiers
//boolean using n or y
unsigned char termRead[TNUM];        //allow typed term read
//extra credit!!
char cmd_hist[TNUM][HNUM][BUFMAX];   //allows for cmd history up to a cnt
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
  int i;
  int j;
  for(i=0;i<TNUM;i++){
    termRead[i] = 'n';
    cmd_len[i] = 0;
    cursorX[i] = 0;
    cursorY[i] = 0;
    cmdX[i] = 0;
    cmdY[i] = 0;
    cursor[i] = 0;
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

//handles system terminal writes
int32_t term_write(int32_t fd, const void * buf, int32_t nbytes){
  int bytecnt;
  unsigned int idx;
  idx = fetch_process();
  bytecnt = 0;
  while(bytecnt < nbytes && !EOF){
    putc(buf[idx][i],term);
    bytecnt++;
  }
  return bytecnt;
}

//handles system terminal reads, deletes typed cmd after use
int32_t term_read(int32_t fd, void * buf, int32_t nbytes){
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
  term_clear();
  while(i<nbytes && c != 0){
    c = cmd_buf[idx][i];
    buf[i] = c;
    cmd_buf[idx][i] = 0;
    i++;
  }
  //reset vals
  cursor[idx] = 0;
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
  unsigned char c;        //char from array
  int i;                  //loop int

  cli();                  //disable interrupts for now
  asm volatile(
    "pushal\n"
    :
    :);
  stat = inb(STARTREG);
  //take all from keyboard buffer
  while(ANDFULL & stat){
    scan = inb(DATAPORT);          //get data from key press
    parse_input(scan);             //parse the input
    //update cursor pos
    cursorX[term] = cmdX[term];
    cursorY[term] = cmdY[term];
    cursor[term] = 0;
    i=0;
    while(i<cmd_len[term]){        //print whole keyboard buffer
      putc(cmd_buf[term][i],term);
      i++;
    }
    stat = inb(STARTREG);          //eval buffer
  }

  sti();                      // reenable interrupts
  send_eoi(1);                //send end of interrupt signal to irq2
  asm volatile(
    "popal\n"
    :
    :);
  asm("leave");
  asm("iret");            // interrupt return
}

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

void insert_char(char c, int idx){
  char temp;
  int len;

  len = cmd_len[term];
  cmd_len[term]++;
  cursor[term]++;
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

//func to convert to char
char convert_scancode(uint8_t scancode){
  char c = check_flags(scancode) & 0x00FF;
  return c;
}

uint16_t check_flags(uint8_t scancode){
  uint16_t c;
  c = 0;
  //[x,x,x,x,alt,caps,ctrl,shift]
  switch(ops)
  {
    case 0:        //no flags
      c = scancode_lowercase[scancode];
      return c;
    case 1:        //shift only
      c = scancode_shift[scancode];
      return c;
    case 2:       //ctrl only
      c = 0x0200;
      return c;
    case 3:       //shift and ctrl
      c = 0x0300;
      return c;
    case 4:       //just caps
      c = scancode_caps[scancode];
      return c;
    case 5:       //caps and shift
      c = scancode_shiftcaps[scancode];
      return c;
    case 6:       //caps and ctrl
      c = 0x0600;
      return c;
    case 7:       //caps, ctrl, shift
      c = 0x0700;
      return c;
    case 8:       //alt only
      c = 0x0800;
      return c;
    case 9:       //alt and shift
      c = 0x0900;
      return c;
    case 10:      //alt and ctrl
      c = 0x0A00;
      return c;
    case 11:      //alt, ctrl, and shift
      c = 0x0B00;
      return c;
    case 12:      //alt and caps
      c = 0x0C00;
      return c;
    case 13:      //alt, ctrl, and shift
      c = 0x0D00;
      return c;
    case 14:      //alt caps, and ctrl
      c = 0x0E00;
      return c;
    case 15:      //alt, caps, ctrl, and shift
      c = 0x0F00;
      return c;
    default:
      return 0;
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

void term_clear(){
  return;
}
