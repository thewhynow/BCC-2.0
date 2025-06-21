#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include "../include/stdio.h"
#include "../include/string.h"

static bool print(const char* data, size_t length){
    for (size_t i = 0; i < length; ++i)
        if (putchar(data[i]) == EOF)
            return false;
    return true;
}

static bool printint(int n){
    char buff[10];
    
    int i = 0;

    if (n < 0){
        n = -n;
        if (putchar('-') == EOF)
            return false;
    }

    do {
        buff[i++] = '0' + (n % 10);
        n /= 10;
    } while(n);

    while (i--)
        if(putchar(buff[i]) == EOF)
            return false;

    return true;
}

static bool printlong(long n){
    char buff[30];
    
    int i = 0;

    do {
        buff[i++] = '0' + ((unsigned long)n % 10);
        n /= 10;
    } while(n);

    while (i--)
        if(putchar(buff[i]) == EOF)
            return false;

    return true;
}

int printf(const char* restrict format, ...){
    va_list params;
    va_start(params, format);

    int bytes_written = 0;

    while (*format){
        if (*format == '%'){
            switch(*++format){
                case 'c': {
                    putchar(va_arg(params, int));

                    ++bytes_written;

                    break;
                }
                
                case 's': {
                    const char* str = va_arg(params, const char*);
                    size_t len = strlen(str);

                    print(str, len);

                    bytes_written += len;

                    break;
                }

                case 'i': {
                    int i = va_arg(params, int);
                    
                    printint(i);

                    do {
                        ++bytes_written;
                        i /= 10;
                    } while(i);

                    break;
                }

                case 'p': {
                    long p = va_arg(params, long);
                    printlong(p);

                    do {
                        ++bytes_written;
                        p /= 10;
                    } while(p);

                    break;
                }

                default: {
                    putchar(*format);
                    ++bytes_written;

                    break;
                }
            }
            ++format;
        }
        else {
            putchar(*format++);
        }
    }

    va_end(params);

    return bytes_written;
}