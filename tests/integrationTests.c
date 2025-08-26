#include <stdio.h>
#define _USE_MATH_DEFINES 
#include <math.h>
#include "integration.h" 

// Exemplo 1: Uma função quadrática simples: f(x) = x^2
double f_quadratic(double x) {
    return x * x;
}

// Exemplo 2: Uma função linear: f(x) = 2x + 1
double f_linear(double x) {
    return 2 * x + 1;
}

// Exemplo 3: A função seno: f(x) = sin(x)
double f_sin(double x) {
    return sin(x);
}

int main() {
    int steps = 10000; // Define a precisão da integração
    double result;

    printf("Usando %d passos para a integracao.\n\n", steps);

    result = riemann_sum(f_quadratic, 0.0, 1.0, steps);
    printf("Integral de f(x) = x^2 de 0 a 1:\n");
    printf("  Resultado numerico: %f\n", result);
    printf("  Resultado analitico: ~0.333333\n\n");

    result = riemann_sum(f_linear, 0.0, 5.0, steps);
    printf("Integral de f(x) = 2x + 1 de 0 a 5:\n");
    printf("  Resultado numerico: %f\n", result);
    printf("  Resultado analitico: 30.0\n\n");

    result = riemann_sum(f_sin, 0.0, M_PI, steps);
    printf("Integral de f(x) = sin(x) de 0 a PI:\n");
    printf("  Resultado numerico: %f\n", result);
    printf("  Resultado analitico: 2.0\n\n\n");

    return 0;
}