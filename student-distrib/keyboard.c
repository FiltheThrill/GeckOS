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
#include "syscalls.h"
//globals
//static vars
//lowercase translational table with numbers
static char scancode_lower[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0x08, 0x09, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '[', ']', 0x0D, 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0x3B,
                             0x3C, 0x3D, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};
//uppercase translational table with special chars
static char scancode_upper[] = {0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
                            '_', '+', 0x08, 0x09, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
                            'I', 'O', 'P', '{', '}', 0x0D, 0, 'A', 'S', 'D', 'F','G',
                            'H', 'J', 'K', 'L', ':', 0x27, 0x60, 0, 0x7c, 'Z', 'X', 'C',
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
//cmd prompt to display in term, also default one
static char prompt[CURSOROFF] = {'3','9','1','O','S','>'};
//dynamic vars (made volatile to survive running)
//all keyboard uses for each possible terminal are stored here
volatile unsigned int term;                      //global for current terminal 0,(1,2,3,4 for multiterm)
//[ins,alt,caps,ctrl,shift]
volatile unsigned char ops[OPNUM];               //char arr to store modifiers
volatile unsigned int cmd_len[TNUM];             //global for each command length
volatile char cmd_buf[TNUM][BUFMAX];             //global cmd store for any terminal
volatile int cmd_start[TNUM];                    //active cmd loc start addr relative to screen
volatile int cursorX[TNUM];                      //cursor x val
volatile int cursorY[TNUM];                      //cursor y val(line num)
volatile uint32_t address[TNUM];                 //usr vid mem pointers
//boolean using n or y
volatile unsigned char termRead[TNUM];           //allow typed term read
volatile char cmd_flag[TNUM];                    //flag to detect prompt writes
//extra credit!!
volatile int histidx;                            //index in the history buf
volatile char cmd_hist[HISTNUM][BUFMAX];         //allows for cmd history up to a cnt
volatile int cmd_hist_len[HISTNUM];              //stores the saved cmd len
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
  //have to init_terminal first in order to run this stuff
  int i,j,k;

  for(i=0;i<TNUM;i++){
    termRead[i] = 'n';
    cmd_len[i] = 0;
    cmd_flag[i] = 'n';
    cursorX[i] = 0;
    cursorY[i] = 0;
    cmd_start[i] = CURSORD;
    address[i] = term_addr(i);
  }
  //set global cmd bufs
  for(i=0;i<TNUM;i++){
    for(j=0;j<BUFMAX;j++){
      //fill with null
      cmd_buf[i][j] = NULLCHAR;
    }
  }
  //fill ops
  for(i=0;i<5;i++){
    ops[i] = 'n';
  }
  //set global hist
  for(j=0;j<HISTNUM;j++){
    for(k=0;k<BUFMAX;k++){
      cmd_hist[j][k] = NULLCHAR;
    }
    cmd_hist_len[i] = 0;
  }
  //set term num to first term (0)
  term = 0;
  histidx = -1;
  //clear vidmem
  clear();
  //enable req
  enable_irq(KEYIRQ);
  return;
}
//updates the current term in use
void update_term(unsigned int t){
  term = t;
  move_cursor(t);
  return;
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
int32_t term_write(int32_t fd, const void * buf, int32_t nbytes){
  int bytecnt, flag;
  unsigned int t,i;
  char put;
  //get current terminal
  t = fetch_process();
  //shorten to string size
  if(nbytes > strlen(buf)){
    nbytes = strlen(buf);
  }
  cli();
  bytecnt = 0;
  flag = 0;
  cmd_flag[t] = 'y';
  while(bytecnt < nbytes){
    //allow new lines (or not)
    if(*((uint8_t *) buf) == '\n'){
      bytecnt++;
      cursorY[t]++;
      cursorX[t] = 0;
      validate_cursor(t);
    }
    else{
      put = *((uint8_t *) buf);
      term_putc(t, put);
      bytecnt++;
      cursorX[t]++;
      if(*((uint8_t *) buf) == '>'){
        //found new cmd prompt, update start pos
        cmd_start[t] = bytecnt + cursorY[t]*XMAX;
        cmd_flag[t] = 'n';
      }
    }
    buf = ((uint8_t *) buf + 1);
  }
  //prompt found, validate
  if(cmd_flag[t] == 'n'){
    cursorX[t]--;
    move_cursor(t);
    //bad newln behind prompt, fix
    if(cursorX[t] > CURSORD){
      for(i=0; i<CURSORD; i++){
        cursorX[t]--;
        term_putc(t, NULLCHAR);
      }
      cursorY[t]++;
      cursorX[t] = 0;
      validate_cursor(t);
      for(i=0; i<CURSOROFF; i++){
        term_putc(t, prompt[i]);
        cursorX[t]++;
      }
      move_cursor(t);
      cmd_start[t] = cursorX[t] + cursorY[t]*XMAX;
    }
    sti();
  }
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
int32_t term_read(int32_t fd, void * buf, int32_t nbytes){
  unsigned int t;
  int32_t i,j;
  char c;

  //check for prev write with no prompt
  t = fetch_process();
  if(cmd_flag[t] == 'y'){
    cmd_start[t] = cursorX[t] + cursorY[t] * XMAX;
    move_cursor(t);
    cmd_flag[t] = 'n';
  }
  sti(); //enable interrupts while wait
  while(termRead[t] == 'n'){
    //spin while wait for read
  }
  cli();
  //save entered command
  if(USEHIST == 1){
    history_write();
  }
  c = cmd_buf[t][0];
  i = 0;
  //pull if inside buffer and set values
  while(i<nbytes && c != 0 && i<BUFMAX){
    c = cmd_buf[t][i];
    *((uint8_t *) buf) = c;
    buf = ((uint8_t *) buf + 1);
    i++;
  }
  //clean up buffer
  for(j=0;j<BUFMAX;j++){
    cmd_buf[t][j] = NULLCHAR;
  }
  //reset vals, ret bytes read
  cmd_len[t] = 0;
  termRead[t] = 'n';
  sti();
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
  int i;          //loop/switch int

  cli();                  //disable interrupts for now
  asm volatile(
    "pushal\n"
    :
    :);
  scan = inb(DATAPORT);          //get data from key press
  i = parse_input(scan);         //parse the input
  switch (i)
  {
    case -1: //added char is inside buf, reprint buf to term
      reprint_cmd(term);
      cursorX[term]++;
      move_cursor(term);
      break;
    case 0:  //add char at end of buf, print new
      term_putc(term,cmd_buf[term][cmd_len[term]-1]);
      cursorX[term]++;
      move_cursor(term);
      break;
    case 1:  //no new char write, handled in cases
      break;
    case 2:  //stop char encountered, handle
      cursorY[term]++;
      cursorX[term] = 0;
      move_cursor(term);
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
*   RETURN VALUE: 0 for moved, 1 for not
*   SIDE EFFECTS: moves the cursor on the display based on the formula to move it
*/
void move_cursor(unsigned int t){
  uint16_t loc;
  //handle screen bounds check
  validate_cursor(t);
  loc = cursorY[t] * XMAX + cursorX[t]; //find loc
  //dont actually move if not the active screen
  if(t != term){
    return;
  }
  //write low bits
  outb(C0F,C3D4);
  outb((uint8_t)(loc& 0xFF),C3D5);
  outb(C0E,C3D4);
  outb(((uint8_t)((loc >> BYTE) & 0xFF)),C3D5);
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
  //fix X value coordinate scheme
  while(cursorX[t] >= XMAX){
    cursorY[t]++;
    cursorX[t] = cursorX[t] - XMAX;
  }
  while(cursorX[t] < 0){
    cursorY[t]--;
    cursorX[t] = cursorX[t] + XMAX;
  }
  if(cursorY[t] < 0){
    cursorY[t] = 0;
  }
  //end of screen reached (SCROLL!!)
  if(cursorY[t] * XMAX + cursorX[t] >= SCRSIZE){
    scroll_screen(t);
  }
  return;
}
/*
* validate_move
*   DESCRIPTION: checks the validity of a given action in the cmd buf
*   INPUTS: terminal to operate on
*   OUTPUTS: none
*   RETURN VALUE: 0 for valid, 1 for invalid
*   SIDE EFFECTS: none
*/
int validate_move(){
  int32_t idx;
  //end of screen check
  idx = 0;
  while(cursorY[term] * XMAX + cursorX[term] > SCRSIZE-1){
    idx = 1;
    cursorX[term]--;
  }
  if(idx == 1){
    move_cursor(term);
    return 1;
  }
  idx = cursorY[term] * XMAX + cursorX[term];
  idx = idx - cmd_start[term];
  //check start and end of buf
  if(idx < 0){
    cursorX[term]++;
    return 1;
  }
  if(idx > cmd_len[term]){
    cursorX[term]--;
    return 1;
  }
  return 0;
}
/*
* scroll_screen
*   DESCRIPTION: scrolls the given terminal, shifts up mem
*   INPUTS: t - term to scroll
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: edits the contents of term vid mem
*/
void scroll_screen(unsigned int t){
  int x,y;
  char c[XMAX];
  uint8_t attr[XMAX];
  uint32_t tmpaddr;

  tmpaddr =  address[t];
  //checck for regular vidmem edit
  if(t == term){
    address[t] = VIDMEM_ADDR;
  }
  cli();
  //iterate over the rows and move them one up
  for(y=1;y<YMAX;y++){
    //load the lower row
    for(x=0;x<XMAX;x++){
      c[x] = *(uint8_t *)(address[t] + ((XMAX * y + x) << 1));
      attr[x] = *(uint8_t *)(address[t] + ((XMAX * y + x) << 1) + 1);
    }
    //copy to above
    for(x=0;x<XMAX;x++){
      *(uint8_t *)(address[t] + ((XMAX * (y-1) + x) << 1)) = c[x];
      *(uint8_t *)(address[t] + ((XMAX * (y-1) + x) << 1) + 1) = attr[x];
    }
  }
  //clear bottom line
  for(x=0;x<XMAX;x++){
    *(uint8_t *)(address[t] + ((XMAX * (YMAX-1) + x) << 1)) = NULLCHAR;
    *(uint8_t *)(address[t] + ((XMAX * (YMAX-1) + x) << 1) + 1) = attr[x];
  }
  address[t] = tmpaddr;
  //rectify cursor and shift cmd_start up
  cursorX[t] = 0;
  cursorY[t] = YMAX-1;
  cmd_start[t] = cmd_start[t] - XMAX;
  reprint_cmd(t);
  sti();
  move_cursor(t);
  return;
}
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
  if(ret == 1 || ret == 2 || ret == -1){
    //was a special char, handled
    return ret;
  }
  //needs to be written, insert to cmd_buf
  ret = insert_char(c);
  return ret;
}
/*
* process_char
*   DESCRIPTION: takes an ascii char val and uses it to do any needed commands
*     based on ops (ex: CTRL+L)
*   INPUTS: char to process
*   OUTPUTS: none
*   RETURN VALUE: 0 for no combo found, 1 for action taken
*   SIDE EFFECTS: can move the cursor or allow term read
*/
int process_char(char c){
  int i,ret,full;
  static unsigned int pow[7] = {1,2,4,8,16,32,64};

  switch(c)
  {
    case ENTER:  //enter
      termRead[term] = 'y';
      return 2;
      break;
    case BACKSPACE: //backspace
      cursorX[term]--;
      ret = validate_move();
      if(ret == 0){
        full = remove_char();
        if(full == -1){
          reprint_cmd(term);
        }
        if(full == 0){
          term_putc(term,' ');
        }
        move_cursor(term);
      }
      return 1;
      break;
    case CTRL_L:  //CTRL + l
      if(ops[1] == 'y'){
        term_clear(term,0);
        move_cursor(term);
        return 1;
      }
      break;
    case CTRL_C:  //CTRL + c
      if(ops[1] == 'y'){
        term_stop(term);
        return 1;
      }
      break;
    case ALT_F1:  //ALT + f1
      if(ops[3] == 'y'){
        term_start(0);
        return 1;
      }
      break;
    case ALT_F2:  //ALT + f2
      if(ops[3] == 'y'){
        term_start(1);
        return 1;
      }
      break;
    case ALT_F3:  //ALT + f3
      if(ops[3] == 'y'){
        term_start(2);
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
  //make sure no unwanted chars slip through
  full = 0;
  for(i=0;i<OPNUM;i++){
    if(ops[i] == 'y'){
      full = full + pow[i];
    }
  }
  if(full == 0 || full == 1 || full == 4 || full == 5){
    //is a printable char
    return 0;
  }
  if(full == 16 || full == 17 || full == 20 || full ==21){
    //is a printable char
    return 0;
  }
  //not wanted based on controls
  return 1;
}
/*
* reprint_cmd
*   DESCRIPTION: reprints the whole buf for char edits inside cmd_buf. avoids
*     messing up the cursor loc while using putc
*   INPUTS: t - term to write to
*   OUTPUTS: none
*   RETURN VALUE: returns 1 on completion
*   SIDE EFFECTS: none
*/
void reprint_cmd(uint8_t t){
  int tempX, tempY, i;

  if(cmd_len[t] == 0){
    return;
  }
  if(cmd_start[t]+BUFMAX >= SCRSIZE){
    validate_cursor(t);
  }
  //save current cursor loc
  tempX = cursorX[t];
  tempY = cursorY[t];
  //check for vals on next line
  cursorX[t] = cmd_start[t];
  cursorY[t] = 0;
  //print the buf, overwrite any old vals
  for(i=0; i<BUFMAX; i++){
    term_putc(t,cmd_buf[t][i]);
    cursorX[t]++;
  }
  //restore cursor
  cursorX[t] = tempX;
  cursorY[t] = tempY;
  move_cursor(t);
  return;
}
/*
* remove_char
*   DESCRIPTION: removes char from idx in buffer. This can be done at any index
*     with the buffer being adjusted accordingly.
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: returns 1 if no action, 0 if removed from end, -1 if removed in buf
*   SIDE EFFECTS: will adjust the cmd_buf and the lenght of the cmd
*/
int remove_char(){
  int i,idx;

  idx = cursorY[term] * XMAX + cursorX[term];
  idx = idx - cmd_start[term];

  if(idx < 0 || idx > BUFMAX){
    return 1;
  }
  if(cmd_len[term] == 0){
    return 1;
  }
  cmd_len[term]--;
  //remove at end of string
  if(idx == cmd_len[term]){
    cmd_buf[term][idx] = NULLCHAR;
    return 0;
  }
  //rm is in buffer, shift down
  for(i=idx;i<BUFMAX;i++){
    cmd_buf[term][i] = cmd_buf[term][i+1];
  }
  cmd_buf[term][i] = NULLCHAR;
  return -1;
}
/*
* insert_char
*   DESCRIPTION: inserts char to screen buffer. This can be done at any index
*     with the buffer being adjusted accordingly
*   INPUTS: character to insert
*   OUTPUTS: none
*   RETURN VALUE: returns 1 if no insert, 0 if inserted, -1 if insert inside buf
*   SIDE EFFECTS: will adjust the cmd_buf and the lenght of the cmd
*/
int insert_char(char c){
  char temp;
  int i,idx;

  idx = cursorY[term] * XMAX + cursorX[term];
  idx = idx - cmd_start[term];
  //check for invalid insert idx e
  if(idx < 0 || idx > BUFMAX-1){
    return 1;
  }
  //apped char at end of buf
  if(idx == cmd_len[term]){
    cmd_buf[term][idx] = c;
    cmd_len[term]++;
    return 0;
  }
  //insert if toggled on
  if(ops[4] == 'y'){
    cmd_buf[term][idx] = c;
    return -1;
  }
  //insert is inside buf, shift the content after up by 1
  for(i=cmd_len[term];i>idx;i--){
    temp = cmd_buf[term][i-1];
    cmd_buf[term][i] = temp;
  }
  cmd_buf[term][idx] = c;
  cmd_len[term]++;
  return -1;
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
  int ret;
  switch(scancode)
  {
    case UPARW:  //up arrow
      if(USEHIST == 1){
        histidx++;
        ret = history_fetch(term);
        if(ret == 0){
          reprint_cmd(term);
          move_cursor(term);
        }
      }
      return 1;
    case LEFTARW: //left arrow
      cursorX[term]--;
      validate_move();
      move_cursor(term);
      return 1;
    case RIGHTARW: //right arrow
      cursorX[term]++;
      validate_move();
      move_cursor(term);
      return 1;
    case DOWNARW: //down arrow
    if(USEHIST == 1){
        histidx--;
        ret = history_fetch(term);
        if(ret == 0){
          reprint_cmd(term);
          move_cursor(term);
        }
      }
      return 1;
    case TAB: //tab key
      auto_comp();
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
    case RSHIFT: //right shift
      ops[0] = 'y';
      return 1;
    case LSHIFT: //left shift
      ops[0] = 'y';
      return 1;
    case CTRL: //ctrl
      ops[1] = 'y';
      return 1;
    case CAPS: //caps (toggle)
      if(ops[2] == 'y'){
        ops[2] = 'n';
        return 1;
      }
      ops[2] = 'y';
      return 1;
    case ALT: //alt
      ops[3] = 'y';
      return 1;
    case INS: //ins (toggle)
      if(ops[4] == 'y'){
        ops[4] = 'n';
        return 1;
      }
      ops[4] = 'y';
      return 1;
  //released cased
    case RSHIFTO: //right shift
      ops[0] = 'n';
      return 1;
    case LSHIFTO: //left shift
      ops[0] = 'n';
      return 1;
    case CTRLO: //ctrl
      ops[1] = 'n';
      return 1;
    case ALTO: //alt
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
    case 16:      //just insert
      c = scancode_lower[scancode];
      return c;
    case 17:      //ins and shift
      c = scancode_upper[scancode];
      return c;
    case 20:      //ins and caps
      c = scancode_caps[scancode];
      return c;
    case 21:      //ins and caps and shift
      c = scancode_caps[scancode];
      return c;
    default:
      return c;
  }
}
/*
* history_fetch
*   DESCRIPTION: change over current term buffer to history and displays
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: 0 for swap, 1 for no action
*   SIDE EFFECTS: swaps in the buffered command from history
*/
int history_fetch(){
  unsigned int i;
  char c;

  if(USEHIST == 0){
    return 1;
  }
  //check for invalid access
  if(histidx < -1){
    histidx = -1;
    return 1;
  }
  if(histidx >= HISTNUM){
    histidx = HISTNUM-1;
    return 1;
  }
  //base, blank cmd
  if(histidx == -1){
    for(i=0;i<BUFMAX;i++){
      cmd_buf[term][i] = NULLCHAR;
    }
    cmd_len[term] = 0;
    return 0;
  }
  //check for history at idx
  if(cmd_hist_len[histidx] == 0){
    histidx--;
    return 1;
  }
  //move command to cmd_buf and set helpers accordingly
  for(i=0;i<BUFMAX;i++){
    c = cmd_hist[histidx][i];
    cmd_buf[term][i] = c;
  }
  cmd_len[term] = cmd_hist_len[histidx];
  cursorY[term] = 0;
  cursorX[term] = cmd_len[term] + cmd_start[term];
  validate_cursor(term);
  return 0;
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
    return;
  }
  for(i=0;i<HISTNUM-1;i++){
    for(j=0;j<BUFMAX;j++){
      cmd_hist[i+1][j] = cmd_hist[i][j];
    }
    cmd_hist_len[i+1] = cmd_hist_len[i];
  }
  //store most recent cmd
  for(i=0;i<BUFMAX;i++){
    cmd_hist[0][i] = cmd_buf[term][i];
  }
  cmd_hist_len[0] = cmd_len[term];
  return;
}
/*
* auto_comp
*   DESCRIPTION: writes an auto completed word to the cmd_buf based on file data
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: changed the contents of the cmd_buf
*/
void auto_comp(){
  uint8_t buf[SBUF];
  uint8_t argbuf[SBUF];
  uint8_t sbuf[2048];
  uint32_t sidx[200];
  char hit[200];
  int32_t cnt, fd, i, j, key;
  int32_t tot, fcnt, charcnt;
  char c;

  i=0;
  cnt=0;
  //find most recent space for next arg
  while(i<BUFMAX && i<cmd_len[term]){
    if(cmd_buf[term][i] == ' '){
      cnt = i;
    }
    i++;
  }
  if(cnt < 1 || cnt == cmd_len[term]-1){
    return; //return if no useful
  }
  key = cnt+1;
  //get rest of arg and count it
  charcnt = 0;
  for(i=cnt;i<cmd_len[term];i++){
    argbuf[charcnt] = cmd_buf[term][i];
    charcnt++;
  }
  //open dir
  fd = open((uint8_t*)".");
  if(fd == -1){
    return;
  }
  //get file count and load array
  cnt = 1;
  tot = 0;
  fcnt = 0;
  while (0 != (cnt = read(fd,buf,SBUF-1))){
    sidx[fcnt] = tot;
    for(i=0;i<cnt;i++){
      sbuf[tot] = buf[i];
      tot++;
    }
    fcnt++;
  }
  sidx[fcnt] = tot - sidx[fcnt-1];
  close(fd);
  //parse the chars
  for(i=0;i<fcnt;i++){
    hit[i] = 'y';
  }
  for(i=0;i<charcnt;i++){
    c = argbuf[i];
    for(j=0;j<fcnt;j++){
      //out of chars
      if(sidx[j]+i >= sidx[j+1]){
        hit[j] = 'n';
      }
      if(c != sbuf[sidx[j]+i]){
        hit[j] = 'n';
      }
      tot = 0;
      for(cnt=0;cnt<fcnt;cnt++){
        if(hit[cnt] == 'y'){
          tot++;
        }
      }
      if(tot == 1){
        break;
      }
    }
  }
  //more than one match
  tot = 0;
  for(cnt=0;cnt<fcnt;cnt++){
    if(hit[cnt] == 'y'){
      tot++;
    }
  }
  if(tot != 1){
    return;
  }
  //found a match!!!
  cursorX[term] = cursorX[term] - charcnt;
  for(i=0;i<fcnt;i++){
    if(hit[i] == 'y'){
      tot = i;
      break;
    }
  }
  cnt = 0;
  j = key;
  for(i=sidx[tot];i<sidx[tot+1];i++){
    cmd_buf[term][j] = sbuf[i];
    j++;
    cnt++;
    cursorX[term]++;
  }
  cmd_len[term] = key+cnt;
  reprint_cmd(term);
  move_cursor(term);
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
  uint32_t tmpaddr;
  //do same as reg putc buf with my vars
  tmpaddr = address[t];
  if(MULTITERM == 0 || t == term){
    //use standard vmem address
    address[t] = VIDMEM_ADDR;
  }
  if(cursorY[t] * XMAX + cursorX[t] > SCRSIZE){
    return;
  }
  *(uint8_t *)(address[t] + ((XMAX * cursorY[t] + cursorX[t]) << 1)) = c;
  *(uint8_t *)(address[t] + ((XMAX * cursorY[t] + cursorX[t]) << 1) + 1) = tattr[t];
  address[t] = tmpaddr;
  return;
}
/*
* term_clear
*   DESCRIPTION: clear and reset the terminal for other uses. can either write
*     just the prompt or write the whole buffer back to screen based on op
*   INPUTS: term number, operation condidions to perform
*   OUTPUTS: none
*   RETURN VALUE: 1 for complete
*   SIDE EFFECTS: will clear the screen or the screen memory of the address chosen
*     can also write to this memory after clear
*/
void term_clear(unsigned int t, int op){
  int i;

  //do reg clear if on curterm
  if(MULTITERM == 0 || t == term){
    clear();
  }
  //modify given clear, blank user vid mem
  else{
    for (i = 0; i < SCRSIZE; i++) {
        *(uint8_t *)(address[t] + (i << 1)) = NULLCHAR;
        *(uint8_t *)(address[t] + (i << 1) + 1) = tattr[t];
    }
  }
  cmd_start[t] = CURSOROFF;
  cursorX[t] = 0;
  cursorY[t] = 0;
  //full clear
  if(op == -1){
    for(i=0;i<BUFMAX;i++){
      cmd_buf[t][i] = NULLCHAR;
    }
    cmd_len[t] = 0;
    return;
  }
  //print prompt and start screen buf fill
  for(i=0;i<cmd_start[t];i++){
    term_putc(t,prompt[i]);
    cursorX[t]++;
  }
  //caller chose to add cmd buffer back to screen
  if(op == 1){
    for(i=0; i<cmd_len[term]; i++){
      term_putc(t,cmd_buf[t][i]);
      cursorX[t]++;
    }
  }
  //clear the cmd_buf
  else{
    for(i=0;i<BUFMAX;i++){
      cmd_buf[t][i] = NULLCHAR;
    }
    cmd_len[t] = 0;
  }
  //iterate cursor past added chars
  move_cursor(t);
  return;
}
