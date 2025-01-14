#ifndef C_VECTOR_H
#define C_VECTOR_H

#include <stdlib.h>

/*
    implements a generic vector in C
    alloc_array returns a pointer to a block of memory where the elements are stored
    there are 3 size_t behind the pointer returned that hold information about the array
    they are stored in this order following furthest from the actual buffer pointer:
        array size
        array count
        array elem size
    
    the one disadvantage of this generic implementation is that rvalues cannot be used to pback on the array, but that is not strictly necessary

*/

void* alloc_array(size_t elem_size, size_t size);

void free_array(void* array, void(*destructor)(void* val));

void pback_array(void* array, void* val);

#define get_size_array(array) (*((size_t*)(array) - 3))

#define get_count_array(array) (*((size_t*)(array) - 2))

#define get_elem_size_array(array) (*((size_t*)(array) - 1))

#endif