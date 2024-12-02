int is_prime(int n) {
    int i;
    {
        if (n <= 1) {
            return 0;
        }
        if (n <= 3) {
            return 1;
        }

        if (n % 2 == 0 || n % 3 == 0) {
            return 0;
        }

        for (i = 5; i * i <= n; i += 6) {
            {
                if (n % i == 0 || n % (i + 2) == 0) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

void find_primes(int start, int end) {
    int num, count = 0;
    {
        printf("Prime numbers between %d and %d:\n", start, end);

        for (num = start; num <= end; num++) {
                if (is_prime(num)) {
                    {
                        printf("%d ", num);
                        count++;
                    }
                }

        }

        printf("\nTotal prime numbers found: %d\n", count);
    }
}

int calculate_factorial(int n) {
    int result = 1;
    {
        while (n > 1) {
            result *= n;
            n--;
        }
    }
    return result;
}

int main() {
    int primes[25] = {0};
    int range_start = 1;
    int range_end = 100;
    int factorial_of = 5;

    {
        find_primes(range_start, range_end);
        printf("Factorial of %d is: %d\n", factorial_of, calculate_factorial(factorial_of));
    }

    return 0;
}