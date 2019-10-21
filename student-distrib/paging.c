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
 
 /*
* paging_init
*   DESCRIPTION: initializes paging
*   INPUTS: none
*   OUTPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: enables paging and fills the kernel page and video memory pages in the page directory/table
*/
 void paging_init()
 {
	 int i;
	 
	 /* Set all PDEs and PTEs for the first page table to not present */
	 for (i = 0; i < NUM_ENTRIES; i++)
	 {
		 page_directory[i] = NOT_PRESENT;
		 page_table[i] = NOT_PRESENT;
	 }
	 
	 // Set 4-8MB to the kernel address as a 4MB PDE
	 // Set the attributes
	 page_directory[1] = KERNEL_ADDR;
	 page_directory[1] |= KERNEL_PAGE_INIT;
	 
	 //Set the Video mem page in the first page table at video mem address / 4kb
	 //Set attributes
	 page_table[(VIDMEM_ADDR/FOUR_KB)] = VIDMEM_ADDR;
	 page_table[(VIDMEM_ADDR/FOUR_KB)] |= VIDMEM_PAGE_INIT;
	 
	 //Set the vid mem page table to the first 4MB of the page directory and mark it present
	 page_directory[0] = (uint32_t)page_table;
	 page_directory[0] |= PRESENT;
	 
	 /* Enable paging */
	 paging_en();
	 
 }
 
