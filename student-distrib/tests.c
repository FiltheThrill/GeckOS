#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "files.h"
#include "types.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"

#define INBOUND	0xB8000
#define OUTBOUND	0x800000

#define PASS 1
#define FAIL 0
//enablers for tests
#define DIVTEST 0
#define PAGETEST 0
#define DEREFTEST 0
#define VALIDPAGETEST 0
#define READFRAME1 0
#define READFRAME0 0
#define READLARGE 0
#define READCAT   0
#define READTEST  0
#define READDIR 0
#define RTCTEST 0
#define KEYTEST 0
#define TERMTEST 0
#define COMMENTFILETEST 0
/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}

/* Divide By Zero Test
 *
 * Asserts that you cannot divide by 0
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: None
 * Coverage: Divide by Zero Exception
 * Files: IDT.c/h
 */
int div_by_0_test()
{
	TEST_HEADER;
	int i;
	int result = FAIL;
	int zero = 0;
	i = 1/zero;
	return result;
}
// add more tests here
/* Page Fault Test
 *
 * Asserts that we cannot access a page thats not present
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: None
 * Coverage: Page Fault Exception
 * Files: IDT.c/h paging.c/h
 */
int page_fault_test()
{
	TEST_HEADER;
	int result = FAIL;
	uint32_t page = page_directory[OUTBOUND];
	page = page + 1;
	return result;
}

/* Valid Page Test
 *
 * Asserts that you can access a page present in the page directory
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: Paging
 * Files: paging.c/h
 */
int page_inbounds_test()
{
	TEST_HEADER;
	int result = PASS;
	uint32_t page = page_directory[INBOUND];
	int i = page;
	i++;
	return result;
}

/* Dereference NULL Test
 *
 * Asserts that you cannot dereference a NULL pointer
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: None
 * Coverage: Page Fault Exception
 * Files: IDT.c/h paging.c/h
 */
int deref_null_test()
{
	TEST_HEADER;
	int result = FAIL;
	int * ptr = NULL;
	int i = *ptr;
	i++;
	return result;
}

/* Checkpoint 2 tests */
#if (COMMENTFILETEST == 1)

/* read frame 1 Test
 *
 * prints frame1.txt to screen
 * Inputs: None
 * Outputs: none
 * Side Effects: None
 * Coverage:reading txt files
 * Files: files.c/h
 */
void read_frame1()
{
	int result = PASS;
	const uint8_t* fname = (const uint8_t*)"frame1.txt";
	int32_t check;
	uint8_t buf[185];
	int bytes;
	check = fopen(fname);
	if(check == -1)
	{
		printf("unable to open file");
		result = FAIL;
	}
	bytes = fread(buf, 0, 185);
	puts((int8_t*)buf);
	fclose(fname);
}

/* read frame 0 Test
 *
 * prints frame0.txt to screen
 * Inputs: None
 * Outputs: none
 * Side Effects: None
 * Coverage:reading txt files
 * Files: files.c/h
 */
void read_frame0()
{
	int result = PASS;
	const uint8_t* fname = (const uint8_t*)"frame0.txt";
	int32_t check;
	uint8_t buf[198];
	int bytes;
	check = fopen(fname);
	if(check == -1)
	{
		printf("unable to open file");
		result = FAIL;
	}
	bytes = fread(buf, 0, 198);
	puts((int8_t*)buf);
	fclose(fname);

}

/* read very large Test
 *
 * prints verylargetextwithverylongname.txt to screen
 * Inputs: None
 * Outputs: none
 * Side Effects: None
 * Coverage:reading large txt files
 * Files: files.c/h
 */
void read_large()
{
	int result = PASS;
	const uint8_t* fname = (const uint8_t*)"verylargetextwithverylongname.txt";
	int32_t check;
	uint8_t buf[5305];
	int bytes;
	check = fopen(fname);
	if(check == -1)
	{
		printf("unable to open file");
		result = FAIL;
	}
	bytes = fread(buf, 0, 5305);
	puts((int8_t*)buf);
	fclose(fname);
}

/* read cat Test
 *
 * prints cat to screen
 * Inputs: None
 * Outputs: none
 * Side Effects: None
 * Coverage:reading non txt files
 * Files: files.c/h
 */
void read_cat()
{
	int result = PASS;
	const uint8_t* fname = (const uint8_t*)"cat";
	int32_t check;
	uint8_t buf[445];
	int bytes;
	int i;
	check = fopen(fname);
	if(check == -1)
	{
		printf("unable to open file");
		result = FAIL;
	}

	bytes = fread(buf, 5000, 445);
	for(i = 0; i < bytes; i++)
	{
		putc(buf[i]);
	}
	fclose(fname);
}

/* read testprint Test
 *
 * prints testprint to screen
 * Inputs: None
 * Outputs: none
 * Side Effects: None
 * Coverage:reading non txt files
 * Files: files.c/h
 */
void read_test()
{
	int result = PASS;
	int32_t fd = 1;

	const uint8_t* fname = (const uint8_t*)"testprint";
	int32_t check;
	uint8_t buf[445];
	int bytes;
	int i;
	check = fopen(fname);
	if(check == -1)
	{
		printf("unable to open file");
		result = FAIL;
	}

	bytes = fread(buf, 5151, 445);
	for(i = 0; i < bytes; i++)
	{
		//term_putc(0,buf[i]);
	}
	printf("attempting");
	term_write(fd, buf, bytes);
	fclose(fname);
}

/* read directory Test
 *
 * prints directory contents to screen
 * Inputs: None
 * Outputs: none
 * Side Effects: None
 * Coverage:reading directory
 * Files: files.c/h
 */
void read_dir()
{
	int result = PASS;
	const uint8_t* fname = (const uint8_t*)".";
	int32_t check;
	uint8_t buf[32];
	int bytes;
	int i, j;
	check = dopen(fname);
	if(check == -1)
	{
		printf("unable to open directory");
		result = FAIL;
	}
	for(i = 1; i < 17; i++)
	{
		bytes = dread(buf, i);
		for(j = 0; j < bytes; j++)
		{
			putc(buf[j]);

		}
		putc('\n');
	}
	dclose(fname);
}
#endif

/*
* rtc_tests
 * Asserts that the rtc frequency can be changed
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: rtc_read/write/open/close
 * Files: rtc.c/h
 */
int rtc_tests()
{
	TEST_HEADER;
	int result = PASS;
	int i, j;

	// random values not used
	const uint8_t* fname = (const uint8_t*)"rtc_f";
	// check if we are successfull in all our operations
	int32_t check_open, check_close, check_read, check_write;
	check_open = rtc_open(fname);
	if (check_open == -1)
	{
		printf("unable to open rtc");
		result = FAIL;
	}

	// more garbage vars
	uint8_t buf[1];
	int bytes;

	// clear the screen for the test
	clear();

	// since we opened the rtc the freq = 2Hz
	// test this by printing 15 ones when the next interrupt occurs (rtc_read returns)
	for (i = 0; i < 15; i++)
	{
		check_read = rtc_read(0, buf, bytes);
		printf("1");
	}
	printf("\n");

	//test 4 Hz, then 8, then 16, then 32, then 64, then ... 1024

	for (i = 4; i < 2048; i *= 2)
	{
		//set new frequency
		check_write = rtc_write(0, buf, i);
		for (j = 0; j < 15; j++)
		{
			//print 15 1s at this frequency using rtc_read to block printing until next interrupt
			check_read = rtc_read(0, buf, bytes);
			printf("1");
		}
		printf("\n");
	}

	// close the rtc (do nothing)
	check_close = rtc_close(check_open);
	if (check_close == -1)
	{
		printf("unable to close rtc");
		result = FAIL;
	}

	// return PASS!
	return result;
}

//blank handler to allow for typing
void key_test(){
	term_clear(0,0);
	return;
}

void term_test(){
	int32_t fd = 0;
	uint8_t buf[128] = "hello";
	int32_t nbytes = 128;
	int32_t write;

	//term_clear(0,0);
	//printf("hello");
	//write = term_read(fd,buf,nbytes);
	term_write(fd,buf,200);
}
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	#if (DIVTEST == 1)
		TEST_OUTPUT("Divide by 0 test", div_by_0_test());
	#endif
	// launch your tests here
	#if (PAGETEST == 1)
		TEST_OUTPUT("Page Fault test", page_fault_test());
	#endif
	#if (VALIDPAGETEST == 1)
		TEST_OUTPUT("Valid Page test", page_inbounds_test());
	#endif
	#if (DEREFTEST == 1)
		TEST_OUTPUT("Dereference NULL test", deref_null_test());
	#endif
	#if (READFRAME1 == 1)
			read_frame1();
	#endif
	#if (READFRAME0 == 1)
			read_frame0();
	#endif
	#if (READLARGE == 1)
			read_large();
	#endif
	#if (READCAT == 1)
			read_cat();
	#endif
	#if (READTEST == 1)
			read_test();
	#endif
	#if (READDIR == 1)
			read_dir();
	#endif
	#if (RTCTEST == 1)
		TEST_OUTPUT("Test our RTC", rtc_tests());
	#endif
	#if (KEYTEST == 1)
		key_test();
	#endif
	#if (TERMTEST == 1)
		term_test();
	#endif
}
