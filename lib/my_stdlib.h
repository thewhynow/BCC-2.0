/* sets *timer if not NULL */
long time(long* timer);

/* writes a format-string to stdout using extra args */
int printf(const char* restrict __fmt, ...);

/* writes a character to stdout */
int putchar(int c);

/* seeds the random generation alg used by "int rand(void)" */
void srand(unsigned int __seed);

/* generates a pusedo-random integer */
int rand();

/*
*   use FILE* as void* to save memory while compiling
*/

/* opens a STREAM */
void* fopen(const char* restrict path, const char* restrict mode);

/* closes a STREAM */
void fclose(void* stream);

/* prints formatted output to a STREAM */
int fprintf(void* stream, const char* restrict format, ...);

/* reads formatted input from STREAM */
int fscanf(void* stream, const char* restrict format, ...);

/* returns !0 if stream is at EOF */
int feof(void* stream);

/* returns a heap-allocated block to at least size bytes */
void* malloc(unsigned long size);

/* frees a block allocated by malloc and friends */
void free(void* ptr);

/* reallocates a block allocated by malloc and friends to N bytes and copies memory */
void* realloc(void* ptr, unsigned long size);

/* prints an error to stderr */
void perror(const char* msg);

/* copies N bytes of src to dst */
void *memcpy(void *restrict __dest, const void *restrict __src, unsigned long __n);

/* reads formatted input from stdin */
int scanf(const char* restrict format, ...);

/* returns # bytes to NULL byte from ptr */
unsigned long strlen(const char* buff);

/* converts a string to an int */
int atoi(const char* buff);

#define NULL ((void*)0)

#define true (1)
#define false (0)

extern void* stdin;
extern void* stdout;
extern void* stderr;

#define stdin stdin
#define stdout stdout
#define stderr stderr

typedef unsigned long size_t;

typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

/* prints a newline-terminated string to stdout */
int puts(const char* str);

int fseek(void*__stream, long __off, int __whence);

void rewind(void*__stream);

size_t fread(void* restrict __ptr, size_t __size, size_t __n, void* restrict __stream);

char *strncpy(char* restrict __dest, const char* restrict __src, size_t __n);

int strncmp(const char* __s1, const char* __s2, size_t __n);

long ftell(void*__stream);

char* strdup(char* str);