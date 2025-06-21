#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
void terminal_init();
void terminal_putchar(char c);
void terminal_write(const char* str, size_t size);
void terminal_print(const char* str);
void terminal_delchar();
void terminal_update_cursor();
void port_write_byte(unsigned short port, unsigned char data);
#ifdef __cplusplus
}
#endif


#endif
