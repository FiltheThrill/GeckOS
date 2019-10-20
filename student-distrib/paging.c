/* paging.c - Functions to initialize paging
Author - Francis Papa
https://wiki.osdev.org/Paging
 */
 #include "paging.h"
 #include "lib.h"
 
 /* Align page directory and our first page table to 4KB */
 uint32_t page_directory[NUM_ENTRIES] __attribute__((aligned(FOUR_KB)));
 uint32_t page_table[NUM_ENTRIES] __attribute__((aligned(FOUR_KB)));
 
 void paging_init()
 {
	 int i;
	 
	 /* Set all PDEs and PTEs for the first page table to not present */
	 for (i = 0; i < NUM_ENTRIES; i++)
	 {
		 page_directory[i] = NOT_PRESENT;
		 page_table[i] = NOT_PRESENT;
	 }
	 
	 //page_directory[KERNEL_ADDR] = KERNEL_PAGE_INIT;
	 //page_table[VIDMEM_ADDR] = VIDMEM_PAGE_INIT;
	 page_directory[0] = page_table;
	 page_directory[0] |= PRESENT;
	 
	 /* Enable paging */
	 //paging_enable();
	 
 }
 
 void paging_enable()
 {
	asm volatile ("                 		\n\
            movl %0, %%eax     				\n\
            movl %%eax, %%cr3			    \n\
            movl %%cr0, %%eax			    \n\
            orl %1, %%eax	       			\n\
            movl %%eax, %%cr0			    \n\
            movl %%cr4, %%eax			    \n\
            orl %2, %%eax		        	\n\
            movl %%eax, %%cr4		        \n\
            "
			: /* no outputs */
			: "r" (page_directory), "r" (PAGING_EN), "r" (PSE_EN)
			: "eax"
    );
 }
 
 void flush_tlb()
 {
	 asm volatile ("                 		\n\
            movl %%cr3, %%eax  				\n\
            movl %%eax, %%cr3			    \n\
            "
			: /* no outputs */
			: /* no inputs  */
			: "eax"
    );
 }
 