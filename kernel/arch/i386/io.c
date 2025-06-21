#include "../../include/kernel/io.h"

void port_write_byte(uint16_t port, uint8_t data){
    asm volatile("outb %0, %1" : : "a" (data), "Nd" (port));
    return;
}

void port_write_word(uint16_t port, uint16_t data){
    asm volatile("outw %0, %1" : : "a" (data), "Nd" (port));
}

void port_write_long(uint16_t port, uint32_t data){
    asm volatile("outl %0, %1" : : "a" (data), "Nd" (port));
}

uint8_t port_read_byte(uint16_t port){
    uint8_t result;
    asm volatile("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

uint16_t port_read_word(uint16_t port){
    uint16_t result;
    asm volatile("inw %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

uint32_t port_read_long(uint16_t port){
    uint32_t result;
    asm volatile("inl %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

void port_write_slow_byte(uint16_t port, uint8_t data){
    asm volatile (
    "outb %0, %1 \n"
    "jmp 1f \n"
    "1: jmp 1f \n"
    "1:"
    : : "a" (data), "Nd" (port)
    );
}

#define UNUSED_PORT 0x80

void io_wait(){
    port_write_byte(UNUSED_PORT, 0);    
}