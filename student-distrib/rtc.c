/* rtc.c - Functions to enable/generate periodic intterrupts using the RTC
Author - Frankie Papa
https://wiki.osdev.org/RTC
https://courses.engr.illinois.edu/ece391/fa2019/secure/references/ds12887.pdf
 */
#include "rtc.h"
#include "lib.h"
#include "i8259.h"

/*
* rtc_init
*   DESCRIPTION: enables irq8 to allow rtc interrupts
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: none
*/
void rtc_init()
{
	cli();	//disable interrupts
	outb(REG_B_NMI, RTC_PORT); //select reg B with NMI disabled
	char prev = inb(CMOS_PORT); //read value in reg B
	outb(REG_B_NMI, RTC_PORT); //set RTC to reg B NMI
	outb(prev | PIE_EN, CMOS_PORT); // Turn on Periodic Interrupt Enable bit
	
	
	//set f = 2Hz
	outb(REG_A_NMI, RTC_PORT);
	prev = inb(CMOS_PORT);
	outb (REG_A_NMI, RTC_PORT);
	outb (((prev & 0xF0) | 0x0F), CMOS_PORT);
	sti();
	
	outb(REG_C, RTC_PORT); // allow interrupts to occur again
	inb(CMOS_PORT);
	
	enable_irq(8);
	
}

/*
* RTC_handler
*   DESCRIPTION: called on an rtc interrupt (set ot base frequency)
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: calls test_interrupts on every rtc interrupt
*/
void RTC_handler()
{
	cli(); //disable interrupts
	asm volatile(
	"pushal\n"
	:
	:);
	//test_interrupts(); // Call test_interrupts
	
	outb (REG_C, RTC_PORT); // Allow more interrupts to occur
	inb(CMOS_PORT);
	
	sti(); //enable interrupts
	send_eoi(8); //send end of interrupt on irq8
	asm volatile(
	"popal\n"
	:
	:);
	asm("leave");
	asm("iret");
}

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
	
	
	return RTC_SUCCESS;
}

int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes)
{	
	char rate;
	
	switch(nbytes)
	{
		case 0:
			rate = 0x00;
			break;
		case 2:
			rate = 0x0F;
			break;
		case 4:
			rate = 0x0E;
			break;
		case 8:
			rate = 0x0D;
			break;
		case 16:
			rate = 0x0C;
			break;
		case 32:
			rate = 0x0B;
			break;
		case 64:
			rate = 0x0A;
			break;
		case 128:
			rate = 0x09;
			break;
		case 256:
			rate = 0x08;
			break;
		case 512:
			rate = 0x07;
			break;
		case 1024:
			rate = 0x06;
			break;
		default:
			return RTC_FAIL;
	}
	
	cli();
	outb(REG_A_NMI, RTC_PORT);
	char prev = inb(CMOS_PORT);
	outb (REG_A_NMI, RTC_PORT);
	outb (((prev & 0xF0) | rate), CMOS_PORT);
	sti();
	
	return RTC_SUCCESS;
}

int32_t rtc_open(const uint8_t* filename)
{
	//set f = 2Hz
	cli();
	outb(REG_A_NMI, RTC_PORT);
	prev = inb(CMOS_PORT);
	outb (REG_A_NMI, RTC_PORT);
	outb (((prev & 0xF0) | 0x0F), CMOS_PORT);
	sti();
}

int32_t rtc_close(int32_t fd)
{
	return RTC_SUCCESS;
}


