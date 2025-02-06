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