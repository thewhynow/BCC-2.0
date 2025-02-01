int printf(const char* __restrict fmt, ...);

unsigned int sleep(unsigned int seconds);



int main(int argc, char** argv){

    char* strings[9] = (char*[9]){
        "I pledge alegiance",
        "to the flag",
        "of the United States of America",
        "and to the republic",
        "for which it stands",
        "one Nation under God",
        "indivisible",
        "liberty and justice",
        "for all."
    };

    for (unsigned long i = 0LU; i < 9LU; ++i){
        printf("%s\n", strings[i]);
        sleep(2U);
    }

    for (unsigned long i = 0LU; i < 9LU; ++i)
        printf("%p\n", strings + i);
}