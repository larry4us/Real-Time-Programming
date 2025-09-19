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

/**
 * @brief Thread 2: Simulador (Período Nominal: 50ms).
 * - Acorda e avança o tempo da simulação.
 * - Calcula o vetor de entrada u(t).
 * - Calcula a derivada do estado ẋ(t).
 * - Integra para obter o novo estado y(t) e o atualiza na memória compartilhada.
 * - Mede o tempo real desde sua última execução (período T(k)).
 * - Salva o período em 'thread2_timing.txt'.
 */
void* thread2_func(void* arg) {
    // Abre o arquivo para escrita dos tempos
    FILE* timing_file = fopen("output/thread2_timing.txt", "w");
    if (timing_file == NULL) {
        perror("Erro ao abrir thread2_timing.txt");
        return NULL;
    }
    fprintf(timing_file, "T(k)\n");

    struct timespec last_time, current_time_spec;
    clock_gettime(CLOCK_MONOTONIC, &last_time); // Tempo inicial

    double simulation_step_time = THREAD2_PERIOD_MS / 1000.0;

    while (current_time <= SIMULATION_TIME) {
        // --- MEDIÇÃO DE TEMPO (INÍCIO) ---
        clock_gettime(CLOCK_MONOTONIC, &current_time_spec);
        double delta_s = (current_time_spec.tv_sec - last_time.tv_sec);
        double delta_ns = (current_time_spec.tv_nsec - last_time.tv_nsec);
        double period_ms = delta_s * 1000.0 + delta_ns / 1e6;
        fprintf(timing_file, "%f\n", period_ms);
        last_time = current_time_spec;
        // --- MEDIÇÃO DE TEMPO (FIM) ---

        // Para a simulação COM CARGA, descomente a linha abaixo
        // simulate_load();

        // --- Lógica da Simulação ---
        Matrix* u = u_matrix(current_time);

        pthread_mutex_lock(&y_mutex);
        double current_theta = shared_y->data[2][0];
        
        Matrix* y_dot = differential_x_matrix(u, current_theta);
        Matrix* term = scalarMultiply(y_dot, simulation_step_time);
        Matrix* y_next = addMatrix(shared_y, term);
        
        freeMatrix(shared_y);
        shared_y = y_next;
        pthread_mutex_unlock(&y_mutex);
        
        freeMatrix(u);
        freeMatrix(y_dot);
        freeMatrix(term);
        
        // Avança o tempo global da simulação e dorme
        current_time += simulation_step_time;
        usleep(THREAD2_PERIOD_MS * 1000);
    }

    fclose(timing_file);
    return NULL;
}

/**
 * @brief Thread 1: Logger (Período Nominal: 30ms).
 * - Acorda, lê o estado atual da simulação (y(t)).
 * - Salva o estado em 'simulation_output.txt'.
 * - Mede o tempo real desde sua última execução (período T(k)).
 * - Salva o período em 'thread1_timing.txt'.
 */
void* thread1_func(void* arg) {
    // Abre os arquivos para escrita
    FILE* output_file = fopen("output/simulation_output.txt", "w");
    if (output_file == NULL) {
        perror("Erro ao abrir simulation_output.txt");
        return NULL;
    }
    fprintf(output_file, "t\tv\tw\tx\ty\ttheta\n");

    FILE* timing_file = fopen("output/thread1_timing.txt", "w");
    if (timing_file == NULL) {
        perror("Erro ao abrir thread1_timing.txt");
        fclose(output_file);
        return NULL;
    }
    fprintf(timing_file, "T(k)\n");

    struct timespec last_time, current_time_spec;
    clock_gettime(CLOCK_MONOTONIC, &last_time); // Tempo inicial

    while (current_time <= SIMULATION_TIME) {
        // --- MEDIÇÃO DE TEMPO (INÍCIO) ---
        clock_gettime(CLOCK_MONOTONIC, &current_time_spec);
        double delta_s = (current_time_spec.tv_sec - last_time.tv_sec);
        double delta_ns = (current_time_spec.tv_nsec - last_time.tv_nsec);
        double period_ms = delta_s * 1000.0 + delta_ns / 1e6;
        fprintf(timing_file, "%f\n", period_ms);
        last_time = current_time_spec;
        // --- MEDIÇÃO DE TEMPO (FIM) ---

        // Para a simulação COM CARGA, descomente a linha abaixo
        // simulate_load();

        // --- Leitura e Log dos Dados da Simulação ---
        pthread_mutex_lock(&y_mutex);
        double time_snapshot = current_time;
        double x_pos = shared_y->data[0][0];
        double y_pos = shared_y->data[1][0];
        double theta = shared_y->data[2][0];
        pthread_mutex_unlock(&y_mutex);

        Matrix* u_log = u_matrix(time_snapshot);
        if (u_log) {
            fprintf(output_file, "%f\t%f\t%f\t%f\t%f\t%f\n",
                    time_snapshot, u_log->data[0][0], u_log->data[1][0],
                    x_pos, y_pos, theta);
            freeMatrix(u_log);
        }

        // Dorme pelo período nominal
        usleep(THREAD1_PERIOD_MS * 1000);
    }

    fclose(output_file);
    fclose(timing_file);
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