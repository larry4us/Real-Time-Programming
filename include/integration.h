#ifndef INTEGRATION_H
#define INTEGRATION_H

/* 
* define um tipo para um ponteiro de função que representa
 * uma função matemática, ex: f(x). */
typedef double (*math_func_t)(double);
double riemann_sum(math_func_t func, double a, double b, int n_steps);


#endif // INTEGRATION_H