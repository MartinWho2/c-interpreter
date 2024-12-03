int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 5;
    int y = 7;
    printf("x = %d\n",x);
    printf("y = %d\n",y);
    int result = add(x, y);
    printf("x + y = %d\n",result);
    return result;
}