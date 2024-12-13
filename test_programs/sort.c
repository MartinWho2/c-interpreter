#include <stdio.h>
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void bubble_sort(int *arr, int size) {
    int i, j;
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            continue;
            if (arr[j] > arr[j + 1]) {
                //int temp = arr[j];
                //arr[j] = arr[j + 1];
                //arr[j + 1] = temp;
                //swap(&arr[j], &arr[j + 1]);
            }
        }
    }
}

int main() {
    int size = 1000;
    int numbers[size];
    int state = 0x9abcdef;
    int mod = 0x1337c0e1;
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