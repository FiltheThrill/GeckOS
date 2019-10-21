#include "rtc.h"
#include "lib.h"
#include "i8259.h"

void rtc_init()
{
	enable_irq(8);
	printf("132\n");
	
	cli();
	outb(0x8B, 0x70);
	char prev = inb(0x71);
	outb(0x8B, 0x70);
	outb(prev | 0x40, 0x71);
	sti();
}

void RTC_handler()
{
	cli();
	asm volatile(
	"pushal\n"
	:
	:);
	
	test_interrupts();
	printf("123 \n");
	
	sti();
	send_eoi(1);
	asm volatile(
	"popal\n"
	:
	:);
	asm("leave");
	asm("iret");
}