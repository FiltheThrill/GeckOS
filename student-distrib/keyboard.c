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

/*
* exceptions
*   DESCRIPTION: enables irq1 to allow keyboard interrupts
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: clears screen
*/
//open keyboard Interrupt on port 1
void open_keyboard_irq(){
  enable_irq(1);
  clear();
  return;
}
/*
* keyboard_handler
*   DESCRIPTION: putc a lowercase character or number to screen from keyboard
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: echo's keypress to screen
*/
void keyboard_handler(){
  unsigned char c;
  cli();                  //disable interrupts for now
  asm volatile(
    "pushal\n"
    :
    :);

  c = inb(DATAPORT);          //get data from key press
  if((c & 0x80) == 0)         //check if its the press and not the release
  {
    c = scancode_lowercase[c];
    putc(c);
  }
  sti();                      // reenable interrupts
  send_eoi(1);                //send end of interrupt signal to irq2
  asm volatile(
    "popal\n"
    :
    :);
  asm("leave");
  asm("iret");            // interrupt return
}
