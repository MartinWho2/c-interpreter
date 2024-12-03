int main(){
    int a = 0xffffffff;
    int *b = &a;
    int *c = b + 3;
    int *d = 4+b;
    int e = c-d ;
    debug();
    return 0;
}