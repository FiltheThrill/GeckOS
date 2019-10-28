/* rtc.c - Functions to enable/generate periodic intterrupts using the RTC
Author - Frankie Papa
https://wiki.osdev.org/RTC
https://courses.engr.illinois.edu/ece391/fa2019/secure/references/ds12887.pdf
 */
#include "lib.h"
#include "i8259.h"
#include "rtc.h"

volatile int rtc_interrupt_flag;

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
	
	const uint8_t* fname = (const uint8_t*)"";
	int32_t open = rtc_open(fname);
	open++;
	
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
	rtc_interrupt_flag = 0;
	
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

/*
* rtc_read
*   DESCRIPTION: loops until an RTC interrupt occurs
*   INPUTS: int32_t fd, void* buf, int32_t nbytes
*			all NOT used
*   OUTPUTS: int32_t
*   RETURN VALUE: 0 on success
*   SIDE EFFECTS: blocks until next interrupt
*/
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
	// a volatile interrupt flag which is changed to 0 in the RTC handler
	// loops and does nothing waiting for next interrupt to occur
	rtc_interrupt_flag = 1;
	while (rtc_interrupt_flag);
	
	return RTC_SUCCESS;
}

/*
* rtc_write
*   DESCRIPTION: Changes the frequency of RTC to a frequency that is a power of 2
*   INPUTS: int32_t fd, void* buf, int32_t nbytes
*			nbytes is the value in Hz of our new frequency
*   OUTPUTS: int32_t
*   RETURN VALUE: 0 on success
*				  -1 when a bad input is given (not power of 2 or inbetween 0 and 1024)
*   SIDE EFFECTS: changes the frequency of the rtc
*/
int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes)
{	
	// initialize the rate to 0 Hz
	char rate = HZ_0;
	
	// use the value of nbytes to determine the frequency of our rtc
	// if nbytes is not between 0 and 1024 or divisible by 2 return -1 for failure
	switch(nbytes)
	{
		case 0:
			rate = HZ_0;
			break;
		case 2:
			rate = HZ_2;
			break;
		case 4:
			rate = HZ_4;
			break;
		case 8:
			rate = HZ_8;
			break;
		case 16:
			rate = HZ_16;
			break;
		case 32:
			rate = HZ_32;
			break;
		case 64:
			rate = HZ_64;
			break;
		case 128:
			rate = HZ_128;
			break;
		case 256:
			rate = HZ_256;
			break;
		case 512:
			rate = HZ_512;
			break;
		case 1024:
			rate = HZ_1024;
			break;
		default:
			return RTC_FAIL;
	}
	
	cli(); // disable interrupts
	outb(REG_A_NMI, RTC_PORT); // choose register A of the rtc with NMI
	char prev = inb(CMOS_PORT); // get the previous value of reg a
	outb (REG_A_NMI, RTC_PORT); // choose register A again
	outb (((prev & FREQ_BITMASK) | rate), CMOS_PORT); // OR our new rate with the lower 4 bits for frequency
	sti(); // enable interrupts
	
	// return 0
	return RTC_SUCCESS;
}

/*
* rtc_read
*   DESCRIPTION: initializes the rtc frequency to 2HZ
*   INPUTS: const uint8_t* filename
*			NOT used
*   OUTPUTS: int32_t
*   RETURN VALUE: 0 on success
*   SIDE EFFECTS: none
*/
int32_t rtc_open(const uint8_t* filename)
{
	//set f = 2Hz
	cli(); // disable interrupts
	outb(REG_A_NMI, RTC_PORT); // choose register A of the rtc with NMI
	char prev = inb(CMOS_PORT); // get the previous value of reg a
	outb (REG_A_NMI, RTC_PORT); // choose register A again
	outb (((prev & FREQ_BITMASK) | HZ_2), CMOS_PORT); // OR our new rate with the lower 4 bits for frequency (2Hz)
	sti(); // enable interrupts
	
	return RTC_SUCCESS;
}

/*
* rtc_read
*   DESCRIPTION: Does nothing
*   INPUTS: int32_t fd - NOT used
*   OUTPUTS: int32_t
*   RETURN VALUE: 0 on success
*   SIDE EFFECTS: none
*/
int32_t rtc_close(int32_t fd)
{
	// return 0 for success
	return RTC_SUCCESS;
}
