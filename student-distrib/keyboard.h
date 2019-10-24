#ifndef _KEY_H
#define _KEY_H

#include "types.h"

#define DATAPORT    0x60        //data port for keybaord
#define STATREG     0x64        //status mem location, also for com writes
#define ANDFULL     0x02        //used to check if buffer empty
#define BUFMAX      0x80        //max size of terminal string (128 for now)
#define XMAX        0x50        //max amount of chars on a line
#define YMAX        0x19        //max amount of allowed rows
#define CURSORLOW   0x0F        //write to low cursor byte
#define CURSORHIGH  0x0E        //write to high cursor byte
#define CURSORLOC   0x3D4       //mem loc for cursor
#define BYTE        0x08        //byte size
#define HNUM        0x05        //cnt for how many commands can be saved
//controls
#define USEPAGE     0x00        //binary on off for paging with the shell
#define USEHIST     0x01        //binary on/off for history feature
#define USECMD      0x01        //binary on/off for extra keyboard commands

extern void keyboard_init();
extern void update_term(unsigned int term);
extern void keyboard_handler();
extern int parse_input(uint8_t scancode);
extern void move_cursor(unsigned int cursor);
extern unsigned int fetch_process();
extern int update_ops(uint8_t scancode);
extern char generate_char(uint8_t scancode);
extern void insert_char(char c, int idx);
extern int32_t term_write(int32_t fd, const char * buf, int32_t nbytes);
extern int32_t term_read(int32_t fd, char * buf, int32_t nbytes);
extern void term_clear(unsigned int t, int op);
extern void term_putc(unsigned int t, uint8_t c);
extern int process_char(char c);
//extras
extern void history_fetch(int idx);
extern void history_write();

#endif
