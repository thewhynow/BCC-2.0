#include "../include/stdio.h"
#include "../../kernel/include/kernel/tty.h"

int putchar(int ic){
    char c = (char)ic;
    #ifdef _KERNEL_LIBC
    terminal_putchar(c);
    #else
    /* implement stdout */
    #endif
    return ic;
}