void *memset(void *__s, int __c, unsigned long __n);

char is_prime[1000];

int main() {
    memset(is_prime, 1, 1000LU);
    is_prime[0LU] = is_prime[1LU] = (char)0;

    for (int i = 2; i * i <= 999; i++) {
        if (is_prime[(unsigned long)i]) {
            for (int j = i * i; j <= 999; j += i) {
                is_prime[(unsigned long)j] = (char)0;
            }
        }
    }
    return 0;
}