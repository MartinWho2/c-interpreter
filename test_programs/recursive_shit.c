int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
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
    int fib_nums[10];
    int reversed_nums[10];
    int i;

    for (i = 0; i < 10; i++) {
        fib_nums[i] = fibonacci(i);
        reversed_nums[i] = bit_reverse(fib_nums[i]);
        printf("Fib(%d) = %d, Bit Reversed = %d\n", i, fib_nums[i], reversed_nums[i]);
    }

    return 0;
}