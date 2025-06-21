#include "../../include/kernel/pit.h"
#define TIMER_IRQ 0x0

void PIT_set_freq(int hz){
    int divisor = 1193180 / hz;
    port_write_byte(0x43, 0x36);
    port_write_byte(0x40, divisor & 0xFF);
    port_write_byte(0x40, divisor >> 8);
}

void IRQ_time_handler(){
    PIC_end_of_int(TIMER_IRQ);
    static unsigned long sys_uptime;
    ++sys_uptime;

    // printf("time: %i\n", sys_uptime);
}

void PIT_enable(){
    PIC_unmask(TIMER_IRQ);
}

void PIT_disable(){
    PIC_mask(TIMER_IRQ);
}