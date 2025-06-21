#include "../../include/kernel/irq.h"

IRQ_handler_t IRQ_handlers[16] = {
    [0] = IRQ_time_handler,
    [1] = IRQ_keyboard_handler,
    [6] = IRQ_FDC_handler,
};

void IRQ_handler(registers_t* regs){
    int irq = regs->int_num;

    IRQ_handler_t handler = IRQ_handlers[irq];

    if (handler)
        handler();
    else {
        printf("Unhandled Hardware Interrupt: %i\n", irq);
        PIC_end_of_int(irq);
    }
}

void IRQ0();
void IRQ1();
void IRQ2();
void IRQ3();
void IRQ4();
void IRQ5();
void IRQ6();
void IRQ7();
void IRQ8();
void IRQ9();
void IRQ10();
void IRQ11();
void IRQ12();
void IRQ13();
void IRQ14();
void IRQ15();

/* in the future should also unmask IRQ0 */
void IRQ_timer_init(){
    PIT_set_freq(1);
}

void IRQ_init() { 
    PIC_configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8);
    IRQ_timer_init();

    {
        IDT_setgate(PIC_REMAP_OFFSET + 0, IRQ0, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 1, IRQ1, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 2, IRQ2, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 3, IRQ3, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 4, IRQ4, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 5, IRQ5, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 6, IRQ6, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 7, IRQ7, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 8, IRQ8, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 9, IRQ9, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 10, IRQ10, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 11, IRQ11, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 12, IRQ12, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 13, IRQ13, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 14, IRQ14, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
        IDT_setgate(PIC_REMAP_OFFSET + 15, IRQ15, GDT_CODE_SEGMENT, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
    }

    asm volatile ("sti");
}