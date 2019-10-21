/* paging.c - Functions to initialize paging
Author - Francis Papa
https://wiki.osdev.org/Paging
https://wiki.osdev.org/Setting_Up_Paging
 */
 #include "paging.h"
 #include "lib.h"
 #include "x86_desc.h"
 #include "paging_asm.h"
 
 /* Align page directory and our first page table to 4KB */
 //uint32_t page_directory[NUM_ENTRIES] __attribute__((aligned(FOUR_KB)));
 //uint32_t page_table[NUM_ENTRIES] __attribute__((aligned(FOUR_KB)));
 
 void paging_init()
 {
	 int i;
	 
	 /* Set all PDEs and PTEs for the first page table to not present */
	 for (i = 0; i < NUM_ENTRIES; i++)
	 {
		 page_directory[i] = NOT_PRESENT;
		 page_table[i] = NOT_PRESENT;
	 }
	 
	 page_directory[1] = KERNEL_ADDR;
	 page_directory[1] |= KERNEL_PAGE_INIT;
	 
	 page_table[(VIDMEM_ADDR/FOUR_KB)] = VIDMEM_ADDR;
	 page_table[(VIDMEM_ADDR/FOUR_KB)] |= VIDMEM_PAGE_INIT;
	 
	 page_directory[0] = page_table;
	 page_directory[0] |= PRESENT;
	 
	 /* Enable paging */
	 paging_enable();
	 
 }
 
 void paging_enable()
 {
	paging_en();
	 
	 
	 
	/*asm volatile ("                 		\n\
            movl %0, %%eax     				\n\
            movl %%eax, %%cr3			    \n\
            movl %%cr0, %%eax			    \n\
            orl %1, %%eax	       			\n\
            movl %%eax, %%cr0			    \n\
            movl %%cr4, %%eax			    \n\
            orl %2, %%eax		        	\n\
            movl %%eax, %%cr4		        \n\
            "
			: 
			: "r" (page_directory), "r" (PAGING_EN), "r" (PSE_EN)
			: "eax"
    );*/
 }
