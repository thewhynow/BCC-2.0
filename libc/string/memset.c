#include "../include/string.h"
#include "../include/types.h"

void* memset(void* ptr, int value, size_t size){
    for (size_t i = 0; i < size; ++i)
        ((uint8_t*)ptr)[i] = value;

    return ptr;
}