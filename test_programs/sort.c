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
    int numbers[10];
    int i;

    // Initialize array with some values
    numbers[0] = 64;
    numbers[1] = 34;
    numbers[2] = 25;
    numbers[3] = 12;
    numbers[4] = 22;
    numbers[5] = 11;
    numbers[6] = 90;
    numbers[7] = 54;
    numbers[8] = 33;
    numbers[9] = 5;

    printf("Original array: ");
    for (i = 0; i < 10; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    bubble_sort(numbers, 10);

    printf("Sorted array: ");
    for (i = 0; i < 10; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    return 0;
}