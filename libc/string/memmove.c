#include "../include/string.h"
#include "../include/types.h"

void* memmove(void* dst, const void* src, size_t size){
    if (dst < src){
        while (size >= 4){
            *(uint32_t*)dst = *(uint32_t*)src;
            size -= 4;
            dst += 4;
            src += 4;
        }

        if (size >= 2){
            *(uint16_t*)dst = *(uint16_t*)src;
            size -= 2;
            dst += 2;
            src += 2;
        }

        if (size){
            *(uint8_t*)dst = *(uint8_t*)(src);
        }
    }
    else {
        dst += size;
        src += size;

        while (size >= 4){
            *(uint32_t*)dst = *(uint32_t*)src;
            size -= 4;
            dst -= 4;
            src -= 4;
        }

        if (size >= 2){
            *(uint16_t*)dst = *(uint16_t*)src;
            size -= 2;
            dst -= 2;
            src -= 2;
        }

        if (size){
            *(uint8_t*)dst = *(uint8_t*)(src);
        }
    }
    return dst;
}