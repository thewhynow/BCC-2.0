#include "../include/string.h"
#include "../include/types.h"

int memcmp(const void* a, const void* b, size_t size){
    for (size_t i = 0; i < size; ++i){
        if (((uint8_t*)a)[i] < ((uint8_t*)b)[i])
            return -1; else
        if (((uint8_t*)a)[i] > ((uint8_t*)b)[i])
            return 1;
    }

    return 0;
}