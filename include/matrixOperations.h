#ifndef MATRIX_OPERATIONS
#define MATRIX_OPERATIONS

//------------------------------------------------------------------
// Estrutura
//------------------------------------------------------------------

typedef struct {
    int rows;
    int cols;
    double** data;
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
Matrix* scalarMultiply(Matrix* matrix, double scalar);
Matrix* transposeMatrix(Matrix* matrix);
Matrix* inverseMatrix(Matrix* matrix);

double determinant(Matrix* matrix);


// Funções internas
Matrix* getCofactor(Matrix* matrix, int p, int q);

#endif // MATRIX_H