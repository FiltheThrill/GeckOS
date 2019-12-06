#ifndef _MOUSE_H
#define _MOUSE_H

#include "types.h"

extern void mouse_init();
extern void mouse_handler();

int32_t mouse_read(int32_t fd, void* buf, int32_t nbytes);
int32_t mouse_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t mouse_open(const uint8_t* filename);
int32_t mouse_close(int32_t fd);

#endif
