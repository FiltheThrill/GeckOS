.data
ONE = 1   #because some TA's lack braincells and
TEN = 10  #think any random number is a magic number
NEGONE = -1

.text
.globl syscall_handler



syscall_handler:
    #check if system call is valid
    cmpl $ONE, %eax
    jl INVALID
    cmpl $TEN, %eax
    jl INVALID

    #save all registers
    pushl %eax
    pushl %ebx
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ebp  #is this one needed, who knows i dont care
    pushl %ecx

    pushfl   #push flags? lol idk why not

    #push the three args (first arg is ebx, second is ecx, thrid is edx)
    pushl %edx
    pushl %ecx
    pushl %ebx
    #jump to correct system call based on eax
    addl NEGONE, %eax #jump tables are 0 indexed
    call *call_jump(,%eax,4)
    popl %ebx
    popl %ecx
    popl %edx

    #pop em like pimples
    popfl
    popl %ecx
    popl %ebp
    popl %edx
    popl %esi
    popl %edi
    popl %ebx
    popl %eax
    iret

  INVALID: # iret that bih, its wrong
    movl $NEGONE, %eax
    iret

call_jump:
.long halt, execute, read, write, open, close, get_args, vidmap, set_handler, sigreturn
.end
