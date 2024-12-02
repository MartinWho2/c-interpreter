int factorial(int n) {
    int result = 1;
    while (n > 0) {
        result = result * n;
        n = n - 1;
    }
    return result;
}

int main() {
    int x = 30;
    int y = 0;

    if (x > 0) {
        y = factorial(x);
    } else {
        y = 1;
    }

    return y;
}