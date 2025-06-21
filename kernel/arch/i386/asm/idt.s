.section .text
.global IDT_load
IDT_load:
    pushl %ebp
    movl %esp, %ebp

    movl 8(%ebp), %eax
    lidt (%eax)

    movl %ebp, %esp
    popl %ebp
    ret
