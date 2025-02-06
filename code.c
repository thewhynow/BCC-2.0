#include "lib/my_stdlib.h"

void* alloc_array(unsigned long elem_size, unsigned long size){
    void* buff = malloc(24LU + elem_size * size);
    unsigned long* size_p = (unsigned long*) buff;
    *size_p = size;
    *(size_p + 1LU) = 0LU;
    *(size_p + 2LU) = elem_size;

    return size_p + 3LU;
}

void free_array(void* array, void(*destructor)(void*)){
    if (destructor) {
        unsigned long* count = (unsigned long*)array - 2LU;
        unsigned long* elem_size = count + 1LU;
        for (unsigned long i = 0LU; i < *count; ++i)
            destructor((char*)array + (*elem_size * i));
    }

    free((unsigned long*)array - 3LU);
}

void pback_array(void** array, void* val){    
    unsigned long elem_size = *((unsigned long*) *array - 1LU);
    unsigned long* count = (unsigned long*)*array - 2LU;
    unsigned long* size = (unsigned long*)*array - 3LU;

    if (*count == *size) {
        *size += *size / 2LU + 1LU;
        *array = realloc(size, 24LU + elem_size * *size) + 24LU;
        if (*array)
            count = (unsigned long*)*array - 2LU;
        else
            perror("ran out of memory while allocating array");
    }

    void* dst = (char*)*array + (*count)++ * elem_size;

    memcpy(dst, val, elem_size);
}

int main(){
    int* a = alloc_array(sizeof(int), 1LU);

    for (int i = 0; i < 10; ++i)
        pback_array((void**)&a, &i);

    for (unsigned long i = 0LU; i < 10LU; ++i)
        printf("%i ", a[i]);

    free_array(a, (void*)0);

    putchar('\n');
}