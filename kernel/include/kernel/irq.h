#ifndef _IRQ_H
#define _IRQ_H

#include "pic.h"
#include "isr.h"
#include "idt.h"
#include "pit.h"
#include "tty.h"
#include "pit.h"
#include "ps2.h"
#include "fdc.h"

typedef void (*IRQ_handler_t)();

#ifdef __cplusplus
extern "C" {
#endif
extern void kernel_panic();

void IRQ_init();
#ifdef __cplusplus
}
#endif


#define PIC_REMAP_OFFSET 0x20

#endif