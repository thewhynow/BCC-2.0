long time(long* timer);
int printf(const char* restrict __fmt, ...);
int putchar(int c);
void srand(unsigned int __seed);
int rand();
void* fopen(const char* restrict path, const char* restrict mode);
void fclose(void* stream);
int fprintf(void* stream, const char* restrict format, ...);
int fscanf(void* stream, const char* restrict format, ...);
int feof(void* stream);
void* malloc(unsigned long size);
void free(void* ptr);
void* realloc(void* ptr, unsigned long size);
void perror(const char* msg);
void *memcpy(void *restrict __dest, const void *restrict __src, unsigned long __n);
int scanf(const char* restrict format, ...);
unsigned long strlen(const char* buff);
int atoi(const char* buff);
extern void* stdin;
extern void* stdout;
extern void* stderr;
typedef unsigned long size_t;
typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;
int puts(const char* str);
int fseek(void*__stream, long __off, int __whence);
void rewind(void*__stream);
size_t fread(void* restrict __ptr, size_t __size, size_t __n, void* restrict __stream);
char *strncpy(char* restrict __dest, const char* restrict __src, size_t __n);
int strncmp(const char* __s1, const char* __s2, size_t __n);
long ftell(void*__stream);
char* strdup(char* str);
void exit(int e);
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
    memcpy((char*)*array + (*count)++ * elem_size, val, elem_size);
}
typedef struct {
    int type;
    int val;
} token_t;
token_t* lex_buff(const char* buff){
    token_t* tokens = alloc_array(sizeof *tokens, 1LU);
    token_t curr_token;
    unsigned long i = 0LU;
    unsigned long buff_len = strlen(buff);
    while (i < buff_len){
        switch((int)buff[i]){
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                curr_token = (token_t){.type = 1, .val = 0};
                while ((char)'0' <= buff[i] && buff[i] <= (char)'9'){
                    curr_token.val = curr_token.val * 10 + (int)(buff[i] - (char)'0');
                    ++i;
                }
                pback_array(&tokens, &curr_token);
                break;
            }
            case '+': {
                curr_token.type = 2;
                pback_array(&tokens, &curr_token);
                ++i;
                break;
            }
            case '-': {
                curr_token.type = 3;
                pback_array(&tokens, &curr_token);
                ++i;
                break;
            }
            case '*': {
                curr_token.type = 4;
                pback_array(&tokens, &curr_token);
                ++i;
                break;
            }
            case '/': {
                curr_token.type = 5;
                pback_array(&tokens, &curr_token);
                ++i;
                break;
            }
        }
    }
    curr_token.type = 0;
    pback_array((void**)&tokens, &curr_token);
    return tokens;
}
typedef struct node_t node_t;
struct node_t {
    int type;
    int val;
    node_t* val_a;
    node_t* val_b;
};
static token_t* curr_token;
void advance_token(){
    static unsigned long token_count = 0LU;
    if (curr_token->type != 0){
        curr_token += 1LU;
        ++token_count;
    }
}
node_t* alloc_node(node_t n){
    node_t* res = malloc(sizeof n);
    *res = n;
    return res;
}
node_t parse_fac(){
    node_t node;
    switch(curr_token->type){
        case 1: {
            node.type = 1;
            node.val = curr_token->val;
            advance_token();
            return node;
        }
        default: {
            printf("expected int\n");
            exit(-1);
        }
    }
}
node_t parse_term(){
    node_t result = parse_fac();
    while ((1)){
        switch(curr_token->type){
            case 4: {
                advance_token();
                result = (node_t){
                    .type = 4,
                    .val_a = alloc_node(result),
                    .val_b = alloc_node(parse_fac())
                };
                break;
            }
            case 5: {
                advance_token();
                result = (node_t){
                    .type = 5,
                    .val_a = alloc_node(result),
                    .val_b = alloc_node(parse_fac())
                };
                break;
            }
            default: {
                return result;
            }
        }
    }
}
node_t parse_expr(){
    node_t result = parse_term();
    while((1)){
        switch(curr_token->type){
            case 2: {
                advance_token();
                result = (node_t){
                    .type = 2,
                    .val_a = alloc_node(result),
                    .val_b = alloc_node(parse_term())
                };
                break;
            }
            case 3: {
                advance_token();
                result = (node_t){
                    .type = 3,
                    .val_a = alloc_node(result),
                    .val_b = alloc_node(parse_term())
                };
                break;
            }
            default: {
                return result;
            }
        }
    }
}
node_t parse(token_t* tokens){
    curr_token = tokens;
    return parse_expr();
}
int interpret(node_t node){
    switch(node.type){
        case 1: {
            return node.val;
        }
        case 2: {
            return interpret(*node.val_a) + interpret(*node.val_b);
        }
        case 3: {
            return interpret(*node.val_a) - interpret(*node.val_b);
        }
        case 4: {
            return interpret(*node.val_a) * interpret(*node.val_b);
        }
        case 5: {
            return interpret(*node.val_a) / interpret(*node.val_b);
        }
    }
    return 0;
}
int main(int argc, char** argv){
    if (argc == 2){
        printf("%i\n", interpret(parse(lex_buff(argv[1L]))));
    }
    else
        puts("enter argument");
}
