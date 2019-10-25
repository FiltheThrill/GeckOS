#ifndef _RTC_H
#define _RTC_H


#define REG_A_NMI	0x8A
#define RTC_PORT	0x70
#define CMOS_PORT	0x71
#define REG_B_NMI	0x8B
#define PIE_EN		0x40
#define REG_C		0x0C

#define RTC_SUCCESS	0
#define RTC_FAIL	-1

void rtc_init();
extern void RTC_handler();

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);

#endif
