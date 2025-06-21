#include "../include/stdio.h"
#include "../include/string.h"

#ifdef _KERNEL_LIBC
#include "../../kernel/include/kernel/tty.h"
#include "../../kernel/include/kernel/ps2.h"

char* gets(char* str){
    char* buff = PS2_read();
    terminal_putchar('\n');

    size_t len = strlen(buff);
    memcpy(str, buff, len);

    memmove(buff, buff + len, PS2_STDIN_SIZE - len);

    return str;
}
#else

#endif
