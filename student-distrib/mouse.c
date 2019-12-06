#include "mouse.h"
#include "i8259.h"
#include "lib.h"

void mouse_init(){


  enable_irq(12);
  return;
}

void mouse_handler(){
  cli();
  asm volatile(
    "pushal\n"
    :
    :);




    sti();
    send_eoi(12);
    asm volatile(
      "popal\n"
      :
      :);

    asm("leave");
    asm("iret");
}

int32_t mouse_read(int32_t fd, void* buf, int32_t nbytes){

  return 0;
}

int32_t mouse_write(int32_t fd, const void* buf, int32_t nbytes){

  return 0;
}

int32_t mouse_open(const uint8_t* filename){

  return 0;
}

int32_t mouse_close(int32_t fd){

  return 0;
}
