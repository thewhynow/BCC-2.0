#include "../../include/kernel/gdt.h"

typedef struct {
    uint16_t limit_low;      /* limit: bits 0-15 */
    uint16_t base_low;       /* base:  bits 0-15 */
    uint8_t base_middle;     /* base:  bits 16-23 */
    uint8_t type;            /* access */
    uint8_t flags__limit_hi; /* limit: bits 16-19 | flags */
    uint8_t base_high;       /* base: bits 24-31 */
}  __attribute__((__packed__)) GDT_entry_t;

typedef struct {
    uint16_t limit;          /* sizeof(GDT_t) - 1 */
    GDT_entry_t* gdt_p;      /* &GDT */ 
} __attribute__((__packed__)) GDT_descriptor_t;

typedef enum {
    GDT_ACCESS_CODE_READABLE = 0x02,
    GDT_ACCESS_DATA_WRITEABLE = 0x02,
    
    GDT_ACCESS_CODE_CONFORMING = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN = 0x04,

    GDT_ACCESS_DATA_SEGMENT = 0x10,
    GDT_ACCESS_CODE_SEGMENT = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS = 0x00,

    GDT_ACCESS_RING0 = 0x00,
    GDT_ACCESS_RING1 = 0x20,
    GDT_ACCESS_RING2 = 0x40,
    GDT_ACCESS_RING3 = 0x60,

    GDT_ACCESS_PRESENT = 0x80,

} GDT_ACCESS_FLAGS;

typedef enum {
    GDT_FLAG_64BIT = 0x20,
    GDT_FLAG_32BIT = 0x40,
    GDT_FLAG_16BIT = 0x00,

    GDT_FLAG_GRANULARITY_1B = 0x00,
    GDT_FLAG_GRANULARITY_4K = 0x80,
} GDT_FLAGS;

#define GDT_ENTRY(base, limit, access, flags)   \
    (GDT_entry_t){                              \
        limit & 0xFFFF,                         \
        base & 0xFFFF,                          \
        (base >> 16) & 0xFF,                    \
        access,                                 \
        ((limit >> 16) & 0xF) | (flags & 0xF0), \
        (base >> 24) & 0xFF                     \
    }

void GDT_load(GDT_descriptor_t* descriptor, uint16_t code_seg, uint16_t data_seg);

GDT_entry_t GDT_entries[3] = {
    GDT_ENTRY(0, 0, 0, 0),

    /* Kernel 32-bit code segment */
    GDT_ENTRY(0, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
              (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)),
    /* Kernel 32-bit data segment */
    GDT_ENTRY(0, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
              (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)),
};

GDT_descriptor_t GDT_descriptor = (GDT_descriptor_t){
    .limit = sizeof(GDT_entries) - 1,
    .gdt_p = (void*)&GDT_entries
};

void GDT_init(){
    GDT_load(&GDT_descriptor, GDT_CODE_SEGMENT, GDT_DATA_SEGMENT);
}