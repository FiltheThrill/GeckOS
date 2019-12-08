#ifndef _PIT_H
#define _PIT_H

#define CHANNEL0 0x40
#define CHANNEL1 0X41
#define CHANNEL2 0x42
#define COMMANDREG 0x43
#define PITIRQ 0
#define MODE3  0x36
#define MASK 0xFF
#define SHIFT8 8
#define RELOAD_VAL 29102  // this will give us a frequency of of 41 HZ

extern void pit_init();
extern void pit_handler();
int curterm_nodisp;
#endif
