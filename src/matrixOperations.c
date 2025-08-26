#include <stdio.h>
#include <stdlib.h>
#include "matrixOperations.h"
#include <math.h>

//------------------------------------------------------------------
// Funções de Gerenciamento de Memória
//------------------------------------------------------------------

Matrix* createMatrix(int rows, int cols) {
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    if (matrix == NULL) return NULL;

    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = (double**)malloc(rows * sizeof(double*));
    if (matrix->data == NULL) {
        free(matrix);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        matrix->data[i] = (double*)malloc(cols * sizeof(double));
        if (matrix->data[i] == NULL) {
            for (int j = 0; j < i; j++) free(matrix->data[j]);
            free(matrix->data);
            free(matrix);
            return NULL;
        }
    }
    return matrix;
}

void freeMatrix(Matrix* matrix) {
    if (matrix == NULL) return;

    for (int i = 0; i < matrix->rows; i++) {
        free(matrix->data[i]);
    }
    free(matrix->data);
    free(matrix);
}

//------------------------------------------------------------------
// Funções de Entrada e Saída (I/O)
//------------------------------------------------------------------

void scanMatrix(Matrix* matrix) {
    printf("Digite os elementos da matriz (%.0dx%.0d):\n", (int)matrix->rows, (int)matrix->cols);
    for (int i = 0; i < matrix->rows; i++) {
        printf("  Linha %d: ", i + 1);
        for (int j = 0; j < matrix->cols; j++) {
            scanf("%lf", &matrix->data[i][j]); // %lf para ler doubles
        }
    }
}

void displayMatrix(Matrix* matrix) {
    if (matrix == NULL) {
        printf("Erro: Matriz nula.\n");
        return;
    }
    printf("Matriz (%dx%d):\n", matrix->rows, matrix->cols);
    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            printf("\t%.2f", matrix->data[i][j]); // %.2f para exibir com 2 casas decimais
        }
        printf("\n");
    }
}

//------------------------------------------------------------------
// Funções de Operações Matemáticas
//------------------------------------------------------------------

Matrix* addMatrix(Matrix* a, Matrix* b) {
    if (a->rows != b->rows || a->cols != b->cols) return NULL;

    Matrix* result = createMatrix(a->rows, a->cols);
    if (result == NULL) return NULL;

    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < a->cols; j++) {
            result->data[i][j] = a->data[i][j] + b->data[i][j];
        }
    }
    return result;
}

Matrix* subMatrix(Matrix* a, Matrix* b) {
    if (a->rows != b->rows || a->cols != b->cols) return NULL;

    Matrix* result = createMatrix(a->rows, a->cols);
    if (result == NULL) return NULL;

    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < a->cols; j++) {
            result->data[i][j] = a->data[i][j] - b->data[i][j];
        }
    }
    return result;
}

Matrix* multiplyMatrix(Matrix* a, Matrix* b) {
    if (a->cols != b->rows) return NULL;

    Matrix* result = createMatrix(a->rows, b->cols);
    if (result == NULL) return NULL;

    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            result->data[i][j] = 0.0; // Inicializar com 0.0
            for (int k = 0; k < a->cols; k++) {
                result->data[i][j] += a->data[i][k] * b->data[k][j];
            }
        }
    }
    return result;
}

Matrix* scalarMultiply(Matrix* matrix, double scalar) {
    Matrix* result = createMatrix(matrix->rows, matrix->cols);
    if (result == NULL) return NULL;

    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            result->data[i][j] = matrix->data[i][j] * scalar;
        }
    }
    return result;
}

Matrix* transposeMatrix(Matrix* matrix) {
    Matrix* result = createMatrix(matrix->cols, matrix->rows);
    if (result == NULL) return NULL;

    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            result->data[j][i] = matrix->data[i][j];
        }
    }
    return result;
}

double determinant(Matrix* matrix) {
    // Apenas matrizes quadradas possuem determinante
    if (matrix->rows != matrix->cols) {
        return NAN;
    }

    int n = matrix->rows;
    double det = 0.0;

    // Caso base: matriz 1x1
    if (n == 1) {
        return matrix->data[0][0];
    }

    // Caso base: matriz 2x2
    if (n == 2) {
        return (matrix->data[0][0] * matrix->data[1][1]) - (matrix->data[0][1] * matrix->data[1][0]);
    }

    // Lógica recursiva para matrizes maiores (expansão pela primeira linha)
    int sign = 1;
    for (int j = 0; j < n; j++) {
        Matrix* cofactor = getCofactor(matrix, 0, j);
        det += sign * matrix->data[0][j] * determinant(cofactor);
        sign = -sign;
        freeMatrix(cofactor);
    }

    return det;
}


/*
 * Calcula a inversa de uma matriz.
 * Retorna NULL se a matriz não for quadrada ou se seu determinante for zero.
 */
Matrix* inverseMatrix(Matrix* matrix) {
    if (matrix->rows != matrix->cols) return NULL;

    double det = determinant(matrix);

    // A matriz não é invertível se o determinante for zero
    if (fabs(det) < 1e-9) return NULL;

    int n = matrix->rows;

    // 1. Calcular a matriz de cofatores
    Matrix* cofactors = createMatrix(n, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            Matrix* submatrix = getCofactor(matrix, i, j);
            int sign = ((i + j) % 2 == 0) ? 1 : -1;
            cofactors->data[i][j] = sign * determinant(submatrix);
            freeMatrix(submatrix);
        }
    }

    // 2. Calcular a matriz adjunta (transposta da matriz de cofatores)
    Matrix* adjugate = transposeMatrix(cofactors);
    freeMatrix(cofactors); // Libera a matriz de cofatores

    // 3. Calcular a inversa dividindo a adjunta pelo determinante
    Matrix* inverse = createMatrix(n, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inverse->data[i][j] = adjugate->data[i][j] / det;
        }
    }
    
    freeMatrix(adjugate);
    return inverse;
}

Matrix* getCofactor(Matrix* matrix, int p, int q) {
    int n = matrix->rows;
    Matrix* cofactor = createMatrix(n - 1, n - 1);
    int row_c = 0, col_c = 0;

    for (int i = 0; i < n; i++) {
        if (i == p) continue;
        col_c = 0;
        for (int j = 0; j < n; j++) {
            if (j == q) continue;
            cofactor->data[row_c][col_c] = matrix->data[i][j];
            col_c++;
        }
        row_c++;
    }
    return cofactor;
}