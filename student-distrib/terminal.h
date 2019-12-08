#ifndef _TERM_H
#define _TERM_H

#define TNUM        0x03        //num of possible terms (3 for now)
#define TMEM        0xB8000     //vidmem address for the active term

//attr for term background color
static uint8_t tattr[TNUM] = {7,6,5};
volatile unsigned int curterm;            //terminal being used (index)
typedef struct terminal_t{
  int process_idx;
  int parent_process;
  int on_process[4];
}terminal_t;
terminal_t terminals[3];
extern unsigned int fetch_process();
extern void term_init();
extern void term_swap(unsigned int t);
extern int term_start(int t);
extern uint32_t term_addr(unsigned int t);
extern int32_t term_open(const uint8_t* filename);
extern int32_t term_close(int32_t fd);
extern void launch_term();
extern void context_swap(int t);
extern void term_stop(int t);

volatile int shellflag;                   //indicator for shell run
#endif
