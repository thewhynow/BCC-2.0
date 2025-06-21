#include "../../libc/include/stdio.h"
#include "../../libc/include/stdlib.h"
#include "../include/kernel/tty.h"
#include "../include/kernel/gdt.h"
#include "../include/kernel/idt.h"
#include "../include/kernel/isr.h"
#include "../include/kernel/irq.h"
#include "../include/kernel/pit.h"
#include "../include/kernel/pmm.h"
#include "../include/kernel/vmm.h"
#include "../include/kernel/fdc.h"

void kernel_main(){
    terminal_init();
    printf("Loading GDT...");
    GDT_init();
    printf("GDT Loaded!\n");
    printf("Loading IDT...");
    IDT_init();
    printf("IDT Loaded!\n");
    printf("Loading ISR...");
    ISR_init();
    printf("ISR Loaded!\n");
    printf("Loading IRQ...");
    IRQ_init();
    printf("IRQ Loaded!\n");
    printf("Loading PMM...");
    PMM_init();
    printf("PMM Loaded!\n");
    printf("Loading VMM...");
    VMM_init();
    printf("VMM Loaded!\n");
    printf("Loading FDC...");
    // FDC_init();
    printf("FDC Loaded!\n");

    printf("Welcome to lakeOS!\n");

    printf("Dumping Sector 0...\n");

    uint8_t *sector = FDC_read_sector(0);

    if (sector) {
        for (int i = 0; i < 512; i += 4)
            if (sector[i])
                printf("%u", sector[i]);
    } 
    else 
        printf("fuck!\n");

    char* string;

    string = alloc_page();
    string = vmm_map_page(string, string + 0xC0000000);

    while (1) {
        memset(string, 0, 100);
        gets(string);
        printf("string: %s\n", string);

        if (!memcmp(string, "quit", 4))
            for (;;);
    }

    for (;;);
}