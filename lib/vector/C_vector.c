#ifndef C_VECTOR_H
#define C_VECTOR_H

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

void* alloc_array(size_t elem_size, size_t size){
    void* buff = malloc(sizeof(size_t) * 3 + elem_size * size);
    size_t* size_p = (size_t*) buff;
    *size_p = size;
    *(size_p + 1) = 0;
    *(size_p + 2) = elem_size;

    return size_p + 3;
}

void free_array(void* array, void(*destructor)(void* val)){
    if (destructor) {
        size_t* count = (size_t*)array - 2;
        size_t* elem_size = count + 1;
        for (size_t i = 0; i < *count; ++i)
            destructor((char*)array + (*elem_size * i));
    }
    
    free((size_t*)array - 3);
}

void pback_array(void** array, void* val){    
    size_t elem_size = *((size_t*) *array - 1);
    size_t* count = (size_t*)*array - 2;
    size_t* size = (size_t*)*array - 3;

    if (*count == *size){
        *size += *size / 2 + 1;
        *array = realloc(size, sizeof(size_t) * 3 + elem_size * *size) + sizeof(size_t) * 3;
        if (*array)
            count = (size_t*)*array - 2;
        else
            perror("ran out of memory while allocating array");
    }
    
    memcpy(*array + ((*count)++ * elem_size), val, elem_size);
}

#endif