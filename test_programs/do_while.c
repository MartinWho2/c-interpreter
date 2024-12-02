int power(int base, int exponent) {
    int res = 1;
    do {
        res *= base;
        exponent--;
    } while (exponent > 0);
    return res;
}

int main() {
    int base = 10;
    for (int i = 0; i < 8; ++i) {
        printf("%d^%d = %d\n",base,i,power(base,i));
    }
    return power(10,100);
}