//Definition of User-Defined Functions

#include <stdio.h>
#include <stdlib.h>

int i, j, k; // for Loops

typedef struct {
    int rows;
    int cols;
    int** data;


} Matrix;


//------------------------------------------------------------------
// Funções de Gerenciamento de Memória
//------------------------------------------------------------------

Matrix* createMatrix(int rows, int cols) {
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    if (matrix == NULL) return NULL;

    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = (int**)malloc(rows * sizeof(int*));
    if (matrix->data == NULL) {
        free(matrix);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        matrix->data[i] = (int*)malloc(cols * sizeof(int));
        if (matrix->data[i] == NULL) {
            // Libera memória já alocada em caso de falha
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
    printf("Digite os elementos da matriz (%d x %d):\n", matrix->rows, matrix->cols);
    for (int i = 0; i < matrix->rows; i++) {
        printf("  Linha %d: ", i + 1);
        for (int j = 0; j < matrix->cols; j++) {
            scanf("%d", &matrix->data[i][j]);
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
            printf("\t%d", matrix->data[i][j]);
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
    // Para multiplicar, o número de colunas de A deve ser igual ao de linhas de B
    if (a->cols != b->rows) return NULL;

    Matrix* result = createMatrix(a->rows, b->cols);
    if (result == NULL) return NULL;

    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            result->data[i][j] = 0;
            for (int k = 0; k < a->cols; k++) {
                result->data[i][j] += a->data[i][k] * b->data[k][j];
            }
        }
    }
    return result;
}

Matrix* scalarMultiply(Matrix* matrix, int scalar) {
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