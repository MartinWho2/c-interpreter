int MAX = 1000;
int FIB[MAX] = {0,1};


int fibonacci(int n) {
    if (FIB[n] != 0 || n == 0) {
        return FIB[n];
    }
    int res = fibonacci(n - 1) + fibonacci(n - 2);
    FIB[n] = res;
    return res;
}
#include <stdio.h>
int factorial(int n){
    if (n <= 1){
        return n;
    }
    return n * factorial(n-1);
}

int bit_reverse(int n) {
    int reversed = 0;
    int bits = sizeof(n) * 8;
    int i;

    for (i = 0; i < bits; i++) {
        reversed = (reversed << 1) | (n & 1);
        n >>= 1;
    }

    return reversed;
}

int main() {
    printf("Enter a number:\n");
    int n = input();
    if (n >= MAX){
        printf("Number too large\n");
        return -1;
    }
    int fib_nums[n];
    int i;

    for (i = 0; i < n; i++) {
        fib_nums[i] = fibonacci(i);
        //reversed_nums[i] = bit_reverse(fib_nums[i]);
        printf("Fib(%d) = %d\n", i, fib_nums[i]);
    }
    return 0;
}