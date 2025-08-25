#include <stdio.h>
#include <stdlib.h>
#include "matrixOperations.h"

int main(void) {
    int rows = 3;
    int cols = 4;

    int **matrixA = createMatrix(rows, cols);
    int **matrixB = createMatrix(rows, cols);

    if (matrixA == NULL || matrixB == NULL) {
        printf("Falha na alocacao de memoria.\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrixA[i][j] = i + j;
            matrixB[i][j] = i * 2 + j;
        }
    }

    printf("Matrix A:\n");
    displayMatrix(matrixA, rows, cols);

    printf("\nMatrix B:\n");
    displayMatrix(matrixB, rows, cols);

    int **matrixC = addMatrix(matrixA, matrixB, rows, cols);

    if (matrixC == NULL) {
        printf("Falha na alocacao de memoria para a soma.\n");
        freeMatrix(matrixA, rows);
        freeMatrix(matrixB, rows);
        return EXIT_FAILURE;
    }

    printf("\nResultado (A + B):\n");
    displayMatrix(matrixC, rows, cols);

    freeMatrix(matrixA, rows);
    freeMatrix(matrixB, rows);
    freeMatrix(matrixC, rows);

    return EXIT_SUCCESS;
}