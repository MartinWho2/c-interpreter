#include <stdio.h>
int complex_calculation(int a, int b) {
    int result = (a + b) * (a - b) / (a | b) & (a ^ b);
    return result;
}

int main() {
    int x = 10;
    int y = 5;
    int z = complex_calculation(x, y);
    printf("%d",z);
    return z;
}