long add(long a, unsigned char b, short c, signed int d, unsigned long e, char f, short g, long h, char i, long j, unsigned long k){
    return a + (long)k;
}

int main(){
    return (int)add(1L, (unsigned char)1, (short)0, 5, 0LU, (char)9, (short)0, 1234567L, (char)0, 0L, 10LU);
}