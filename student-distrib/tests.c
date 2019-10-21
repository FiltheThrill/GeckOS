#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

#define INBOUND	0xB8000
#define OUTBOUND	0x800000

#define PASS 1
#define FAIL 0
//enablers for tests
#define DIVTEST 0
#define PAGETEST 0
#define DEREFTEST 0
#define VALIDPAGETEST 1

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
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
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
}
