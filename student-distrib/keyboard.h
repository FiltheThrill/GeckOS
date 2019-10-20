#ifndef _KEY_H
#define _KEY_H

#define DATAPORT    0x60        //data port for keybaord
#define STATREG   0x64          //status mem location, also for com writes

extern void open_keyboard_irq();
extern void keyboard_handler();

#endif
