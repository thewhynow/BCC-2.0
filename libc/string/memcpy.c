#include "../include/string.h"
#include "../include/types.h"

void* memcpy(void* restrict dst, const void* restrict src, size_t len){
    while (len >= 4){
        *(uint32_t*)dst = *(uint32_t*)src; 
        len -= 4;

        dst += 4;
        src += 4;
    }
    if (len >= 2){
        *(uint16_t*)dst = *(uint16_t*)src;
        len -= 2;

        dst += 2;
        src += 2;
    }
    if (len)
        *(uint8_t*)dst = *(uint8_t*)src;

    return dst;
}