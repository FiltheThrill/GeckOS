/* rtc.c - Functions to enable/generate periodic intterrupts using the RTC
Author - Frankie Papa
https://wiki.osdev.org/RTC
https://courses.engr.illinois.edu/ece391/fa2019/secure/references/ds12887.pdf
 */
#include "lib.h"
#include "i8259.h"
#include "rtc.h"

volatile int rtc_interrupt_flag;
volatile int rtc_counter;
volatile int rtc_curr_freq;

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

	cli(); // disable interrupts
	outb(REG_A_NMI, RTC_PORT); // choose register A of the rtc with NMI
	prev = inb(CMOS_PORT); // get the previous value of reg a
	outb (REG_A_NMI, RTC_PORT); // choose register A again
	outb (((prev & FREQ_BITMASK) | HZ_1024), CMOS_PORT); // OR our new rate with the lower 4 bits for frequency
	sti(); // enable interrupts

	rtc_counter = COUNTER_INIT;
	rtc_curr_freq = FREQ_INIT;

	enable_irq(RTC_IRQ);

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

	if (rtc_counter > MAX_HZ/rtc_curr_freq)
		rtc_counter = COUNTER_INIT;
	else
		rtc_counter++;

	//test_interrupts(); // Call test_interrupts
	if (MAX_HZ/rtc_curr_freq == rtc_counter)
		rtc_interrupt_flag = 0;

	outb (REG_C, RTC_PORT); // Allow more interrupts to occur
	inb(CMOS_PORT);

	sti(); //enable interrupts
	send_eoi(RTC_IRQ); //send end of interrupt on irq8
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
	sti();
	rtc_interrupt_flag = 1;
	while (rtc_interrupt_flag);
	cli();
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
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
	// use the value of nbytes to determine the frequency of our rtc
	// if nbytes is not between 0 and 1024 or divisible by 2 return -1 for failure
	switch(*((uint32_t *) buf))
	{
		case 0:
			rtc_curr_freq = 2;
			break;
		case 2:
			rtc_curr_freq = 2;
			break;
		case 4:
			rtc_curr_freq = 4;
			break;
		case 8:
			rtc_curr_freq = 8;
			break;
		case 16:
			rtc_curr_freq = 16;
			break;
		case 32:
			rtc_curr_freq = 32;
			break;
		case 64:
			rtc_curr_freq = 64;
			break;
		case 128:
			rtc_curr_freq = 128;
			break;
		case 256:
			rtc_curr_freq = 256;
			break;
		case 512:
			rtc_curr_freq = 512;
			break;
		case 1024:
			rtc_curr_freq = 1024;
			break;
		default:
			return RTC_FAIL;
	}

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
	rtc_curr_freq = FREQ_INIT;

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
