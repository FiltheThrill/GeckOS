#ifndef _KEY_H
#define _KEY_H

#define DATAPORT    0x60        //data port for keybaord
#define STATREG     0x64        //status mem location, also for com writes
#define ANDFULL     0x02        //used to check if buffer empty
#define BUFMAX      0x400       //max size of terminal string (128 for now)
#define TNUM        0x01        //num of possible terms (1 for now)
#define XMAX        0x50        //max amount of chars on a line
#define YMAX        0x19        //max amount of allowed rows
#define CURSORLOW   0x0F        //write to low cursor byte
#define CURSORHIGH  0x0E        //write to high cursor byte
#define CURSORLOC   0x3D4       //mem loc for cursor
#define BYTE        0x08        //byte size
//extras
#define HNUM        0x05        //cnt for how many commands can be saved
#define USEHIST     0x01        //binary on/off for history feature

extern void keyboard_init();
extern void keyboard_handler();
extern void move_cursor(unsigned int cursor);
extern void fetch_pid();
extern char convert_scancode(uint8_t scancode);
extern uint16_t check_flags(uint8_t scancode);
extern void insert_char(char c, int idx);
extern int32_t term_write(int32_t fd, const void * buf, int32_t nbytes);
extern int32_t term_read(int32_t fd, void * buf, int32_t nbytes);
extern void term_clear();
//extras
extern void history_fetch(int idx);
extern void history_write();

#endif
