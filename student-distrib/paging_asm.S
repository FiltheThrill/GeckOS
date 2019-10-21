#define ASM     1
#include "paging_asm.h"

.global paging_en

paging_en:
	pushl %ebp
	movl %esp, %ebp
	pushl %eax
	movl page_directory, %eax
    movl %eax, %cr3
	movl %cr4, %eax
    orl $PSE_EN, %eax
    movl %eax, %cr4
    movl %cr0, %eax
    movl $PAGING_EN, %eax
    movl %eax, %cr0
	popl %eax
	leave
	ret