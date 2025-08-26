#ifndef MATRIX_OPERATIONS
#define MATRIX_OPERATIONS

//------------------------------------------------------------------
// Estrutura
//------------------------------------------------------------------

typedef struct {
    int rows;
    int cols;
    int** data;
} Matrix;


//------------------------------------------------------------------
// Declaração das Funções
//------------------------------------------------------------------

// Funções de Gerenciamento de Memória
Matrix* createMatrix(int rows, int cols);
void freeMatrix(Matrix* matrix);

// Funções de Entrada e Saída (I/O)
void scanMatrix(Matrix* matrix);
void displayMatrix(Matrix* matrix);

// Funções de Operações Matemáticas
Matrix* addMatrix(Matrix* a, Matrix* b);
Matrix* subMatrix(Matrix* a, Matrix* b);
Matrix* multiplyMatrix(Matrix* a, Matrix* b);
Matrix* scalarMultiply(Matrix* matrix, int scalar);
Matrix* transposeMatrix(Matrix* matrix);


#endif // MATRIX_H