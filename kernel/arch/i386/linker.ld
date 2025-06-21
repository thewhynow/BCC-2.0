OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS {
    . = 0xC0100000;
    
    _begin = . - 0xC0000000;

    .text : AT(ADDR(.text)-0xC0000000){
        . = ALIGN(4);
        LONG(0x1BADB002) /* multiboot magic */
        LONG(0x00000003) /* multiboot flags */
        LONG(0xE4524FFB) /* multiboot checksum = -(magic + flags) */
        *(.text)
        *(.rodata)
        . = ALIGN(4096);
    }

    .data : AT(ADDR(.data)-0xC0000000){
        *(.data)
        . = ALIGN(4096);
    }

    .rodata : AT(ADDR(.rodata)-0xC0000000){
        *(.rodata)
    }

    .bss : AT(ADDR(.bss)-0xC0000000){
        *(.bss)
        . = ALIGN(4096);
    }

    _end = .;
}