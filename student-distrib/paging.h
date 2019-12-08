#ifndef _PAGING_H
#define _PAGING_H

//all defs in x86_desc.h
#include "types.h"

extern int term_page(int tnum, uint32_t* addr);
extern void paging_init(); //initializes paging
extern void flush_tlb();
extern int page_to_phys(uint32_t Vaddr, uint32_t Paddr); //copies a page to user mem
extern int page_remap(uint32_t Vaddr, uint32_t Paddr); //sets up paging after halts
extern uint32_t get_attr(char table, int check); //gets the permissions for a given table
#endif
