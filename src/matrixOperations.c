//Definition of User-Defined Functions

#include <stdio.h>
#include <stdlib.h>

int i, j, k; // for Loops

// Função para criar uma matriz dinamicamente
int** createMatrix(int rows, int columns) {
    int i;
    // 1. Aloca um ponteiro para cada linha
    int **matrix = (int **)malloc(rows * sizeof(int *));

    // 2. Para cada linha, aloca espaço para as colunas
    for (i = 0; i < rows; i++) {
        matrix[i] = (int *)malloc(columns * sizeof(int));
    }

    return matrix;
}

// Função para liberar a memória da matriz
void freeMatrix(int **matrix, int rows) {
    int i;
    // 1. Libera a memória de cada linha
    for (i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    // 2. Libera o ponteiro das linhas
    free(matrix);
}

void displayMatrix(int **array, int rows, int columns){
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++){
            printf("\t%d", array[i][j]);
        }
        printf("\n");
    }
}


int** alocateSpace (int rows, int columns) {
    int i, j; // for Loops

       // Cria a matriz de soma dinamicamente, com o tamanho exato necessário.
       int **sum = (int **)malloc(rows * sizeof(int *));
       if (sum == NULL) {
           return NULL;
       }
       
       
       for (i = 0; i < rows; i++) {
           sum[i] = (int *)malloc(columns * sizeof(int));
           if (sum[i] == NULL) {
                // Tratamento de erro: se falhar, libera o que já foi alocado
               for (j = 0; j < i; j++) {
                   free(sum[j]);
               }
               free(sum);
               return NULL;
           }
       }

       return sum;
}

int** addMatrix(int **matrixA, int **matrixB, int rows, int columns) {

    int i, j;

    int **sum = alocateSpace(rows, columns);

    // 3. Realize a soma, guardando o resultado na nova matriz.
    for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++) {
            sum[i][j] = matrixA[i][j] + matrixB[i][j];
        }
    }

    // 4. Retorne o ponteiro para a matriz recém-criada.
    return sum;
}

int** subMatrix(int **matrixA, int **matrixB, int rows, int columns) {

    int i, j;

    int **sub = alocateSpace(rows, columns);

    // 3. Realize a soma, guardando o resultado na nova matriz.
    for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++) {
            sub[i][j] = matrixA[i][j] - matrixB[i][j];
        }
    }

    // 4. Retorne o ponteiro para a matriz recém-criada.
    return sub;
}

void scanMatrix(int **array, int rows, int columns){
    
    for (i = 0; i < rows; i++){
        printf("\t%d entries for row %d: ", columns, i + 1);
        for (j = 0; j < columns; j++){
            scanf("%d", &array[i][j]);
        }
    }

    return;
}

// void SubMatrix(int arrayone[10][10], int arraytwo[10][10], int rows, int columns){
    
//     int dif[10][10];

//     for (i = 0; i < rows; i++){
//         for (j = 0; j < columns; j++){
//             dif[i][j] = arrayone[i][j] - arraytwo[i][j];
//             printf("\t%d", dif[i][j]);
//         }
//         printf("\n");
//     }
// }

// void ScalarProduct(int array[10][10], int scalar, int rows, int columns){
    
//     int sca[10][10];
//     for (i = 0; i < rows; i++){
//         for (j = 0; j < columns; j++){
//             sca[i][j] = scalar * array[i][j];
//             printf("%d\t", sca[i][j]);
//         }
//         printf("\n");
//     }

// }

// void ProductMatrix(int arrayone[10][10], int arraytwo[10][10], int rowsA, int columnsA,int columnsB){
    
//     int pro[10][10];

//      // Initializing all the Elements of Matrix Pro to 0
//     for (i = 0; i<rowsA; ++i)
//         for (j = 0; j<columnsB; ++j)
//         {
//             pro[i][j] = 0;
//         }

//     for (i = 0; i<rowsA; i++)
//         for (j = 0; j<columnsB; j++)
//         // Initializing the Elements of Result Matrix to 0 before Processing
//         //pro[i][j] = 0;
//             for (k = 0; k<columnsA; ++k)
//             {
//                 pro[i][j] += arrayone[i][k] * arraytwo[k][j];
//             }
//     for (i = 0; i<rowsA; ++i){
//         for (j = 0; j<columnsB; ++j)
//         {
//             printf("\t%d ", pro[i][j]);
//         }
//                 printf("\n\n");
//     }
// }
// void TransposeMatrix(int arrayone[10][10], int rows, int columns){

//     int transpose[10][10];

//     for (i=0;i<columns;i++){
//             for(j=0;j<rows;j++){
//                 transpose[i][j]=arrayone[j][i];
//                 printf("%d\t",transpose[i][j]);
//             }
//             printf("\n\n");
//         }

// }
