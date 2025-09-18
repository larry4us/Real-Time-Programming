#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "matrixOperations.h"

// --- Constantes da Simulação ---
#define SIMULATION_TIME 20.0   // segundos
#define THREAD1_PERIOD_MS 30   // ms
#define THREAD2_PERIOD_MS 50   // ms
#define ROBOT_DIAMETER 0.3     // metros
#define V_LINEAR 1.0           // m/s (velocidade linear constante)
#define PI 3.14159265358979323846

typedef struct {
    double v;
    double omega;
} InputVector;

// Dados compartilhados entre as threads
InputVector shared_u;
Matrix* shared_y; // Vetor de estado y(t) = x(t) = [x, y, θ]ᵀ

// Mutex para proteger o acesso aos dados compartilhados
pthread_mutex_t u_mutex;
pthread_mutex_t y_mutex;

// Variável global para controlar o tempo
double current_time = 0.0;

// --- Funções das Threads ---

// VERSÃO CORRIGIDA da u_matrix
Matrix* u_matrix(double time) {
    Matrix* u = createMatrix(2, 1);
    if (u == NULL) return NULL;

    bool isBetweenZeroAndTen = (time >= 0.0 && time < 10.0);

    // Linha 0, Coluna 0: Velocidade linear 'v'
    u->data[0][0] = 1.0; 

    // Linha 1, Coluna 0: Velocidade angular 'ω'
    if (isBetweenZeroAndTen) {
        u->data[1][0] = 0.2 * PI;
    } else {
        u->data[1][0] = -0.2 * PI;
    }

    return u;
}

/*
X é a multiplicação de duas matrizes, sendo uma delas o próprio u(t).
*/
Matrix* differential_x_matrix(Matrix* u, double theta) {
    Matrix* matrix_A = createMatrix(3, 2);
    if (matrix_A == NULL) return NULL;

    // Preenche a matriz A conforme a fórmula do PDF
    // ẋ(t) = [ sin(θ)  0 ] * [ v ]
    //        [ cos(θ)  0 ]   [ ω ]
    //        [   0     1 ]

    matrix_A->data[0][0] = sin(theta);
    matrix_A->data[0][1] = 0.0;
    
    matrix_A->data[1][0] = cos(theta);
    matrix_A->data[1][1] = 0.0;

    matrix_A->data[2][0] = 0.0;
    matrix_A->data[2][1] = 1.0;

    Matrix* result = multiplyMatrix(matrix_A, u);
    
    freeMatrix(matrix_A); // Libera a matriz A, que era temporária
    return result;
}

void* thread2_func(void* arg) {
    double dt = THREAD2_PERIOD_MS / 1000.0; // Intervalo de tempo (0.05s)

    while (current_time <= SIMULATION_TIME) {
        // --- 1. Ler u(t) da área compartilhada ---
        pthread_mutex_lock(&u_mutex);
        Matrix* u = u_matrix(current_time); 
        pthread_mutex_unlock(&u_mutex);

        // --- 2. Obter o theta atual ---
        pthread_mutex_lock(&y_mutex);
        double current_theta = shared_y->data[2][0];
        pthread_mutex_unlock(&y_mutex);

        // --- 3. Calcular ẋ(t) ---
        Matrix* y_dot = differential_x_matrix(u, current_theta);
        
        // --- 4. INTEGRAR (Método de Euler) ---
        Matrix* term = scalarMultiply(y_dot, dt);
        
        pthread_mutex_lock(&y_mutex);
        Matrix* y_next = addMatrix(shared_y, term);
        
        // Atualiza a variável compartilhada
        freeMatrix(shared_y);
        shared_y = y_next;
        pthread_mutex_unlock(&y_mutex);

        // Libera memória
        freeMatrix(u);
        freeMatrix(y_dot);
        freeMatrix(term);
        
        // Dorme pelo período da thread
        usleep(THREAD2_PERIOD_MS * 1000);
    }
    return NULL;
}

/*
 * Thread 1: Gerador de Entrada e Amostrador (Período: 30ms)
 * - Gera o vetor de entrada u(t).
 * - Lê o vetor de estado y(t) do simulador.
 * - Salva os dados (t, u(t), y(t)) em um arquivo.
 */
void* thread1_func(void* arg) {
    FILE* output_file = fopen("simulation_output.txt", "w");
    if (output_file == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        return NULL;
    }
    fprintf(output_file, "t\tv\tw\tx\ty\ttheta\n");

    while (current_time <= SIMULATION_TIME) {
        // --- 1. Gera e compartilha u(t) ---
        // (A Thread 2 também calcula u(t), mas em um sistema real, a Thread 1
        // seria a única a definir o valor compartilhado. Para simplificar,
        // vamos apenas calcular os valores para o log aqui)
        Matrix* u = u_matrix(current_time);
        
        // --- 2. Lê y(t) da área compartilhada ---
        pthread_mutex_lock(&y_mutex);
        double x_pos = shared_y->data[0][0];
        double y_pos = shared_y->data[1][0];
        double theta = shared_y->data[2][0];
        pthread_mutex_unlock(&y_mutex);

        // --- 3. Salva os dados no arquivo ---
        fprintf(output_file, "%f\t%f\t%f\t%f\t%f\t%f\n", 
                current_time, 
                u->data[0][0], // v
                u->data[1][0], // ω
                x_pos, y_pos, theta);
        
        freeMatrix(u);

        // Dorme e atualiza o tempo
        usleep(THREAD1_PERIOD_MS * 1000);
        current_time += (THREAD1_PERIOD_MS / 1000.0);
    }

    fclose(output_file);
    return NULL;
}

// --- Função Principal ---
int main() {
    pthread_t tid1, tid2;

    // Inicializa o estado inicial y(0) = [0, 0, 0]ᵀ
    shared_y = createMatrix(3, 1);
    shared_y->data[0][0] = 0.0;
    shared_y->data[1][0] = 0.0;
    shared_y->data[2][0] = 0.0;

    // Inicializa os mutexes
    pthread_mutex_init(&u_mutex, NULL);
    pthread_mutex_init(&y_mutex, NULL);

    // Cria as threads
    pthread_create(&tid1, NULL, thread1_func, NULL);
    pthread_create(&tid2, NULL, thread2_func, NULL);

    // Espera as threads terminarem
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    // Destrói os mutexes e libera a memória
    pthread_mutex_destroy(&u_mutex);
    pthread_mutex_destroy(&y_mutex);
    freeMatrix(shared_y);

    printf("Simulação concluída. Resultados salvos em 'simulation_output.txt'.\n");
    printf("\n\nCaso queira ver os gráficos gerados com o matplotlib, execute:\n- make plot\n");

    return 0;
}