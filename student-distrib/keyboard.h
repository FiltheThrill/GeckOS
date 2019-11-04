#ifndef _KEY_H
#define _KEY_H

#include "types.h"

#define KEYIRQ      0x01        //irq # for keyboard
#define DATAPORT    0x60        //data port for keybaord
#define STATREG     0x64        //status mem location, also for com writes
#define ANDFULL     0x02        //used to check if buffer empty
#define BUFMAX      0x80        //max size of terminal string (128 for now)
#define XMAX        0x50        //max amount of chars on a line
#define YMAX        0x19        //max amount of allowed rows
#define SCRSIZE     0x7D0       //size of the screen
#define CURSOROFF   0x07        //offset for cursor to allow prompt
#define CURSORLB    0x0F        //write to low cursor byte
#define CURSORHB    0x0E        //write to high cursor byte
#define CURSORLA    0x3D4       //low mem loc for cursor
#define CURSORHA    0x3D5       //high mem loc for cursor
#define BYTE        0x08        //byte size
#define HISTNUM     0x05        //cnt for how many commands can be saved
#define OPNUM       0x05        //amount of ops to watch
//feature controls
#define USEPAGE     0x00        //binary on off for paging with the shell
#define USEHIST     0x00        //binary on/off for history feature
#define USECMD      0x01        //binary on/off for extra keyboard commands
//character switch defs
#define ATTR        0x07
#define NULLCHAR    0x00
#define ENTER       0x0D
#define BACKSPACE   0x08
#define CTRL_L      0x6C
#define UPARW       0x48
#define LEFTARW     0x4B
#define RIGHTARW    0x4D
#define DOWNARW     0x50
#define RSHIFT      0x36
#define LSHIFT      0x2A
#define CTRL        0x1D
#define CAPS        0x3A
#define ALT         0x38
#define INS         0x52
#define RSHIFTO     0xB6
#define LSHIFTO     0xAA
#define CTRLO       0x9D
#define ALTO        0xB9
//externals
extern void keyboard_init();
extern void update_term(unsigned int term);
extern void keyboard_handler();
extern int32_t term_write(int32_t fd, const void * buf, int32_t nbytes);
extern int32_t term_read(int32_t fd, void * buf, int32_t nbytes);
extern void term_clear(unsigned int t, int op);
extern void term_putc(unsigned int t, uint8_t c);
//internals
void move_cursor(unsigned int t);
unsigned int fetch_process();
void fill_cmdbuf();
int process_char(char c);
int process_media(uint8_t scancode);
int parse_input(uint8_t scancode);
int update_ops(uint8_t scancode);
char generate_char(uint8_t scancode);
int insert_char(char c, int idx);
int remove_char(int idx);
int validate_cursor(uint8_t t);
int reprint_screen();
//extras
void history_fetch(int idx);
void history_write();

#endif
