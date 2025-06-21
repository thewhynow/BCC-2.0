.section .text
.global GDT_load
GDT_load:
    pushl %ebp
    movl %esp, %ebp

    // load GDT
    movl 8(%ebp), %eax
    lgdt (%eax)

    // reload code segment
    movl 12(%ebp), %eax
    pushl %eax
    pushl $.reload_cs
    retf
.reload_cs:
    nop
    addl $8, %esp

    // reload data segments
    movw 16(%ebp), %ax
    movw %ax, %ds
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    movl %ebp, %esp
    pop %ebp
    ret
