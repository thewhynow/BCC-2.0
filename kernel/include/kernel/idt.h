#ifndef _IDT_H
#define _IDT_H

#include "../../../libc/include/types.h"


typedef enum {
    IDT_FLAG_GATE_TASK       = 0x5,
    IDT_FLAG_GATE_16BIT_INT  = 0x6,
    IDT_FLAG_GATE_16BIT_TRAP = 0x7,
    IDT_FLAG_GATE_32BIT_INT  = 0xE,
    IDT_FLAG_GATE_32BIT_TRAP = 0xF,

    IDT_FLAG_RING0           = (0 << 5),
    IDT_FLAG_RING1           = (1 << 5),
    IDT_FLAG_RING2           = (2 << 5),
    IDT_FLAG_RING3           = (3 << 5),

    IDT_FLAG_PRESENT         = 0x80,
} IDT_FLAGS;

typedef struct {
    uint16_t base_low;
    uint16_t segment_select;
    uint8_t reserved;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((__packed__)) IDT_entry_t;

typedef struct {
    uint16_t limit;
    IDT_entry_t* ptr;
} __attribute__((__packed__)) IDT_descriptor_t;

#ifdef __cplusplus
extern "C" {
#endif
void IDT_init();
void IDT_setgate(int int_num, void(*_base)(), uint16_t segment_descriptor, uint8_t flags);
extern IDT_descriptor_t IDT_descriptor;
void IDT_enablegate(int int_num);
void IDT_disablegate(int int_num);
#ifdef __cplusplus
}
#endif

#endif