#include "lib.h"
#include "i8259.h"
#include "speaker.h"

//Play sound using built in speaker
static void play_sound(uint32_t nFrequence) {
 uint32_t Div;
 uint8_t tmp;

       //Set the PIT to the desired frequency
 Div = 1193180 / nFrequence;

 outb(0xb6, 0x43);
 outb((uint8_t) (Div), 0x42);
 outb((uint8_t) (Div >> 8), 0x42);

       //And play the sound using the PC speaker
 tmp = inb(0x61);
   if (tmp != (tmp | 3)) {
   outb(tmp | 3, 0x61);
 }
}

//make it shutup
static void nosound() {
 uint8_t tmp = inb(0x61) & 0xFC;

 outb(tmp, 0x61);
}

//Make a beep
void beep() {
  play_sound(1000);
  int i = 0;
  while (i < 1000)
  {
    i++;
  }
  nosound();
}
