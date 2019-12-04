#ifndef _KEY_H
#define _KEY_H

#include "types.h"

#define KEYIRQ      0x01        //irq # for keyboard
#define DATAPORT    0x60        //data port for keybaord
#define STATREG     0x64        //status mem location, also for com writes
#define ANDFULL     0x02        //used to check if buffer empty
#define BUFMAX      0x80        //max size of terminal string (128 char)
#define XMAX        0x50        //max amount of chars on a line
#define YMAX        0x19        //max amount of allowed rows
#define SCRSIZE     0x7D0       //size of the screen
#define CURSORD     0x06        //size of default prompt
#define CURSOROFF   0x06        //start offset for cursor to allow prompt
#define C0F         0x0F        //write to low cursor byte
#define C0E         0x0E        //write to high cursor byte
#define C3D4        0x3D4       //low mem loc for cursor
#define C3D5        0x3D5       //high mem loc for cursor
#define BYTE        0x08        //byte size
#define HISTNUM     0x05        //cnt for how many commands can be saved
#define OPNUM       0x05        //amount of ops to watch
#define SBUF        0x21        //size of name buffer
#define VIDMEM_ADDR 0xB8000     //vidmem address
//feature controls
#define MULTITERM   0x01        //binary on off for multi term
#define USEHIST     0x01        //binary on/off for history feature
#define USECMD      0x01        //binary on/off for extra keyboard commands
//character switch defs
#define ATTR        0x07
#define NULLCHAR    0x00
#define ENTER       0x0D
#define BACKSPACE   0x08
#define CTRL_L      0x6C
#define CTRL_C      0x63
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
#define ALTO        0xB8
#define TAB         0x0F
#define ALT_F1      0x3B
#define ALT_F2      0x3C
#define ALT_F3      0x3D
//external
extern void keyboard_init();
extern void update_term(unsigned int term);
extern void keyboard_handler();
extern int32_t term_write(int32_t fd, const void * buf, int32_t nbytes);
extern int32_t term_read(int32_t fd, void * buf, int32_t nbytes);
extern void term_clear(unsigned int t, int op);
extern void term_putc(unsigned int t, uint8_t c);
//internals
void move_cursor(unsigned int t);
int process_char(char c);
int process_media(uint8_t scancode);
int parse_input(uint8_t scancode);
int update_ops(uint8_t scancode);
char generate_char(uint8_t scancode);
int insert_char(char c);
int remove_char();
void validate_cursor(uint8_t t);
int validate_move();
void reprint_cmd(uint8_t t);
void scroll_screen(unsigned int t);
//extras
int history_fetch();
void history_write();
void auto_comp();

#endif
