/* terminal.c - Functions to handle terminal stuff
Author - Ian Goodwin
//resources used
https://wiki.osdev.org/Multitasking_Systems
https://wiki.osdev.org/Page_Frame_Allocation
*/

#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"
#include "syscalls.h"

volatile unsigned int curterm;            //terminal being used (index)
volatile uint32_t termmem[TNUM];          //vid mem pointers
volatile int shellpid[TNUM];              //process id given to the coresponding shell
volatile char termrun[TNUM];              //check if term is active
volatile uint32_t swapebp[TNUM];          //storage for each ebp
volatile uint32_t swapesp[TNUM];          //storage for each esp

/*
* term_init
*   DESCRIPTION: This function fills the terminal global vars with default values
*     to initialize them
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: fills the globals with default vals and adresses
*/
void term_init(){
  int i,j;
  uint32_t buf[TNUM];

  for(i=0;i<TNUM;i++){
    shellpid[i] = -1; //no process
    termrun[i] = 'n'; //not running
    swapebp[TNUM] = 0;
    swapesp[TNUM] = 0;
  }
  //fill vid mem address for each terminal
  j = TNUM;
  i = term_page(j,buf);
  if(i != TNUM){
    //idk? thats bad tho
  }
  for(j=0;j<TNUM;j++){
    termmem[j] = buf[j];
  }
  for(i=0; i<TNUM; i++){
    //clear all usr mem
    for(j=0; j<SCRSIZE; j++){
      *(uint8_t *)(termmem[i] + (j << 1)) = NULLCHAR;
      *(uint8_t *)(termmem[i] + (j << 1) + 1) = tattr[i];
      return;
    }
  }
  curterm = 0;
  shellflag = 0;
  return;
}
//launch term executes term after everything else is initialized
void launch_term(){
  curterm = 0;
  termrun[0] = 'y';
  shellpid[0] = 0; //first process num
  shellflag = 1;
  execute((const uint8_t*) "shell");
  return;
}
/*
* term_start
*   DESCRIPTION: turns on a terminal or makes it the active one based on the
*     desired terminal
*   INPUTS: t - number of the terminal to opreate on
*   OUTPUTS: none
*   RETURN VALUE: returns 0 if already running, 1 if fresh start
*   SIDE EFFECTS: starts a new operational terminal
*/
int term_start(int t){
  //validate requested start
  if(t >= TNUM || t == curterm || t < -1){
    return -1;
  }
  //reset from first term
  if(t == -1){
    launch_term();
    return 1;
  }
  //running, switch back to it
  if(termrun[t] == 'y'){
    term_swap(t);
    context_swap(t);
    curterm = t;
    return 0;
  }
  //not running, must start a new shell and swap to
  else{
    termrun[t] = 'y';
    //start new shell
    shellflag = 1;
    shellpid[t] = c_process_num;
    term_swap(t);
    context_swap(t);
    curterm = t;
    execute((const uint8_t*) "shell");
    return 1;
  }
}
/*
* term_stop
*   DESCRIPTION: turns off the given terminal
*   INPUTS: t - number of the terminal to opreate on
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: kills a terminal and its processes
*/
void term_stop(int t){
  int j,cnt;
  //validate requested stop
  if(t >= TNUM || t < 0){
    return;
  }
  //shut off the given terminal
  termrun[t] = 'n';
  shellpid[t] = -1;
  for(j=0; j<SCRSIZE; j++){
    *(uint8_t *)(termmem[t] + (j << 1)) = NULLCHAR;
    *(uint8_t *)(termmem[t] + (j << 1) + 1) = tattr[t];
  }
  swapebp[t] = 0;
  swapesp[t] = 0;
  //kill the given processes belonging to the shell
  while(PCB_arr[c_process_num]->parent_process != c_process_num){
    //halt(-2);
    break;
  }
  //kill the shell
  //halt(-2);
  cnt = -1;
  for(j=TNUM; j>0; j--){
    if(termrun[j-1] == 'y'){
      cnt = j-1;
    }
  }
  //killed last terminal, reset
  if(cnt == -1){
    term_start(-1);
    return;
  }
  //swap to the highest avaliable term
  term_start(cnt);
  return;
}
/*
* term_swap
*   DESCRIPTION: moves between the active terminals and updates the video
*     memory with the desired terminal memory
*   INPUTS: t - term number to swap to
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: will change the active terminal and clear the screen, also
*     updates the address to write to
*/
void term_swap(unsigned int t){
  int i;
  uint8_t c,a;
  //uint8_t* vidaddy;
  //save the current screen into usr mem
  //vidmap(&vidaddy);
  for (i = 0; i < SCRSIZE; i++) {
    c = *(uint8_t *)(VIDMEM_ADDR + (i << 1));
    a = *(uint8_t *)(VIDMEM_ADDR + (i << 1) + 1);
    *(uint8_t *)(termmem[curterm] + (i << 1)) = c;
    *(uint8_t *)(termmem[curterm] + (i << 1) + 1) = a;
  }
  //move the usr mem to video
  for (i = 0; i < SCRSIZE; i++) {
    c = *(uint8_t *)(termmem[t] + (i << 1));
    a = *(uint8_t *)(termmem[t] + (i << 1) + 1);
    *(uint8_t *)(VIDMEM_ADDR + (i << 1)) = c;
    *(uint8_t *)(VIDMEM_ADDR + (i << 1) + 1) = a;
  }
  //update curterm in keyboard and terminal
  curterm = t;
  update_term(t);
  return;
}
/*
* context_swap
*   DESCRIPTION: switches the active process
*   INPUTS: t - term to swap to
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: will change the active process
*/
void context_swap(int t){
  uint32_t base, stack;
  //save esp and ebp for swap back later
  asm volatile(
    "movl %%esp, %0\n"
    "movl %%ebp, %1\n"
    :"=r"(stack), "=r"(base)
  );
  swapebp[curterm] = base;
  swapesp[curterm] = stack;
  //the destination is new, execute will swap?
  if(swapebp[t] == 0 || swapesp[t] == 0){
    base = PCB_arr[c_process_num]->ebp;
    stack = PCB_arr[c_process_num]->esp;
    return;
  }
  else{
    base = swapebp[t];
    stack = swapesp[t];
  }
  //switch to geico
  asm volatile(
       "movl %%ebx, %%esp\n"
       :
       :"b"(stack)
     );
  asm volatile(
       "mov %%ebx, %%ebp\n"
       :
       :"b"(base)
     );
  flush_tlb();
  return;
}
//function to get the proper terminal process id in order to do sys calls
unsigned int fetch_process(){
  unsigned int id, pid;
  int i;
  PCB_t* node;
  //find the parent (shell)
  id = c_process_num;
  for(i=0; i<MAXPROCESSES; i++){
    node = PCB_arr[id];
    pid = node->p_index;
    if(id == pid){
      return id;
    }
    id = pid;
  }
  //return curterm ( failed to find D: )
  return curterm;
}
//returns the requested term's usr mem adress
uint32_t term_addr(unsigned int t){
  return termmem[t];
}
//opens the termianl (dummy func)
int32_t term_open(const uint8_t* filename){
  return 0;
}
//closes the terminal (dummy func)
int32_t term_close(int32_t fd){
  return 0;
}
