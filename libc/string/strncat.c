#include "../include/string.h"
#include "../include/types.h"

char* strncat(char* restrict s1, const char* restrict s2, size_t n){
    s1 += strlen(s1);

    size_t s2_len = strlen(s2);
    size_t i = 0;

    for (; i < n && i < s2_len; ++i)
        s1[i] = s2[i];

    s1[i] = '\0';

    return s1;
}