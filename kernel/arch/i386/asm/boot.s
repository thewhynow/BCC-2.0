/* constants for multiboot header */
.set ALIGN,    1<<0             /* aligns modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set GRAPHICS, 0                /* dont use graphics */
.set FLAGS,    ALIGN | MEMINFO  /* multiboot flag field */
.set MAGIC,    0x1BADB002       /* magic number that lets bootloader find header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above */

/* 
    declare multiboot header - makes program the kernel
    bootloader searches for this signature in the first 8KiB
    of the kernel file, aligned to 4 bytes
*/


.set PAGE_STRUCT_PRESENT,   0b00000000000000000000000000000001
.set PAGE_STRUCT_WRITEABLE, 0b00000000000000000000000000000010
.set PAGE_STRUCT_FLAGS,     0b00000000000000000000000000000011

.section .data

.align 4096
.global page_directory
page_directory:
    .skip 4096

/* identity map first 4MB */
.align 4096
identity_page_table:
    .set p_addr, 0
    .rept 1024
        .long p_addr | (PAGE_STRUCT_FLAGS)
        .set p_addr, p_addr + 4096
    .endr

/* map 0MB -> 3GB */
.align 4096
.global higher_half_page_table
higher_half_page_table:
    .set p_addr, 0
    .rept 1024
        .long p_addr | (PAGE_STRUCT_FLAGS)
        .set p_addr, p_addr + 4096
    .endr

.section .text
/* entry point of kernel */
.global _start
.type _start, @function
/* set up paging */
_start:
    movl $page_directory, %eax
    subl $0xC0000000, %eax
    
    movl $identity_page_table, %ecx
    subl $0xC0000000, %ecx
    orl $PAGE_STRUCT_FLAGS, %ecx
    movl %ecx, (%eax)
    
    movl $higher_half_page_table, %ecx
    subl $0xC0000000, %ecx
    orl $PAGE_STRUCT_FLAGS, %ecx
    movl %ecx, 3072(%eax)

    movl %eax, %cr3

    /* enable paging */
    movl %cr0, %eax
    orl $0b10000000000000000000000000000000, %eax
    movl %eax, %cr0

    /* jump to higher half */
    movl $_higher_half, %eax
    jmp *%eax

/*
    create a "stack" -> just static memory
    since stack grows downwards, lowest address = bottom of stack
*/
.section .bss
    .align 16 // stack must be 16-byte aligned according to System-V
    stack_bottom:
        .skip 16384 // 16 KiB
    stack_top:

.section .text
_higher_half:
    /* 
    set up the "stack" with the stack pointer and 
    base pointer that will be used by C programs
    */

    movl $stack_top, %esp // use 32-bit assembly, no lea required here
    movl %esp, %ebp

    cli // disable interrupts

    movl %ebx, multiboot_info // save the multiboot info

    call kernel_main // hand over control to the C portion, everything initialized

    /*
    if the system has nothing more to do, infinite loop:
        1) disable interrupts with cli (clear interrupt enable in eflags)
        2) halt the CPU until next interrupt ... which will never come
        3) if the interrupt does somehow come go back to the halt
    */

    cli
hlt:hlt
    jmp hlt

/*
apparently useful for debugging later
*/
.size _start, . - _start
