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
  uint32_t attr;

	/* Set all PDEs and PTEs for the first page table to not present */
	for (i = 0; i < NUM_ENTRIES; i++)
	{
		page_directory[i] = NOT_PRESENT;
    //rework to add in permissions attributes, leave 12 bits per page
		page_table[i] = (FOUR_KB * i) | NOT_PRESENT;
	}
  //set the first attributes
  attr = get_attr('u',7);
  page_directory[0] = attr;
	// Set 4-8MB to the kernel address as a 4MB PDE
	// Set more attributes
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
/*
* get_attr
*   DESCRIPTION: returns the permissions value for a given table
*   INPUTS: table - char stating the desired table to read from
*           d = directory, p = page_table, u = user_table
            check - int to use to specify desired permissions
*   OUTPUTS: none
*   RETURN VALUE: the attr value, -1 if no table found
*   SIDE EFFECTS: none
*/
uint32_t get_attr(char table, int check){
  //use the selected table to aquire attributes
  uint32_t attr;

  switch (table){
    case 'd':
      return 0;
    case 'p':
      attr = (uint32_t)page_table;
      attr = attr | check;
      return attr;
    case 'u':
      attr = (uint32_t)usr_page_table;
      attr = attr | check;
      return attr;
    default:
      return -1;
  }
}
 /*
* page_to_phys
*   DESCRIPTION: maps any given virtual adress to user space page
*                This is done in one chunk (4MB) of mem
*   INPUTS: Vaddr - virtual page address
*           Paddr - physical page address
*   OUTPUTS: none
*   RETURN VALUE: 0 if successfull map, -1 if failed
*   SIDE EFFECTS: writes to user memory space and reads virtual
*/
int page_to_phys(uint32_t Vaddr, uint32_t Paddr){
  uint32_t idx, attr;

  if(Vaddr == NULL || Paddr == NULL){
    return -1;
  }
  //find page index
  idx = Vaddr/KERNEL_ADDR;
  //get user permissions
  attr = get_attr('u',7);
  if(attr == -1){
    return -1;
  }
  page_directory[idx] = attr;
  usr_page_table[0] = Paddr | 7;
  //changed paging, flush tlb
  flush_tlb();
  return 0;
}
 void flush_tlb()
 {
   flush_tlb_asm();
 }
