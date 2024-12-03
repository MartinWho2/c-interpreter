// Program 4: Pointer-based String Manipulation
int string_length(char* str) {
    char* start = str;
    while (*str) { str++; }
    return str - start;
}

void string_copy(char* dest, char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

int main() {
    char original[20] = {'H','e','l','l','o',',',' ','P','o','i','n','t','e','r','s','!'};
    char buffer[50];

    string_copy(buffer, original);
    int length = string_length(buffer);

    return length;
}