int string_length(char *str) {
    int length = 0;
    {
        while (str[length] != '\0') {
            {
                length++;
            }
        }
    }
    return length;
}

void reverse_string(char *str) {
    int length = string_length(str);
    int start = 0;
    int end = length - 1;
    char temp;

    {
        while (start < end) {
            {
                debug();
                temp = str[start];
                str[start] = str[end];
                str[end] = temp;
                start++;
                end--;
            }
        }
    }
}

int is_palindrome(char *str) {
    int length = string_length(str);
    for (int i = 0; i < length / 2; i++) {
        if (str[i] != str[length - 1 - i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    char* str2 = "1234567";

    char str1[16] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0'};
    //char* palindromes[3] = {"racecar", "level", "madam"};
    int i;
    printf("Original str1: %s\n", str2);
    reverse_string(str2);
    debug();
    printf("Reversed str1: %s\n\n", str2);
    /*
    for (i = 0; i < 3; i++) {
        printf("Is '%s' a palindrome? ",palindromes[i]);
        if (is_palindrome(palindromes[i])){
            printf("Yes\n");
        }else{
            printf("No\n");
        }
    }*/
    return 0;
}