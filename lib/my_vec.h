#define get_size_array(array) (*((unsigned long*)(array) - 3))

#define get_count_array(array) (*((unsigned long*)(array) - 2))

#define get_elem_size_array(array) (*((unsigned long*)(array) - 1))

void pback_array(void** array, void* val);

void free_array(void* array, void(*destructor)(void*));

void* alloc_array(unsigned long elem_size, unsigned long size);