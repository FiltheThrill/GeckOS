#ifndef _PAGING_ASM_H
#define _PAGING_ASM_H

#include "x86_desc.h"


#ifndef ASM

extern void paging_en();

extern void flush_tlb_asm();
#endif /* ASM */

#endif /* _PAGING_ASM_H */
