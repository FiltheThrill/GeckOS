#ifndef _PAGING_H
#define _PAGING_H

#define NUM_ENTRIES	1024
#define KB_FOUR	4096
#define PAGING_EN 0x80000001
#define PSE_EN	0x00000010
#define NOT_PRESENT	0x00000002
#define KERNEL_ADDR	0x400000
#define VIDMEM_ADDR 0xB8000
#define KERNEL_PAGE_INIT 0x00000193
#define VIDMEM_PAGE_INIT 0x00000003

void paging_init();
void paging_enable();
void flush_tlb();







#endif