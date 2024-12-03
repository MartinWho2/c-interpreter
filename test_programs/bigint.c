int MAX_DIGITS = 100;
//padding
int A = 0, B=0, C=0;
void set_sign(char* bigint, char is_negative) {
    *(bigint + MAX_DIGITS - 1) = is_negative;
}

char get_sign(char* bigint) {
    return *(bigint + MAX_DIGITS - 1);
}

void set_num_digits(char* bigint, char count) {
    *(bigint + MAX_DIGITS - 2) = count;
}

char get_num_digits(char* bigint) {
    return *(bigint + MAX_DIGITS - 2);
}

void int_to_bigint(int value, char* result) {
    set_num_digits(result, (char)0);
    set_sign(result, (char)(value < 0));

    if (value < 0) { value = -value; }
    while (value > 0) {
        *(result + (int)get_num_digits(result)) = (char)(value % 10);
        set_num_digits(result, get_num_digits(result) + (char)1);
        value /= 10;
    }
}

void add_bigint(char* a, char* b, char* result) {
    // Reset result
    set_num_digits(result, (char)0);
    set_sign(result, (char)0);

    // Determine if result will be negative
    char a_sign = get_sign(a);
    char b_sign = get_sign(b);
    char a_digits = get_num_digits(a);
    char b_digits = get_num_digits(b);

    // If signs are different, this becomes a subtraction problem
    if (a_sign != b_sign) {
        // TODO: Implement full subtraction logic if needed
        // For simplicity, this implementation assumes same signs
        return;
    }

    // Propagate the sign
    set_sign(result, a_sign);

    // Perform addition
    char carry = (char)0;
    char max_digits = b_digits;
    if (a_digits > b_digits){
        max_digits = a_digits;
    }
    for (char i = (char)0; i < max_digits || carry; i++) {
        // Get digits, default to 0 if out of range
        char digit_a = (char)0;
        if (i < a_digits){
            digit_a =  *(a + (int)i);
        }
        char digit_b = (char)0;
        if(i < b_digits){digit_b = *(b + (int)i);}

        // Add digits and carry
        char sum = digit_a + digit_b + carry;
        carry = sum / (char)10;
        sum =(char)(((int)sum) % 10);

        // Store result digit
        *(result + (int)i) = sum;
        set_num_digits(result, get_num_digits(result) + (char)1);
    }
}

void print_bigint(char* num) {
    char num_digits = get_num_digits(num);
    if (num_digits == (char)0) {
        printf("0");
        return;
    }

    // Print sign if negative
    if (get_sign(num)) {
        printf("-");
    }
    // Print digits in reverse order
    for (int i = (int)num_digits - 1; i >= 0; i--) {
        printf("%c", *(num + i) + '0');
    }
}
void print_bigint(char* num) {
    char num_digits = get_num_digits(num);
    if (num_digits == (char)0) {
        // Handle zero case
        return;
    }

    // Print sign if negative
    if (get_sign(num)) {
        printf("-");
    }
    // Print digits in reverse order
    for (int i = (int)num_digits - 1; i >= 0; i--) {
        char val = *(num + i);
        // Placeholder print function for single digit
        printf("%c",*(num + i) + '0');
    }
}

int main() {
    char a[MAX_DIGITS], b[MAX_DIGITS],c[MAX_DIGITS], d[MAX_DIGITS];
    int_to_bigint(0, a);
    int_to_bigint(654321,b);
    add_bigint(a,b,c);
    int r = 654321;
    for (int i = 0; i < 100; ++i) {
        add_bigint(c,c,d);
        add_bigint(a,d,c);
        printf("c = ");
        print_bigint(c);
        printf(" (should be %d)\n",r*2);
        r *= 2;
    }
    return 0;
}