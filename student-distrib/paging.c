/* paging.c - Functions to initialize paging
Author - Francis Papa
https://wiki.osdev.org/Paging
 */
 #include "paging.h"
 
 /* Align page directory and our first page table to 4KB */
 uint32_t page_directory[NUM_ENTRIES] __attribute__((aligned(KB_FOUR)));
 uint32_t page_table[NUM_ENTRIES] __attribute__((aligned(KB_FOUR)));
 
 void paging_init()
 {
	 int i;
	 
	 /* Enable paging */
	 paging_enable();
	 
	 /* Set all PDEs and PTEs for the first page table to not present */
	 for (i = 0; i < NUM_ENTRIES; i++)
	 {
		 page_directory[i] = NOT_PRESENT;
		 page_table[i] = NOT_PRESENT;
	 }
	 
	 page_directory[KERNEL_ADDR] = KERNEL_PAGE_INIT;
	 page_table[VIDMEM_ADDR] = VIDMEM_PAGE_INIT;
	 page_directory[0] = page_table;
	 
 }
 
 void paging_enable()
 {
	 asm volatile (
        "movl page_directory, %%eax							;"
       	"movl %%eax, %%cr3									;"
       	"movl %%cr0, %%eax									;"
		"orl $PAGING_EN, %%eax								;"
		"movl %%eax, %%cr0									;"
		"movl %%cr4, %%eax									;"
		"orl $PSE_EN, %%eax									;"
		"movl %%eax, %%cr4									;"
      : /* no outputs */
      : 
      :
    );
 }
 
 void flush_tlb()
 {
	 asm volatile (
		"movl %%cr3, %%eax									;"
		"movl %%eax, %%cr3									;"
		:
		:
		:
	 );
 }