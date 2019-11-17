//gui stuff
//author - Ian Goodwin
#include "gui.h"
#include "lib.h"
#include "types.h"

//global coordinaes for mouse?
volatile int x, y;

void vga_text(){
  VGA_blank (1);
  set_seq_regs_and_reset (text_seq, 0x67);
  set_CRTC_registers (text_CRTC);
  set_attr_registers (text_attr);
  set_graphics_registers (text_graphics);
  fill_palette (3);
  VGA_blank (0);
  return;
}
//fill the screen with a color
void vga_fill(){
  int i;
  char c;

  c = 0x07;
  for(i=0;i<300;i++){
      //*(uint8_t *)((VMEM_START)+(i << 1)) = c;
  }
  return;
}
//changes the vga stuff for img display
void vga_mode13(){
  x = 0;
  y = 0;
  /* (might need proper pages from 0xA0000) */
  //VGA_blank (1);
  set_seq_regs_and_reset (mode_13_seq, 0x63);
  set_CRTC_registers (mode_13_CRTC);
  set_attr_registers (mode_13_attr);
  set_graphics_registers (mode_13_graphics);
  fill_palette(13);
  vga_fill();
  VGA_blank (0);
  return;
}
//blank the vga
void VGA_blank (int blank_bit)
{
    blank_bit = ((blank_bit & 1) << 5);

    asm volatile (
	"movb $0x01,%%al         /* Set sequencer index to 1. */       ;"
	"movw $0x03C4,%%dx                                             ;"
	"outb %%al,(%%dx)                                              ;"
	"incw %%dx                                                     ;"
	"inb (%%dx),%%al         /* Read old value.           */       ;"
	"andb $0xDF,%%al         /* Calculate new value.      */       ;"
	"orl %0,%%eax                                                  ;"
	"outb %%al,(%%dx)        /* Write new value.          */       ;"
	"movw $0x03DA,%%dx       /* Enable display (0x20->P[0x3C0]) */ ;"
	"inb (%%dx),%%al         /* Set attr reg state to index. */    ;"
	"movw $0x03C0,%%dx       /* Write index 0x20 to enable. */     ;"
	"movb $0x20,%%al                                               ;"
	"outb %%al,(%%dx)                                               "
      : : "g" (blank_bit) : "eax", "edx", "memory");
}
//sets regs
void set_seq_regs_and_reset (unsigned short table[NUM_SEQUENCER_REGS],
			unsigned char val)
{
    REP_OUTSW (0x03C4, table, NUM_SEQUENCER_REGS);
    {volatile int ii; for (ii = 0; ii < 10000; ii++);}
    OUTB (0x03C2, val);
    OUTW (0x03C4,0x0300);
    return;
}
//set CRTC
void set_CRTC_registers (unsigned short table[NUM_CRTC_REGS])
{
    OUTW (0x03D4, 0x0011);
    REP_OUTSW (0x03D4, table, NUM_CRTC_REGS);
    return;
}
//sets attrs
void set_attr_registers (unsigned char table[NUM_ATTR_REGS * 2])
{
    asm volatile (
	"inb (%%dx),%%al"
      : : "d" (0x03DA) : "eax", "memory");
    REP_OUTSB (0x03C0, table, NUM_ATTR_REGS * 2);
    return;
}
//set graphics
void set_graphics_registers (unsigned short table[NUM_GRAPHICS_REGS])
{
    REP_OUTSW (0x03CE, table, NUM_GRAPHICS_REGS);
    return;
}
//fill the vga pallete
void fill_palette (int type)
{
    if(type == 3){
      OUTB (0x03C8, 0x00);
      REP_OUTSB (0x03C9, palette_RGB, 16 * 3);
    }
    if(type == 13){
      OUTB (0x03C8, 0x00);
      REP_OUTSB (0x03C9, palette_RGB, 64 * 3);
    }
    return;
}
