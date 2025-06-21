#ifndef _GDT_H
#define _GDT_H

#include "../../../libc/include/types.h"

#ifdef __cplusplus
extern "C" {
#endif
void GDT_init();
#ifdef __cplusplus
}
#endif


#define GDT_CODE_SEGMENT 0x08
#define GDT_DATA_SEGMENT 0x10

#endif