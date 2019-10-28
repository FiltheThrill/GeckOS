#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"

volatile unsigned int curterm;            //terminal being used (index)
volatile unsigned int termnum[TNUM];      //id to correspond with term var
volatile uint8_t termmem[TNUM];           //vid mem pointers
volatile int termpid[TNUM];               //process id given to the terminal
volatile char termrun[TNUM];              //check if term is active

/*
* term_init
*   DESCRIPTION: This function fills the terminal global vars with default values
*     to initialize them
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: fills the globals with default vals
*/
void term_init(){
  int i;
  uint32_t v,p;  //variables for paging setup
  for(i=0;i<TNUM;i++){
    termnum[i] = i;
    termpid[i] = -1; //no process
    termrun[i] = 'n'; //not running
  }
  //0 is default term
  curterm = 0;
  //get vid mem address for each terminal(dead for now)
  v =0;
  p =0;
  for(i=0;i<TNUM;i++){
    //termmem[i] = paging_terminal(v,p);
    termmem[i] = 0;
  }
  return;
}
/*
* term_start
*   DESCRIPTION: turns on a terminal or makes it the active one based on the
*     desired termional
*   INPUTS: num - number of the terminal to opreate on
*   OUTPUTS: none
*   RETURN VALUE: returns 0 if already running, 1 if fresh start
*   SIDE EFFECTS: starts a new operational terminal
*/
int term_start(unsigned int num){
  //protect memory
  cli();
  //validate request
  if(num > TNUM || num == curterm){
    sti();
    return -1;
  }
  //running, switch back to it
  if(termrun[num] == 'y'){
    term_swap(num);
    sti();
    return 0;
  }
  //not running, must start shell (will use swap for now)
  termrun[num] = 'y';
  term_swap(num);
  sti();
  return 1;
}
/*
* term_swap
*   DESCRIPTION: moves between the active terminals and updates the screen
*     memory with the desired terminal memory
*   INPUTS: term - term number to swap to
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: will change the active terminal and clear the screen, also
*     updates the address to write to
*/
void term_swap(unsigned int term){
  //update values in both instances
  curterm = term;
  update_term(term);
  //clear and write whatever is in the buffer
  term_clear(term,1);
  return;
}
uint8_t term_addr(unsigned int term){
  return termmem[term];
}
