#include "integration.h"

double riemann_sum(math_func_t func, double a, double b, int n_steps) {
    // Retorna 0 se o número de passos for inválido ou o intervalo for nulo.
    if (n_steps <= 0 || a == b) {
        return 0.0;
    }

    double total_area = 0.0;
    double dx = (b - a) / n_steps; // Largura de cada retângulo

    // Itera por cada retângulo, usando o ponto médio para calcular a altura.
    for (int i = 0; i < n_steps; i++) {
        // Ponto x no meio do retângulo atual
        double mid_point = a + (i + 0.5) * dx;
        
        // Altura do retângulo é o valor da função no ponto médio
        double height = func(mid_point);
        
        // Soma a área do retângulo atual (altura * largura) à área total
        total_area += height * dx;
    }

    return total_area;
}