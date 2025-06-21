#include "../../include/kernel/idt.h"

IDT_entry_t IDT[256];

IDT_descriptor_t IDT_descriptor = (IDT_descriptor_t){
    .limit = sizeof(IDT) - 1,
    .ptr = (void*)&IDT
};

void IDT_setgate(int int_num, void(*_base)(), uint16_t segment_descriptor, uint8_t flags){
    uint32_t base = (uint32_t)_base;

    IDT[int_num] = (IDT_entry_t){
        .base_low = base & 0xFFFF,
        .base_high = (base >> 16) & 0xFFFF,
        .segment_select = segment_descriptor,
        .reserved = 0,
        .flags = flags
    };
}

void IDT_enablegate(int int_num){
    IDT[int_num].flags |= IDT_FLAG_PRESENT;
}

void IDT_disablegate(int int_num){
    IDT[int_num].flags &= ~IDT_FLAG_PRESENT;
}

void IDT_load(IDT_descriptor_t* descriptor);

void IDT_init(){
    IDT_load(&IDT_descriptor);
}