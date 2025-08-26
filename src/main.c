#include <stdio.h>
#include <stdlib.h>
#include "matrixOperations.h" 

int main() {
    // Matrizes de teste
    Matrix* M1 = createMatrix(2, 3);
    M1->data[0][0] = 1.0; M1->data[0][1] = 2.0; M1->data[0][2] = 3.0;
    M1->data[1][0] = 4.0; M1->data[1][1] = 5.0; M1->data[1][2] = 6.0;

    Matrix* M2 = createMatrix(3, 2);
    M2->data[0][0] = 7.0; M2->data[0][1] = 8.0;
    M2->data[1][0] = 9.0; M2->data[1][1] = 1.0;
    M2->data[2][0] = 2.0; M2->data[2][1] = 3.0;

    Matrix* M3 = createMatrix(2, 2);
    M3->data[0][0] = 1.0; M3->data[0][1] = 2.0;
    M3->data[1][0] = 3.0; M3->data[1][1] = 4.0;

    Matrix* M4 = createMatrix(2, 2);
    M4->data[0][0] = 4.0; M4->data[0][1] = 3.0;
    M4->data[1][0] = 2.0; M4->data[1][1] = 1.0;

    Matrix* M5_invertible = createMatrix(3, 3);
    M5_invertible->data[0][0] = 2; M5_invertible->data[0][1] = -1; M5_invertible->data[0][2] = 0;
    M5_invertible->data[1][0] = -1; M5_invertible->data[1][1] = 2; M5_invertible->data[1][2] = -1;
    M5_invertible->data[2][0] = 0; M5_invertible->data[2][1] = -1; M5_invertible->data[2][2] = 2;

    Matrix* M6_singular = createMatrix(2, 2);
    M6_singular->data[0][0] = 1.0; M6_singular->data[0][1] = 2.0;
    M6_singular->data[1][0] = 2.0; M6_singular->data[1][1] = 4.0;

    printf("--- MATRIZES DE TESTE INICIAIS ---\n");
    printf("M3\n");
    displayMatrix(M3);
    printf("\n");
    printf("M4\n");
    displayMatrix(M4);
    
    // === 2. TESTE: Adição, Subtração e Multiplicação por Escalar ===
    printf("\n\n--- TESTE: OPERACOES BASICAS ---\n");
    Matrix* add_res = addMatrix(M3, M4);
    printf("\nAdicao (M3 + M4):\n");
    displayMatrix(add_res);

    Matrix* sub_res = subMatrix(M3, M4);
    printf("\nSubtracao (M3 - M4):\n");
    displayMatrix(sub_res);
    
    double scalar = 2.5;
    Matrix* scalar_res = scalarMultiply(M3, scalar);
    printf("\nMultiplicacao por Escalar (M3 * %.1f):\n", scalar);
    displayMatrix(scalar_res);

    // === 3. TESTE: Multiplicação e Transposição ===
    printf("\n\n--- TESTE: MULTIPLICACAO E TRANSPOSICAO ---\n");
    displayMatrix(M1);
    printf("\n");
    displayMatrix(M2);

    Matrix* mul_res = multiplyMatrix(M1, M2);
    printf("\nMultiplicacao (M1 * M2):\n");
    displayMatrix(mul_res);

    Matrix* transpose_res = transposeMatrix(mul_res);
    printf("\nTransposta do Resultado:\n");
    displayMatrix(transpose_res);

    // === 4. TESTE: Determinante e Inversão (Caso Válido) ===
    printf("\n\n--- TESTE: DETERMINANTE E INVERSAO (MATRIZ INVERTIVEL) ---\n");
    displayMatrix(M5_invertible);
    
    double det_res = determinant(M5_invertible);
    printf("\nDeterminante: %.2f\n", det_res);

    Matrix* inv_res = inverseMatrix(M5_invertible);
    printf("\nMatriz Inversa:\n");
    displayMatrix(inv_res);

    printf("\nVerificacao (Original * Inversa = Identidade):\n");
    Matrix* identity_res = multiplyMatrix(M5_invertible, inv_res);
    displayMatrix(identity_res);

    // === 5. TESTE: Determinante e Inversão (Caso Singular) ===
    printf("\n\n--- TESTE: INVERSAO (MATRIZ SINGULAR/NAO INVERTIVEL) ---\n");
    displayMatrix(M6_singular);

    double det_singular = determinant(M6_singular);
    printf("\nDeterminante: %.2f\n", det_singular);

    Matrix* inv_singular_res = inverseMatrix(M6_singular);
    if (inv_singular_res == NULL) {
        printf("\nResultado: A matriz nao e invertivel, como esperado.\n");
    }

    freeMatrix(M1);
    freeMatrix(M2);
    freeMatrix(M3);
    freeMatrix(M4);
    freeMatrix(M5_invertible);
    freeMatrix(M6_singular);

    freeMatrix(add_res);
    freeMatrix(sub_res);
    freeMatrix(scalar_res);
    freeMatrix(mul_res);
    freeMatrix(transpose_res);
    freeMatrix(inv_res);
    freeMatrix(identity_res);
    freeMatrix(inv_singular_res);

    return 0;
}
