void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void bubble_sort(int *arr, int size) {
    int i, j;
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(&arr[j], &arr[j + 1]);
            }
        }
    }
}

int main() {
    int size = 400;
    int numbers[size];
    int state = 0xabcdef;
    int mod = 0xdeadc0ed;
    int a = 3;
    int b = 0x12345678;
    for (int j = 0; j < size; ++j) {
        numbers[j] = state;
        state = (state* a + b) % mod;
    }
    // Initialize array with some values

    printf("Original array: ");
    for (int i = 0; i < size; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    bubble_sort(numbers, size);

    printf("Sorted array: ");
    for (int i = 0; i < size; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    return 0;
}