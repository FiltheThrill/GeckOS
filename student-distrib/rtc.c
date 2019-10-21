/* rtc.c - Functions to display character to screen
Author - Frankie Papa
https://wiki.osdev.org/RTC
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
	sti(); // enable interrupts
	
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
