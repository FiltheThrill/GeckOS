/* keyboard.c - Functions to display character to screen
Author - Ian Goodwin
https://wiki.osdev.org/PS2_Keyboard
 */

#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
char scancode_lowercase[] = {0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
                            '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
                            'i', 'o', 'p', '{', '}', '\n', 0, 'a', 's', 'd', 'f','g',
                            'h', 'j', 'k', 'l', ';', 0x27, 0x60, 0, 0x5c, 'z', 'x', 'c',
                            'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4',
                            '5', '6', '+', '1', '2', '3', '0', '.'};

//open keyboard Interrupt on port 1
void open_keyboard_irq(){
  enable_irq(1);
  clear();
  return;
}

void keyboard_handler(){
  char c;
  cli();
  asm volatile(
    "pushal\n"
    :
    :);

  c = inb(DATAPORT);
  if((c & 0x80) == 0)
  {
    c = scancode_lowercase[c];
    putc(c);
  }
  sti();
  send_eoi(1);
  asm volatile(
    "popal\n"
    :
    :);
  asm("leave");
  asm("iret");
}
