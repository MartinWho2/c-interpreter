int sum_array(int* arr, int length) {
    int total = 0;
    int i = 0;
    while (i < length) {
        total = total + arr[i];
        i = i + 1;
    }
    return total;
}

int main() {
    int x = 12;
    int numbers[5] = {1};
    int *ptr = numbers;
    int result = sum_array(ptr, 5);
    return result;
}