char hello_world[15] = (char[15]){'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0'};

long zeros[123];

int puts(char* str);

int main(){
    puts(hello_world);

    return (int)((zeros + 12LU) - (zeros + 3LU));
}
