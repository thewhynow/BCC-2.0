#ifndef _PIT_H
#define _PIT_H

#include "io.h"
#include "idt.h"
#include "irq.h"

#ifdef __cplusplus
extern "C" {
#endif
void PIT_set_freq(int hz);
void PIT_enable();
void PIT_disable();
void IRQ_time_handler();
#ifdef __cplusplus
}
#endif


#endif