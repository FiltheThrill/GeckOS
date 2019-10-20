#include "keyboard.h"
#include "i8259.h"
#include "lib.h"

//open keyboard Interrupt on port 1
void open_keyboard_irq(){
  enable_irq(1);
  return;
}

void keyboard_handler(){
  clear();
  char c;
  c = inb(DATAPORT);

  printf("brug\n");
  printf("%c",c);
  return;
}
