void matrix_multiply(int *a, int *b, int *result, int rows_a, int cols_a, int cols_b) {
    int i, j, k;

    for (i = 0; i < rows_a; i++) {
        for (j = 0; j < cols_b; j++) {
            *(result + i*cols_b + j) = 0;
            for (k = 0; k < cols_a; k++) {
                *(result + i*cols_b + j) += (*(a + i*cols_a + k)) * (*(b + k*cols_b + j));
            }
        }
    }
}

int matrix_determinant(int *matrix, int size) {
    if (size == 1) {
        return *matrix;
    }

    if (size == 2) {
        return matrix[0] * matrix[3] - matrix[1] * matrix[2];
    }

    int det = 0;
    int submatrix[16];
    int sign = 1;
    int i, j, k, m;

    for (i = 0; i < size; i++) {
        m = 0;
        for (j = 1; j < size; j++) {
            for (k = 0; k < size; k++) {
                if (k == i) { continue; }
                submatrix[m++] = matrix[j * size + k];
            }
        }

        det += sign * matrix[i] * matrix_determinant(submatrix, size - 1);
        sign = -sign;
    }

    return det;
}

int main() {
    int matrix1[4] = {1, 2, 3, 4};
    int matrix2[4] = {5, 6, 7, 8};
    int result[4];
    matrix_multiply(matrix1, matrix2, result, 2, 2, 2);

    printf("Matrix Multiplication Result:\n");
    printf("%d %d\n%d %d\n", result[0], result[1], result[2], result[3]);

    int det = matrix_determinant(matrix1, 2);
    printf("Determinant of matrix1: %d\n", det);

    return 0;
}