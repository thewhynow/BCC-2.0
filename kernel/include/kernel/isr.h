#ifndef _ISR_H
#define _ISR_H

#include "../../../libc/include/types.h"
#include "../../../libc/include/stdio.h"
#include "idt.h"
#include "gdt.h"

typedef struct {
    // in reverse order they are pushed
    uint32_t ds; // datasegment pushed by us 
    uint32_t edi, esi, ebp, kern_esp, ebx, edx, ecx, eax; // general purpose register saved with "pusha"
    uint32_t int_num, error; // pushed int_num and error code
    uint32_t eip, cs, eflags, esp, ss; // pushed by CPU when issuing interrupt

} __attribute__((__packed__)) registers_t;

#ifdef __cplusplus
extern "C" {
#endif
void ISR_init();
#ifdef __cplusplus
}
#endif


typedef void(*ISR_handler_t)(registers_t* regs);

#endif