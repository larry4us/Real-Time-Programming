#include <stdio.h>
#include <stdlib.h>
#include "matrixOperations.h"

int main() {
    // Criando duas matrizes para teste
    Matrix* A = createMatrix(2, 3);
    Matrix* B = createMatrix(3, 2);

    scanMatrix(A);
    printf("\n");
    scanMatrix(B);
    printf("\n");

    printf("--- Matriz A ---\n");
    displayMatrix(A);
    printf("\n--- Matriz B ---\n");
    displayMatrix(B);

    // Testando a multiplicação
    Matrix* C = multiplyMatrix(A, B);
    if (C != NULL) {
        printf("\n--- Resultado A * B ---\n");
        displayMatrix(C);
    } else {
        printf("\nNao foi possivel multiplicar A e B (dimensoes incompativeis).\n");
    }
    
    // Testando a transposição
    Matrix* C = transposeMatrix(C);
    if (C != NULL) {
        printf("\n--- Transposta de C ---\n");
        displayMatrix(C);
    }

    // Liberando toda a memória alocada
    freeMatrix(A);
    freeMatrix(B);
    freeMatrix(C);

    return 0;
}