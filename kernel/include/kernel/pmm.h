#ifndef _PMM_H
#define _PMM_H

#include "multiboot.h"

#include "../../../libc/include/string.h"
#include "../../../libc/include/stdio.h"
#include "../../../libc/include/stdlib.h"

#define MEMORY_BLOCK_SIZE 0x1000

#define BITMAP_BLOCK_FREE 0x0
#define BITMAP_BLOCK_USED 0x1

#ifdef __cplusplus
extern "C" {
#endif

void PMM_init();
void* alloc_page();
void free_page(void* page);

/* up to the caller to keep track of allocation sizes >:) */
void* alloc_pages(size_t n);
void free_pages(void* page, size_t n);

#ifdef __cplusplus
}
#endif

#endif