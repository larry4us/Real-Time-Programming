#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include "matrixOperations.h"
#include <sys/time.h>
#include <time.h>
#include <termios.h> // Para controle do terminal
#include <fcntl.h>   // Para controle de arquivos
#include <sched.h>
#include <sys/mman.h>

// --- Constantes da Simulação ---
#define SIMULATION_TIME 20.0
#define ROBOT_DIAMETER 0.6
#define R_ROBOT (ROBOT_DIAMETER / 2.0)
#define PI 3.14159265358979323846

// --- Períodos das Threads (em milissegundos) ---
#define ROBOT_SIM_PERIOD_MS 30
#define LINEARIZATION_PERIOD_MS 40
#define CONTROL_PERIOD_MS 50
#define REF_MODEL_X_PERIOD_MS 50
#define REF_MODEL_Y_PERIOD_MS 50
#define REFERENCE_GEN_PERIOD_MS 120
#define LOGGER_PERIOD_MS 100

// --- Variáveis Compartilhadas e Mutexes ---
double current_time = 0.0;
pthread_mutex_t time_mutex;

// Estado do robô: x_state = [xc, yc, theta]T
Matrix* x_state;
pthread_mutex_t x_state_mutex;

// Saída do robô: y_output = [y1, y2]T
Matrix* y_output;
pthread_mutex_t y_output_mutex;

// Entrada de controle linearizada: v_input = [v1, v2]T
Matrix* v_input;
pthread_mutex_t v_input_mutex;

// Entrada do robô: u_input = [v, w]T
Matrix* u_input;
pthread_mutex_t u_input_mutex;

// Saídas do modelo de referência: ym = [ymx, ymy]T
Matrix* ym_output;
pthread_mutex_t ym_output_mutex;

// Derivadas do modelo de referência: ym_dot = [ymx_dot, ymy_dot]T
Matrix* ym_dot_output;
pthread_mutex_t ym_dot_output_mutex;

// Referência: ref = [xref, yref]T
Matrix* ref_input;
pthread_mutex_t ref_input_mutex;

// Parâmetros do controlador
double alpha1 = 3.0;
double alpha2 = 3.0;
pthread_mutex_t alpha_mutex;

// --- Protótipos das Funções das Threads ---
void* robot_simulation_thread(void* arg);
void* linearization_thread(void* arg);
void* control_thread(void* arg);
void* ref_model_x_thread(void* arg);
void* ref_model_y_thread(void* arg);
void* reference_generation_thread(void* arg);
void* user_interface_thread(void* arg);

// Adiciona milissegundos a um timespec, tratando overflow de nanosegundos
void timespec_add_ms(struct timespec *t, long ms) {
    t->tv_sec += ms / 1000;
    t->tv_nsec += (ms % 1000) * 1000000;
    if (t->tv_nsec >= 1000000000) {
        t->tv_nsec -= 1000000000;
        t->tv_sec++;
    }
}

// Configura prioridade (sched_setscheduler wrapper)
void configure_rt_thread(pthread_attr_t *attr, int priority) {
    struct sched_param param;
    pthread_attr_init(attr);
    pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED); 
    pthread_attr_setschedpolicy(attr, SCHED_FIFO); 
    param.sched_priority = priority;
    pthread_attr_setschedparam(attr, &param);
}

void write_timing_info(FILE* file, struct timespec* last_time) {
    struct timespec current_spec;
    clock_gettime(CLOCK_MONOTONIC, &current_spec);
    double delta_s = (current_spec.tv_sec - last_time->tv_sec);
    double delta_ns = (current_spec.tv_nsec - last_time->tv_nsec);
    double period_ms = delta_s * 1000.0 + delta_ns / 1e6;
    if (period_ms > 0) {
        fprintf(file, "%f\n", period_ms);
    }
    *last_time = current_spec;
}

// Antiga função de simulação de carga
// Agora a forma que estou usando no trabalho é o stress da CPU via terminal.
void simulate_load(long duration_ms) {
    long iterations = duration_ms * 10000; 
    double result = 0.0;
    for (long i = 0; i < iterations; i++) {
        result += sin(i) * tan(i);
    }
}

int main() {
    pthread_t tid_robot, tid_linear, tid_control, tid_ref_x, tid_ref_y, tid_ref_gen, tid_logger;
    
    pthread_attr_t attr_robot, attr_linear, attr_control;
    pthread_attr_t attr_ref_x, attr_ref_y, attr_ref_gen, attr_logger;

    // 1. Bloqueio de Memória (Page Fault Prevention)
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("AVISO: Falha ao bloquear memória (mlockall). Execute com SUDO para comportamento RT real.");
    } else {
        printf("Memória bloqueada com sucesso (RT Safe).\n");
    }

    // Inicialização das variáveis
    x_state = createMatrix(3, 1); 
    y_output = createMatrix(2, 1); 
    v_input = createMatrix(2, 1);
    u_input = createMatrix(2, 1);
    ym_output = createMatrix(2, 1);
    ym_dot_output = createMatrix(2, 1);
    ref_input = createMatrix(2, 1);

    // Inicialização dos Mutexes
    pthread_mutex_init(&time_mutex, NULL);
    pthread_mutex_init(&x_state_mutex, NULL);
    pthread_mutex_init(&y_output_mutex, NULL);
    pthread_mutex_init(&v_input_mutex, NULL);
    pthread_mutex_init(&u_input_mutex, NULL);
    pthread_mutex_init(&ym_output_mutex, NULL);
    pthread_mutex_init(&ym_dot_output_mutex, NULL);
    pthread_mutex_init(&ref_input_mutex, NULL);
    pthread_mutex_init(&alpha_mutex, NULL);

    // 2. Definição de Prioridades (RMS)
    configure_rt_thread(&attr_robot, 80);      // 30ms
    configure_rt_thread(&attr_linear, 70);     // 40ms
    configure_rt_thread(&attr_control, 60);    // 50ms
    configure_rt_thread(&attr_ref_x, 60);      // 50ms
    configure_rt_thread(&attr_ref_y, 60);      // 50ms
    configure_rt_thread(&attr_logger, 30);     // 100ms (UI)
    configure_rt_thread(&attr_ref_gen, 20);    // 120ms

    printf("Iniciando threads com escalonamento SCHED_FIFO e Loop Temporal Absoluto...\n");

    pthread_create(&tid_ref_gen, &attr_ref_gen, reference_generation_thread, NULL);
    pthread_create(&tid_ref_x, &attr_ref_x, ref_model_x_thread, NULL);
    pthread_create(&tid_ref_y, &attr_ref_y, ref_model_y_thread, NULL);
    pthread_create(&tid_control, &attr_control, control_thread, NULL);
    pthread_create(&tid_linear, &attr_linear, linearization_thread, NULL);
    pthread_create(&tid_robot, &attr_robot, robot_simulation_thread, NULL);
    pthread_create(&tid_logger, &attr_logger, user_interface_thread, NULL);

    pthread_join(tid_logger, NULL);

    // Cleanup
    pthread_attr_destroy(&attr_robot);
    pthread_attr_destroy(&attr_linear);
    pthread_attr_destroy(&attr_control);
    pthread_attr_destroy(&attr_ref_x);
    pthread_attr_destroy(&attr_ref_y);
    pthread_attr_destroy(&attr_ref_gen);
    pthread_attr_destroy(&attr_logger);

    freeMatrix(x_state);
    freeMatrix(y_output);
    freeMatrix(v_input);
    freeMatrix(u_input);
    freeMatrix(ym_output);
    freeMatrix(ym_dot_output);
    freeMatrix(ref_input);
    
    pthread_mutex_destroy(&time_mutex);
    pthread_mutex_destroy(&x_state_mutex);
    pthread_mutex_destroy(&y_output_mutex);
    pthread_mutex_destroy(&v_input_mutex);
    pthread_mutex_destroy(&u_input_mutex);
    pthread_mutex_destroy(&ym_output_mutex);
    pthread_mutex_destroy(&ym_dot_output_mutex);
    pthread_mutex_destroy(&ref_input_mutex);
    pthread_mutex_destroy(&alpha_mutex);

    printf("Simulação concluída. Execute 'make plot' para ver os resultados.\n");
    return 0;
}

// --- Implementação das Threads ---

void* reference_generation_thread(void* arg) {
    FILE* timing_file = fopen("output/ref_gen_timing.txt", "w");
    if (!timing_file) return NULL;
    fprintf(timing_file, "T(k)\n");

    struct timespec next_wake_time;
    struct timespec last_time; // Apenas para log estatístico
    
    clock_gettime(CLOCK_MONOTONIC, &next_wake_time);
    last_time = next_wake_time;

    while (current_time < SIMULATION_TIME) {
        pthread_mutex_lock(&time_mutex);
        double t = current_time;
        pthread_mutex_unlock(&time_mutex);
        
        double xref_val = (5.0 / PI) * cos(0.2 * PI * t);
        double yref_val = (t < 10.0) ? (5.0 / PI) * sin(0.2 * PI * t) : -(5.0 / PI) * sin(0.2 * PI * t);

        pthread_mutex_lock(&ref_input_mutex);
        ref_input->data[0][0] = xref_val;
        ref_input->data[1][0] = yref_val;
        pthread_mutex_unlock(&ref_input_mutex);

        // Lógica de Tempo Absoluto
        timespec_add_ms(&next_wake_time, REFERENCE_GEN_PERIOD_MS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wake_time, NULL);
        
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* ref_model_x_thread(void* arg) {
    FILE* timing_file = fopen("output/ref_model_x_timing.txt", "w");
    if (!timing_file) return NULL;
    fprintf(timing_file, "T(k)\n");

    struct timespec next_wake_time;
    struct timespec last_time;
    
    clock_gettime(CLOCK_MONOTONIC, &next_wake_time);
    last_time = next_wake_time;

    double ymx = 0.0;
    double dt = REF_MODEL_X_PERIOD_MS / 1000.0;

    while (current_time < SIMULATION_TIME) {
        pthread_mutex_lock(&ref_input_mutex);
        double xref = ref_input->data[0][0];
        pthread_mutex_unlock(&ref_input_mutex);

        pthread_mutex_lock(&alpha_mutex);
        double a1 = alpha1;
        pthread_mutex_unlock(&alpha_mutex);

        double ymx_dot = a1 * (xref - ymx);
        ymx += ymx_dot * dt;

        pthread_mutex_lock(&ym_output_mutex);
        ym_output->data[0][0] = ymx;
        pthread_mutex_unlock(&ym_output_mutex);
        
        pthread_mutex_lock(&ym_dot_output_mutex);
        ym_dot_output->data[0][0] = ymx_dot;
        pthread_mutex_unlock(&ym_dot_output_mutex);

        // Lógica de Tempo Absoluto
        timespec_add_ms(&next_wake_time, REF_MODEL_X_PERIOD_MS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wake_time, NULL);
        
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* ref_model_y_thread(void* arg) {
    FILE* timing_file = fopen("output/ref_model_y_timing.txt", "w");
    if (!timing_file) return NULL;
    fprintf(timing_file, "T(k)\n");

    struct timespec next_wake_time;
    struct timespec last_time;

    clock_gettime(CLOCK_MONOTONIC, &next_wake_time);
    last_time = next_wake_time;

    double ymy = 0.0;
    double dt = REF_MODEL_Y_PERIOD_MS / 1000.0;

    while (current_time < SIMULATION_TIME) {
        pthread_mutex_lock(&ref_input_mutex);
        double yref = ref_input->data[1][0];
        pthread_mutex_unlock(&ref_input_mutex);

        pthread_mutex_lock(&alpha_mutex);
        double a2 = alpha2;
        pthread_mutex_unlock(&alpha_mutex);

        double ymy_dot = a2 * (yref - ymy);
        ymy += ymy_dot * dt;

        pthread_mutex_lock(&ym_output_mutex);
        ym_output->data[1][0] = ymy;
        pthread_mutex_unlock(&ym_output_mutex);

        pthread_mutex_lock(&ym_dot_output_mutex);
        ym_dot_output->data[1][0] = ymy_dot;
        pthread_mutex_unlock(&ym_dot_output_mutex);

        // Lógica de Tempo Absoluto
        timespec_add_ms(&next_wake_time, REF_MODEL_Y_PERIOD_MS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wake_time, NULL);
        
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* control_thread(void* arg) {
    FILE* timing_file = fopen("output/control_timing.txt", "w");
    if (!timing_file) return NULL;
    fprintf(timing_file, "T(k)\n");

    struct timespec next_wake_time;
    struct timespec last_time;

    clock_gettime(CLOCK_MONOTONIC, &next_wake_time);
    last_time = next_wake_time;

    while (current_time < SIMULATION_TIME) {
        pthread_mutex_lock(&y_output_mutex);
        double y1 = y_output->data[0][0];
        double y2 = y_output->data[1][0];
        pthread_mutex_unlock(&y_output_mutex);

        pthread_mutex_lock(&ym_output_mutex);
        double ymx = ym_output->data[0][0];
        double ymy = ym_output->data[1][0];
        pthread_mutex_unlock(&ym_output_mutex);

        pthread_mutex_lock(&ym_dot_output_mutex);
        double ymx_dot = ym_dot_output->data[0][0];
        double ymy_dot = ym_dot_output->data[1][0];
        pthread_mutex_unlock(&ym_dot_output_mutex);
        
        pthread_mutex_lock(&alpha_mutex);
        double a1 = alpha1;
        double a2 = alpha2;
        pthread_mutex_unlock(&alpha_mutex);

        double v1 = ymx_dot + a1 * (ymx - y1);
        double v2 = ymy_dot + a2 * (ymy - y2);

        pthread_mutex_lock(&v_input_mutex);
        v_input->data[0][0] = v1;
        v_input->data[1][0] = v2;
        pthread_mutex_unlock(&v_input_mutex);

        // Lógica de Tempo Absoluto
        timespec_add_ms(&next_wake_time, CONTROL_PERIOD_MS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wake_time, NULL);
        
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* linearization_thread(void* arg) {
    FILE* timing_file = fopen("output/linearization_timing.txt", "w");
    if (!timing_file) return NULL;
    fprintf(timing_file, "T(k)\n");

    struct timespec next_wake_time;
    struct timespec last_time;

    clock_gettime(CLOCK_MONOTONIC, &next_wake_time);
    last_time = next_wake_time;

    while (current_time < SIMULATION_TIME) {
        pthread_mutex_lock(&x_state_mutex);
        double theta = x_state->data[2][0];
        pthread_mutex_unlock(&x_state_mutex);
        
        pthread_mutex_lock(&v_input_mutex);
        Matrix* v = createMatrix(2, 1);
        v->data[0][0] = v_input->data[0][0];
        v->data[1][0] = v_input->data[1][0];
        pthread_mutex_unlock(&v_input_mutex);
        
        Matrix* L = createMatrix(2, 2);
        L->data[0][0] = cos(theta);
        L->data[0][1] = -R_ROBOT * sin(theta);
        L->data[1][0] = sin(theta);
        L->data[1][1] = R_ROBOT * cos(theta);

        Matrix* L_inv = inverseMatrix(L);
        if (L_inv) {
            Matrix* u = multiplyMatrix(L_inv, v);
            
            pthread_mutex_lock(&u_input_mutex);
            u_input->data[0][0] = u->data[0][0];
            u_input->data[1][0] = u->data[1][0];
            pthread_mutex_unlock(&u_input_mutex);

            freeMatrix(u);
            freeMatrix(L_inv);
        }
        
        freeMatrix(L);
        freeMatrix(v);

        // Lógica de Tempo Absoluto
        timespec_add_ms(&next_wake_time, LINEARIZATION_PERIOD_MS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wake_time, NULL);
        
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* robot_simulation_thread(void* arg) {
    FILE* timing_file = fopen("output/robot_sim_timing.txt", "w");
    if (!timing_file) return NULL;
    fprintf(timing_file, "T(k)\n");

    struct timespec next_wake_time;
    struct timespec last_time;

    clock_gettime(CLOCK_MONOTONIC, &next_wake_time);
    last_time = next_wake_time;
    
    double dt = ROBOT_SIM_PERIOD_MS / 1000.0;

    while (current_time < SIMULATION_TIME) {
        pthread_mutex_lock(&u_input_mutex);
        Matrix* u = createMatrix(2, 1);
        u->data[0][0] = u_input->data[0][0]; // v
        u->data[1][0] = u_input->data[1][0]; // w
        pthread_mutex_unlock(&u_input_mutex);

        pthread_mutex_lock(&x_state_mutex);
        double theta = x_state->data[2][0];
        
        Matrix* x_dot_calc = createMatrix(3, 2);
        x_dot_calc->data[0][0] = cos(theta);
        x_dot_calc->data[1][0] = sin(theta);
        x_dot_calc->data[2][1] = 1.0;
        
        Matrix* u_vec_for_x_dot = createMatrix(2,1);
        u_vec_for_x_dot->data[0][0] = u->data[0][0];
        u_vec_for_x_dot->data[1][0] = u->data[1][0];
        
        Matrix* x_dot = multiplyMatrix(x_dot_calc, u_vec_for_x_dot);
        Matrix* term = scalarMultiply(x_dot, dt);
        Matrix* x_next = addMatrix(x_state, term);
        
        // Atualiza o estado
        freeMatrix(x_state);
        x_state = x_next;
        
        // Calcula a nova saída y
        double new_xc = x_state->data[0][0];
        double new_yc = x_state->data[1][0];
        double new_theta = x_state->data[2][0];
        pthread_mutex_unlock(&x_state_mutex);

        pthread_mutex_lock(&y_output_mutex);
        y_output->data[0][0] = new_xc + R_ROBOT * cos(new_theta);
        y_output->data[1][0] = new_yc + R_ROBOT * sin(new_theta);
        pthread_mutex_unlock(&y_output_mutex);

        freeMatrix(u);
        freeMatrix(u_vec_for_x_dot);
        freeMatrix(x_dot_calc);
        freeMatrix(x_dot);
        freeMatrix(term);
        
        pthread_mutex_lock(&time_mutex);
        current_time += dt;
        pthread_mutex_unlock(&time_mutex);

        // Lógica de Tempo Absoluto (Determinismo)
        timespec_add_ms(&next_wake_time, ROBOT_SIM_PERIOD_MS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wake_time, NULL);
        
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* user_interface_thread(void* arg) {
    FILE* output_file = fopen("output/simulation_output.txt", "w");
    if (!output_file) return NULL;
    fprintf(output_file, "t\tx\ty\ttheta\txref\tyref\n");

    FILE* timing_file = fopen("output/logger_timing.txt", "w");
    fprintf(timing_file, "T(k)\n");
    
    struct timespec next_wake_time;
    struct timespec last_time;

    clock_gettime(CLOCK_MONOTONIC, &next_wake_time);
    last_time = next_wake_time;

    // --- Configuração do terminal para leitura ---
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    while (current_time < SIMULATION_TIME) {
        // --- Leitura do teclado ---
        ch = getchar();
        if (ch != EOF) {
            pthread_mutex_lock(&alpha_mutex);
            if (ch == 'q') alpha1 += 0.1;
            if (ch == 'a') alpha1 = (alpha1 > 0.1) ? alpha1 - 0.1 : 0.1;
            if (ch == 'w') alpha2 += 0.1;
            if (ch == 's') alpha2 = (alpha2 > 0.1) ? alpha2 - 0.1 : 0.1;
            pthread_mutex_unlock(&alpha_mutex);
        }

        pthread_mutex_lock(&time_mutex);
        double t = current_time;
        pthread_mutex_unlock(&time_mutex);

        pthread_mutex_lock(&y_output_mutex);
        double y1 = y_output->data[0][0];
        double y2 = y_output->data[1][0];
        pthread_mutex_unlock(&y_output_mutex);

        pthread_mutex_lock(&x_state_mutex);
        double theta = x_state->data[2][0];
        pthread_mutex_unlock(&x_state_mutex);

        pthread_mutex_lock(&ref_input_mutex);
        double xref = ref_input->data[0][0];
        double yref = ref_input->data[1][0];
        pthread_mutex_unlock(&ref_input_mutex);
        
        pthread_mutex_lock(&alpha_mutex);
        double a1_val = alpha1;
        double a2_val = alpha2;
        pthread_mutex_unlock(&alpha_mutex);

        // --- Exibição na Tela ---
        printf("\033[H\033[J"); // Limpa o console
        printf("--- Simulação Robô Lab 5 (RTOS) ---\n");
        printf("Tempo: %.2f / %.2f s\n\n", t, SIMULATION_TIME);
        printf("Posição Robô (x, y):     (%.3f, %.3f)\n", y1, y2);
        printf("Referência   (xref, yref): (%.3f, %.3f)\n", xref, yref);
        printf("Orientação (theta):      %.3f rad\n\n", theta);
        printf("--- Controle ---\n");
        printf("alpha1: %.2f  (q: aumenta | a: diminui)\n", a1_val);
        printf("alpha2: %.2f  (w: aumenta | s: diminui)\n", a2_val);
        fflush(stdout); 

        // Grava no arquivo de log
        fprintf(output_file, "%f\t%f\t%f\t%f\t%f\t%f\n", t, y1, y2, theta, xref, yref);
        
        // Lógica de Tempo Absoluto
        timespec_add_ms(&next_wake_time, LOGGER_PERIOD_MS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wake_time, NULL);
        
        write_timing_info(timing_file, &last_time);
    }

    // --- Restaura as configurações do terminal ---
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    fclose(output_file);
    fclose(timing_file);
    printf("\n\nInterface finalizada. Log salvo.\n");
    return NULL;
}