#ifndef _IO_H
#define _IO_H

#include "../../../libc/include/types.h"

#ifdef __cplusplus
extern "C" {
#endif
void port_write_byte(uint16_t port, uint8_t data);
void port_write_word(uint16_t port, uint16_t data);
void port_write_long(uint16_t port, uint32_t data);

uint8_t port_read_byte(uint16_t port);
uint16_t port_read_word(uint16_t port);
uint32_t port_read_long(uint16_t port);

void port_write_slow_byte(uint16_t port, uint8_t data);

void io_wait();
#ifdef __cplusplus
}
#endif


#endif 