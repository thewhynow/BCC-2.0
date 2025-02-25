int printf(const char* restrict __fmt, ...);

typedef struct {
    int a;
    char c[3];
    int b;
} tri;

int add_pair(tri p);

typedef struct {
    char elem[7];
} three;

void print_three(three t){
    printf("%s\n", t.elem);
}

tri idk = (tri){
    .a = 1,
    .b = 2,
    .c = {(char)'a', (char)'b', (char)'c'}
};

int main(){
    print_three((three){.elem = {(char)'a', (char)'b', (char)'c', (char)'d', (char)'e', (char)'f', (char)0}});
}