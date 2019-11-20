.data
NEGONE = -1

.text
.globl syscall_handler

syscall_handler:
    # movl %eax, %eax
    cmpl $1, %eax # check if system call is valid
    jl INVALID
    cmpl $10, %eax
    ja INVALID

    # save all registers
    pushl %ebx
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ebp  # is this one needed, who knows i dont care
    pushl %ecx

    # 'push the three args (first arg is ebx, second is ecx, thrid is edx)
    pushl %edx
    pushl %ecx
    pushl %ebx

    # jump to correct system call based on eax
    addl $NEGONE, %eax #jump tables are 0 indexed
    call *call_jump(,%eax,4)
    popl %ebx
    popl %ecx
    popl %edx

    # 'pop em like pimples
    popl %ecx
    popl %ebp
    popl %edx
    popl %esi
    popl %edi
    popl %ebx
    iret

  INVALID: # iret that bih, its wrong
    movl $NEGONE, %eax
    iret


call_jump:
.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
.end
