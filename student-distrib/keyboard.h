#ifndef _KEY_H
#define _KEY_H

#include "types.h"

#define DATAPORT    0x60        //data port for keybaord
#define STATREG     0x64        //status mem location, also for com writes
#define ANDFULL     0x02        //used to check if buffer empty
#define BUFMAX      0x80        //max size of terminal string (128 for now)
#define XMAX        0x50        //max amount of chars on a line
#define YMAX        0x19        //max amount of allowed rows
#define CURSOROFF   0x08        //offset for cursor to allow prompt
#define CURSORLB    0x0F        //write to low cursor byte
#define CURSORHB    0x0E        //write to high cursor byte
#define CURSORLA    0x3D4       //low mem loc for cursor
#define CURSORHA    0x3D5       //high mem loc for cursor
#define BYTE        0x08        //byte size
#define HISTNUM     0x05        //cnt for how many commands can be saved
//controls
#define USEPAGE     0x00        //binary on off for paging with the shell
#define USEHIST     0x01        //binary on/off for history feature
#define USECMD      0x01        //binary on/off for extra keyboard commands

//externals
extern void keyboard_init();
extern void update_term(unsigned int term);
extern void keyboard_handler();
extern void move_cursor();
extern unsigned int fetch_process();
extern int32_t term_write(int32_t fd, const char * buf, int32_t nbytes);
extern int32_t term_read(int32_t fd, char * buf, int32_t nbytes);
extern void term_clear(unsigned int t, int op);
extern void term_putc(unsigned int t, uint8_t c);
//internals
int process_char(char c);
int process_media(uint8_t scancode);
int parse_input(uint8_t scancode);
int update_ops(uint8_t scancode);
char generate_char(uint8_t scancode);
void insert_char(char c, int idx);
void validate_cursor(uint8_t t);
//extras
void history_fetch(int idx);
void history_write();

#endif
