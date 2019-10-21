#include "rtc.h"
#include "lib.h"
#include "i8259.h"

void rtc_init()
{
	
	
	cli();
	outb(0x8B, 0x70);
	char prev = inb(0x71);
	outb(0x8B, 0x70);
	outb(prev | 0x40, 0x71);
	sti();
	
	outb(0x0C, 0x70);
	inb(0x71);
	
	enable_irq(8);
	
}

void RTC_handler()
{
	cli();
	asm volatile(
	"pushal\n"
	:
	:);
	//test_interrupts();
	//printf("123 \n");
	
	outb (0x0C, 0x70);
	inb(0x71);
	
	sti();
	send_eoi(8);
	asm volatile(
	"popal\n"
	:
	:);
	asm("leave");
	asm("iret");
}
