int read_array(long array[2][2]){
    return (int)array[1LU][0LU];
}

int main(){
    long array[2][2] = (long[2][2]){{1L, 2L}, {3L, 4L}};
    array[1LU][0LU] = 12L;
    return read_array(array);
}
