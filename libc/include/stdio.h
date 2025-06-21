#ifndef _STDIO_H
#define _STDIO_H
    #define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif
    int printf(const char* __restrict, ...);
    int putchar(int);
    int puts(const char*);
    int getchar();
    char* gets(char*);
#ifdef __cplusplus
}
#endif

#endif