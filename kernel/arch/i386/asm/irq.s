.section .text

.macro IMPL_IRQ num
.global IRQ\num
IRQ\num:
    push $\num
    jmp common_irq
.endm


common_irq:
    pusha // pushes all general-purpose registers

    xorl %eax, %eax
    movw %ds, %ax
    pushl %eax

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    
    pushl %esp
    call IRQ_handler
    addl $4, %esp

    popl %eax // restore old segment
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    popa
    addl $4, %esp // remove error code and irq_num from stack
    iret

IMPL_IRQ 0
IMPL_IRQ 1
IMPL_IRQ 2
IMPL_IRQ 3
IMPL_IRQ 4
IMPL_IRQ 5
IMPL_IRQ 6
IMPL_IRQ 7
IMPL_IRQ 8
IMPL_IRQ 9
IMPL_IRQ 10
IMPL_IRQ 11
IMPL_IRQ 12
IMPL_IRQ 13
IMPL_IRQ 14
IMPL_IRQ 15
