#include "keyboard.h"
#include "i8259.h"
#include "lib.h"

//open keyboard Interrupt on port 1
void open_keyboard_irq(){
  enable_irq(1);
  return;
}

void keyboard_handler(){
  cli();
  clear();
  asm volatile(
    "pushal\n"
    :
    :);
  char c;
  c = inb(DATAPORT);

  printf("brug\n");
  printf("%d",c);
  sti();
  send_eoi(1);
  asm volatile(
    "popal\n"
    :
    :);
  asm("leave");
  asm("iret");
}
