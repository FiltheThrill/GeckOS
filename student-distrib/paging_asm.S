#define ASM     1
#include "paging_asm.h"

.global paging_en
.global flush_tlb_asm

# paging_en:
# Puts the page directory into the PDBR which is stored in cr3
# Enables paging and page size extension
paging_en:
	pushl %ebp
	movl %esp, %ebp

	# Save eax which is clobbered
	pushl %eax
	# Move the page directory into cr3
	movl $page_directory, %eax
    movl %eax, %cr3
	# Enable page size extension in cr4
	movl %cr4, %eax
    orl $PSE_EN, %eax
    movl %eax, %cr4
	# Enable paging in cr0
    movl %cr0, %eax
    orl $PAGING_EN, %eax
    movl %eax, %cr0
	popl %eax

	leave
	ret

flush_tlb_asm:

		movl %cr0, %eax
		movl %eax, %cr0

		ret
