int main(){
    int x[100];
    int a[12] = {1,0};
    printf("%p == %p\n",&(a[3])-3, a);
    char c[12] = {(char)1,(char)0};
    printf("%p == %p\n",&(c[3])-3, c);
    char* d[12] = {c};
    printf("%p == %p\n",&(d[3])-3, d);
    printf("%f",10*(12.0-0.7));

    return 0;
}