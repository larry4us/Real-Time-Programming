//Declarartion of User-Defined Functions

#ifndef MATRIXOPERATIONS_H
#define MATRIXOPERATIONS_H

int** createMatrix(int rows, int columns);
void freeMatrix(int** matrix, int rows);
void displayMatrix(int** matrix, int rows, int columns);
int** addMatrix(int** matrixA, int** matrixB, int rows, int columns);

#endif // MATRIXOPERATIONS_H   