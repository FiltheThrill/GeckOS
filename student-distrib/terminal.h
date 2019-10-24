#ifndef _TERM_H
#define _TERM_H

#define TNUM        0x03        //num of possible terms (3 for now)

extern void term_init();
extern void term_put(unsigned int term);
extern void term_swap(unsigned int term);
extern int term_start(unsigned int num);
extern uint8_t term_addr(unsigned int term);
#endif
