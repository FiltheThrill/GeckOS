#ifndef _RTC_H
#define _RTC_H


#define REG_A_NMI	0x8A
#define RTC_PORT	0x70
#define CMOS_PORT	0x71
#define REG_B_NMI	0x8B
#define PIE_EN		0x40
#define REG_C		0x0C

void rtc_init();
extern void RTC_handler();



#endif
