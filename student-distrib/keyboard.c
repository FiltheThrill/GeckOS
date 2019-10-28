/* keyboard.c - Functions to handle terminal io and typing
Author - Ian Goodwin
//resources used
https://wiki.osdev.org/PS2_Keyboard
https://wiki.osdev.org/Text_Mode_Cursor

*/

#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "types.h"
#include "terminal.h"
//globals
//static vars
//lowercase translational table with numbers
static char scancode_lower[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0x08, 0x09, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '[', ']', 0x0D, 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//uppercase translational table with special chars
static char scancode_upper[] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
                            '_', '+', 0x08, 0x09, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
                            'I', 'O', 'P', '{', '}', 0x0D, 0, 'A', 'S', 'D', 'F','G',
                            'H', 'J', 'K', 'L', ':', 0x27, 0x60, 0, 0x5c, 'Z', 'X', 'C',
                            'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//translational table for caps case
static char scancode_caps[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0x08, 0x09, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
                            'I', 'O', 'P', '[', ']', 0x0D, 0, 'A', 'S', 'D', 'F','G',
                            'H', 'J', 'K', 'L', ';', 0x27, 0x60, 0, 0x5c, 'Z', 'X', 'C',
                            'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//prompt to display in term
static char prompt[CURSOROFF] = {'[','t','e','r','m',']',':',' '};
//dynamic vars (made volatile to survive running)
//all keyboard uses for each possible terminal are stored here
volatile unsigned int term;                   //global for current terminal 0,(1,2,3,4 for multiterm)
//[ins,alt,caps,ctrl,shift]
volatile unsigned char ops[OPNUM];            //char arr to store modifiers
volatile unsigned int cursoff[TNUM];          //used to add terminal prompt
volatile unsigned int cmd_len[TNUM];          //global for each command length
volatile char cmd_buf[TNUM][BUFMAX];          //global cmd store for any terminal
volatile unsigned int cursorX[TNUM];          //cursor x val
volatile unsigned int cursorY[TNUM];          //cursor y val(line num)
volatile uint8_t attr[TNUM];                  //attributes for console
volatile uint32_t address[TNUM];               //vid mem pointers
//boolean using n or y
volatile unsigned char termRead[TNUM];        //allow typed term read
//extra credit!!
volatile unsigned int histnum[TNUM];          //cnt for place in hist
volatile char cmd_hist[TNUM][HISTNUM][BUFMAX];   //allows for cmd history up to a cnt
/*
* keyboard_init
*   DESCRIPTION: enables irq1 to allow keyboard interrupts
*     also sets up the global vars for keyboard use
*   INPUTS: none
*   OUTPUTS: inserts default values to all of the global vars
*   RETURN VALUE: none
*   SIDE EFFECTS: none
*/
void keyboard_init(){
  //no protect needed, interrupt not enabled
  //set global trackers and commands
  //have to init_terminal first in order to run this
  int i,j,k;
  for(i=0;i<TNUM;i++){
    termRead[i] = 'n';
    cmd_len[i] = 0;
    cursorX[i] = 0;
    cursorY[i] = 0;
    histnum[i] = 0;
    cursoff[i] = CURSOROFF;
    //change for color?
    attr[i] = 0x07;
    address[i] = 0;//term_addr(i);
  }
  //set global cmd bufs
  for(i=0;i<TNUM;i++){
    for(j=0;j<BUFMAX;j++){
      //fill with non vals
      cmd_buf[i][j] = 0;
    }
  }
  //fill ops
  for(i=0;i<5;i++){
    ops[i] = 'n';
  }
  //set global hist
  for(i=0;i<TNUM;i++){
    for(j=0;j<HISTNUM;j++){
      for(k=0;k<BUFMAX;k++){
        cmd_hist[i][j][k] = 0;
      }
    }
  }
  //set term num to first term (0)
  term = 0;
  //enable req and clear the screen
  enable_irq(1);
  return;
}
//update the current terminal
//pulls this data from term init
void update_term(unsigned int t){
  term = t;
}
/*
* term_write
*   DESCRIPTION: handles system terminal writes, uses the provided
*     convention of fd, buffer, and nbytes to print to the screen
*   INPUTS: file descriptor int, pointer to data buffer, nbytes to write
*   OUTPUTS: none
*   RETURN VALUE: number of bytes written (can vary based on screen size)
*   SIDE EFFECTS: clears the screen
*/
int32_t term_write(int32_t fd, const uint8_t * buf, int32_t nbytes){
  int bytecnt;
  unsigned int idx;
  idx = fetch_process();
  bytecnt = 0;
  //no offset for term writes
  cursoff[idx] = 0;
  term_clear(idx,0);
  //kill if excedes term buffer size
  while(bytecnt < nbytes && bytecnt < XMAX * YMAX){
    term_putc(idx,buf[bytecnt]);
    cursorX[term]++;
    bytecnt++;
  }
  cursoff[idx] = CURSOROFF;
  return bytecnt;
}

//handles system terminal reads, deletes typed cmd after use
/*
* term_reaad
*   DESCRIPTION: takes the values from the cmd_buf global in keyboard and
*     copies this to the provided buffer based on amount of bytes to read
*   INPUTS: file descriptor int, pointer to the buf to fill, nbytes to write
*   OUTPUTS: none
*   RETURN VALUE: returns the amount of bytes written
*   SIDE EFFECTS: writes history if desired, clears screen
*/
int32_t term_read(int32_t fd, uint8_t * buf, int32_t nbytes){
  //start at the cmd
  unsigned int idx = fetch_process();
  int32_t i,j;
  char c;
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
  c = cmd_buf[idx][0];
  i = 0;
  //pull if inside buffer and set values
  while(i<nbytes && c != 0 && i<BUFMAX){
    c = cmd_buf[idx][i];
    buf[i] = c;
    i++;
  }
  //clean up and read buffer
  term_clear(term,0);
  //clean up buffer
  for(j=0;j<BUFMAX;j++){
    cmd_buf[idx][j] = 0;
  }
  //reset vals, ret bytes read
  cmd_len[idx] = 0;
  termRead[idx] = 'n';
  return i;
}
/*
* keyboard_handler
*   DESCRIPTION: this function handles the keybaord interupts based on the PIC
*     it gets the scan code, parses the input, and updates the screen and
*     command buffer as neccesary
*   INPUTS: none
*   OUTPUTS: outputs chars to the cmd buffer, the screen, and moves the cursor
*     on the screen as desired based on the key pressed
*   RETURN VALUE: none
*   SIDE EFFECTS: can call basically all of the keybaord functions, so can
*     initiate most terminal actions and display actions
*/
void keyboard_handler(){
  uint8_t scan;           //scancode
  int i;                  //loop int

  cli();                  //disable interrupts for now
  asm volatile(
    "pushal\n"
    :
    :);
  scan = inb(DATAPORT);          //get data from key press
  i = parse_input(scan);         //parse the input
  switch (i)
  {
    case 0:  //write new char
      term_putc(term,cmd_buf[term][cmd_len[term]-1]);
      cursorX[term]++;
      move_cursor(term);
      break;
    case 1:  //no new char write, handled in cases
      break;
    case 2:  //stop char encountered, handle
      break;
    case 3:  //debug case
      putc('Q');
      break;
    default:
      break;
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
/*
* move_cursor
*   DESCRIPTION: moves the cursor forward based on coords globals
*   INPUTS: terminal number to move the cursor on
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: moves the cursor onn the display based on the formula to move it
*/
void move_cursor(unsigned int t){
  validate_cursor(t);
  uint16_t loc = cursorY[t] * XMAX;     //iterate the pos down rows
  loc = loc + cursorX[t];               //add in x vals
  uint8_t writeL = loc&0x00FF;          //mask all bits outside of char val
  uint8_t writeH = loc&0xFF00;
  writeH = writeH>>BYTE;
  //write low bits
  outb(CURSORLB,CURSORLA);
  outb(writeL,CURSORHA);
  //write high bits
  outb(CURSORHB,CURSORLA);
  outb(writeH,CURSORHA);
  return;
}
/*
* validate_cursor
*   DESCRIPTION: validates the cursor location global vars. The x is iterated past
*     the end in alot of cases, this fixes the coordinate scheme as needed
*   INPUTS: terminal to operate on
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: changes the values of the cursor globals
*/
void validate_cursor(uint8_t t){
  //fix 2d coordinate scheme
  while(cursorX[t] > XMAX){
    cursorY[t]++;
    cursorX[t] = cursorX[t] - XMAX;
  }
  //dont overwrite prompt
  if(cursorX[t] < cursoff[t] && cmd_len[t] < 6){
    cursorX[t] = cursoff[t];
  }
  //end of screen
  if(cursorY[t] * XMAX + cursorX[t] > XMAX*YMAX){
    cursorY[t] = YMAX;
    cursorX[t] = XMAX;
    return;
  }
}
//ret 0 for char wrote, 1 for non char write input, 2 for stop input
/*
* parse_input
*   DESCRIPTION: uses many helper functions to take a scancode and perform the
*     neccesary operations on it. Will add the char to the buffer if it is desired
*     also could update the special ops or chars
*   INPUTS: scancode
*   OUTPUTS: none
*   RETURN VALUE: int; 0 for char wrote, 1 for non char write input, 2 for stop input
*   SIDE EFFECTS: could write to buffer or update ops
*/
int parse_input(uint8_t scancode){
  int ret;
  char c;

  //update the ops
  ret = update_ops(scancode);
  if(ret == 1){
    //was a flag, ret
    return ret;
  }
  //handle media key input
  ret = process_media(scancode);
  if(ret == 1 || ret == 2){
    return ret;
  }
  //must be in the char array
  c = generate_char(scancode);
  if(c == 0){
    //didnot generate useful char
    ret = 1;
    return ret;
  }
  ret = process_char(c);
  if(ret == 1 || ret == 2 || ret == 3){
    //was a special char, handled
    return ret;
  }
  //needs to be written, insert to build buf
  return insert_char(c,cmd_len[term]);
}
/*
* process_char
*   DESCRIPTION: takes a char val and uses it to do any needed commands
*     based on ops (ex: CTRL+L)
*   INPUTS: char to process
*   OUTPUTS: none
*   RETURN VALUE: 0 for no combo found, 1 for action taken
*   SIDE EFFECTS: can move the cursor or allow term read
*/
int process_char(char c){
  int i;
  switch(c)
  {
    case 13:  //enter
      termRead[term] = 'y';
      return 2;
    case 8: //backspace
      if(cursorX[term] > cursoff[term]){
        cursorX[term]--;
        cmd_len[term]--;
        cmd_buf[term][cmd_len[term]]=0;
        term_putc(term,0);
        move_cursor(term);
      }
      return 1;
    case 108:  //CTRL + l
      if(ops[1] == 'y'){
        term_clear(term,0);
        return 1;
      }
      break;
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
/*
* insert_char
*   DESCRIPTION: inserts char to build buffer. This can be done at any index
*     with the buffer being adjusted accordingly
*   INPUTS: character to insert, index to insert at
*   OUTPUTS: none
*   RETURN VALUE: returns 1 if no insert, 0 if inserted
*   SIDE EFFECTS: will adjust the cmd_buf and the lenght of the cmd
*/
int insert_char(char c, int idx){
  char temp;
  int i;
  if(idx < 0){
    return 1;
  }
  //do nothing if buf overflow
  if(idx >= BUFMAX-2 || cmd_len[term] > BUFMAX-1){
    return 1;
  }
  //check for invalid insert idx
  if(idx > cmd_len[term]){
    return 1;
  }
  //insert if toggled on
  if(ops[4] == 'y'){
    cmd_buf[term][idx] = c;
    term_putc(term,idx);
    return 1;
  }
  //apped char at end
  if(idx == cmd_len[term]){
    cmd_buf[term][cmd_len[term]] = c;
    cmd_len[term]++;
    return 0;
  }
  //insert is inside buf, shift the content after up by 1
  i = cmd_len[term];
  i++;
  while(idx-cursoff[term] < i){
    temp = cmd_buf[term][i-1];
    cmd_buf[term][i] = temp;
    i--;
  }
  cmd_buf[term][idx] = c;
  cmd_len[term]++;
  return 0;
}
/*
* process_media
*   DESCRIPTION: handles media key presses (num pad and arrows mainly)
*   INPUTS: scancode of pressed key
*   OUTPUTS: none
*   RETURN VALUE: returns 1 if media key was handled, 0 if not
*   SIDE EFFECTS: can move the cursor or clear the screen in some cases
*/
int process_media(uint8_t scancode){
  int i;
  switch(scancode)
  {
    case 0x48:  //up arrow
      if(USEHIST == 1){
        history_fetch(term);
        i=0;
        term_clear(term,1);
        move_cursor(term);
        return 2;
      }
      return 1;
    case 0x04B: //left arrow
      if(cursorX[term] > cursoff[term] && cmd_len[term] > 0){
        cursorX[term]--;
        move_cursor(term);
      }
      return 1;
    case 0x04D: //right arrow
      if(cursorX[term] < BUFMAX && cmd_buf[term][cursorX[term]-cursoff[term]] != 0){
        cursorX[term]++;
        move_cursor(term);
      }
      return 1;
    default:
      return 0;
  }
}
/*
* update_ops
*   DESCRIPTION: updates the op code array for pressed control keys
*     things like shift, caps, ctrl...
*     current srtucture : [ins,alt,caps,ctrl,shift]
*   INPUTS: scancode of pressed key
*   OUTPUTS: none
*   RETURN VALUE: returns 1 if opkey used, 0 otherwise
*   SIDE EFFECTS: changes the status of the ops array based on presses
*/
int update_ops(uint8_t scancode){
  switch(scancode)
  {
  //pressed cases
    case 0x36: //right shift
      ops[0] = 'y';
      return 1;
    case 0x2A: //left shift
      ops[0] = 'y';
      return 1;
    case 0x1D: //ctrl
      ops[1] = 'y';
      return 1;
    case 0x3A: //caps (toggle)
      if(ops[2] == 'y'){
        ops[2] = 'n';
        return 1;
      }
      ops[2] = 'y';
      return 1;
    case 0x38: //alt
      ops[3] = 'y';
      return 1;
    case 0x52: //ins (toggle)
      if(ops[4] == 'y'){
        ops[4] = 'n';
        return 1;
      }
      ops[4] = 'y';
      return 1;
  //released cased
    case 0xB6: //right shift
      ops[0] = 'n';
      return 1;
    case 0xAA: //left shift
      ops[0] = 'n';
      return 1;
    case 0x9D: //ctrl
      ops[1] = 'n';
      return 1;
    case 0xB9: //alt
      ops[3] = 'n';
      return 1;
    default:   //no ops
      return 0;
  }
}
/*
* generate_char
*   DESCRIPTION: picks correct char based on flags, will
*     return in bottom bits with flags.
*   INPUTS: scancode of pressed key
*   OUTPUTS: none
*   RETURN VALUE: char that is generated from the arrays based on scancode,
*     otherwise 0 for no char generated (flags affect generation)
*   SIDE EFFECTS: none
*/
char generate_char(uint8_t scancode){
  char c;
  unsigned int full;
  int i;
  static unsigned int pow[7] = {1,2,4,8,16,32,64};
  c = 0; //will ret if no char made
  full = 0;
  //sum for switch
  for(i=0;i<OPNUM;i++){
    if(ops[i] == 'y'){
      full = full + pow[i];
    }
  }
  //make sure not out of bounds
  if(scancode > 0x53){
    return c;
  }
  switch(full)
  {
    case 0:        //no flags
      c = scancode_lower[scancode];
      return c;
    case 1:        //shift only
      c = scancode_upper[scancode];
      return c;
    case 2:       //ctrl only
      c = scancode_lower[scancode];
      return c;
    case 3:       //shift and ctrl
      return c;
    case 4:       //just caps
      c = scancode_caps[scancode];
      return c;
    case 5:       //caps and shift (fix later)
      c = scancode_upper[scancode];
      return c;
    case 6:       //caps and ctrl
      c = scancode_caps[scancode];
      return c;
    case 7:       //caps, ctrl, shift
      return c;
    case 8:       //alt only
      c = scancode_lower[scancode];
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
/*
* history_fetch
*   DESCRIPTION: change over current term buffer to history and displays
*   INPUTS: idx of the buffer to fetch from
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: swaps in the buffered command from history
*/
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
/*
* history_write
*   DESCRIPTION: write the entered command to history buffer array
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: updates the history buffer array for later used if desired
*/
void history_write(){
  int i,j;
  if(USEHIST == 0){
    //do nothing
    return;
  }
  for(i=0;i<HISTNUM-1;i++){
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
//this returns 0 for now but will be useful for multi terms
unsigned int fetch_process(){
  unsigned int id;

  id = term;
  if(id<0 || id>TNUM){      //invalid process, make guess
    return term;
  }
  //return the term for now
  return id;
}
//
/*
* term_putc
*   DESCRIPTION: puts char at loc based on given cursor loc. works much
*     like the given putc, but has more flexible addressign and indexing
*     based on the cursor loc and write addresses
*   INPUTS: term number, char
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: will add a char in the idex of the memory location desired
*/
void term_putc(unsigned int t, uint8_t c){
  //do same as putc
  if(USEPAGE == 0){
    //use standard vmem address
    address[t] = 0xB8000;
  }
  if(cursorY[t] * XMAX + cursorX[t] > XMAX*YMAX){
    return;
  }
  *(uint8_t *)(address[t] + ((XMAX * cursorY[t] + cursorX[t]) << 1)) = c;
  *(uint8_t *)(address[t] + ((XMAX * cursorY[t] + cursorX[t]) << 1) + 1) = attr[t];
  return;
}
/*
* term_clear
*   DESCRIPTION: clear and reset the terminal for other uses. can either write
*     just the prompt or write the whole buffer back to screen based on op
*   INPUTS: term number, operation condidions to perform
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: will clear the screen or the screen memory of the address chosen
*     can also write to this memory after clear
*/
void term_clear(unsigned int t, int op){
  int cnt, i;

  cursorX[t] = 0;
  cursorY[t] = 0;
  //do reg clear if not paging
  if(USEPAGE == 0){
    clear();
  }
  //modify given clear, blank vid mem
  else{
    for (i = 0; i < YMAX * XMAX; i++) {
        *(uint8_t *)(address[t] + (i << 1)) = ' ';
        *(uint8_t *)(address[t] + (i << 1) + 1) = attr[t];
    }
  }
  //print prompt
  for(i=0;i<cursoff[term];i++){
    term_putc(t,prompt[i]);
    cursorX[t]++;
  }
  //clear buffer as well
  if(op == 0){
    for(i=0;i<BUFMAX;i++){
      cmd_buf[t][i] = 0;
    }
    cmd_len[t]=0;
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
    cursorX[t]++;
  }
  move_cursor(t);
  return;
}
