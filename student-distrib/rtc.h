#ifndef _RTC_H
#define _RTC_H

#define REG_A_NMI	0x8A
#define RTC_PORT	0x70
#define CMOS_PORT	0x71
#define REG_B_NMI	0x8B
#define PIE_EN		0x40
#define REG_C		0x0C
#define FREQ_BITMASK	0xF0

#define HZ_0		0x00
#define HZ_2		0x0F
#define HZ_4		0x0E
#define HZ_8		0x0D
#define HZ_16		0x0C
#define HZ_32		0x0B
#define HZ_64		0x0A
#define HZ_128		0x09
#define HZ_256		0x08
#define HZ_512		0x07
#define HZ_1024		0x06

#define RTC_SUCCESS	0
#define RTC_FAIL	-1

void rtc_init();
extern void RTC_handler();

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);

#endif
