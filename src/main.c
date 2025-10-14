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
// void* logger_thread(void* arg);
void* user_interface_thread(void* arg);

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

// --- Função Principal ---
int main() {
    pthread_t tid_robot, tid_linear, tid_control, tid_ref_x, tid_ref_y, tid_ref_gen, tid_logger;

    // Inicialização das variáveis
    x_state = createMatrix(3, 1); // [xc, yc, theta]
    y_output = createMatrix(2, 1); // [y1, y2]
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

    // Criação das Threads
    pthread_create(&tid_ref_gen, NULL, reference_generation_thread, NULL);
    pthread_create(&tid_ref_x, NULL, ref_model_x_thread, NULL);
    pthread_create(&tid_ref_y, NULL, ref_model_y_thread, NULL);
    pthread_create(&tid_control, NULL, control_thread, NULL);
    pthread_create(&tid_linear, NULL, linearization_thread, NULL);
    pthread_create(&tid_robot, NULL, robot_simulation_thread, NULL);
    //pthread_create(&tid_logger, NULL, logger_thread, NULL);
    pthread_create(&tid_logger, NULL, user_interface_thread, NULL);

    // Aguarda o término das threads
    pthread_join(tid_logger, NULL);
    // As outras threads são terminadas quando a simulação acaba

    // Liberação de recursos
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

// Executa cálculos intensos pra simular uma carga de trabalho na CPU.
void simulate_load(long duration_ms) {
    long iterations = duration_ms * 10000; // Ajuste conforme necessário. Depende do hardware de quem está executando
    double result = 0.0;
    for (long i = 0; i < iterations; i++) {
        result += sin(i) * tan(i);
    }
}

void* reference_generation_thread(void* arg) {
    FILE* timing_file = fopen("output/ref_gen_timing.txt", "w");
    if (!timing_file) {
        perror("Erro ao abrir o arquivo de timing da geração de referência");
        return NULL;
    }
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    fprintf(timing_file, "T(k)\n");

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

        usleep(REFERENCE_GEN_PERIOD_MS * 1000);
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* ref_model_x_thread(void* arg) {
    FILE* timing_file = fopen("output/ref_model_x_timing.txt", "w");
    if (!timing_file) {
        perror("Erro ao abrir o arquivo de timing do modelo de referência X");
        return NULL;
    }
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    double ymx = 0.0;
    double dt = REF_MODEL_X_PERIOD_MS / 1000.0;

    fprintf(timing_file, "T(k)\n");

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

        usleep(REF_MODEL_X_PERIOD_MS * 1000);
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* ref_model_y_thread(void* arg) {
    FILE* timing_file = fopen("output/ref_model_y_timing.txt", "w");
    if (!timing_file) {
        perror("Erro ao abrir o arquivo de timing do modelo de referência Y");
        return NULL;
    }
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);
    
    fprintf(timing_file, "T(k)\n");

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

        usleep(REF_MODEL_Y_PERIOD_MS * 1000);
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* control_thread(void* arg) {
    FILE* timing_file = fopen("output/control_timing.txt", "w");
    if (!timing_file) {
        perror("Erro ao abrir o arquivo de timing do controle");
        return NULL;
    }
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    fprintf(timing_file, "T(k)\n");

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

        usleep(CONTROL_PERIOD_MS * 1000);
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* linearization_thread(void* arg) {
    FILE* timing_file = fopen("output/linearization_timing.txt", "w");
    if (!timing_file) {
        perror("Erro ao abrir o arquivo de timing da linearização");
        return NULL;
    }
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    fprintf(timing_file, "T(k)\n");

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

        usleep(LINEARIZATION_PERIOD_MS * 1000);
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* robot_simulation_thread(void* arg) {
    FILE* timing_file = fopen("output/robot_sim_timing.txt", "w");
    if (!timing_file) {
        perror("Erro ao abrir o arquivo de timing do robô");
        return NULL;
    }
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);
    
    double dt = ROBOT_SIM_PERIOD_MS / 1000.0;

    fprintf(timing_file, "T(k)\n");

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

        usleep(ROBOT_SIM_PERIOD_MS * 1000);
        write_timing_info(timing_file, &last_time);
    }
    fclose(timing_file);
    return NULL;
}

void* user_interface_thread(void* arg) {
    FILE* output_file = fopen("output/simulation_output.txt", "w");
    if (!output_file) {
        perror("Erro ao abrir o arquivo de saída");
        return NULL;
    }
    //fprintf(output_file, "t\ty1\ty2\ttheta\txref\tyref\n");
    fprintf(output_file, "t\tx\ty\ttheta\txref\tyref\n");

    FILE* timing_file = fopen("output/logger_timing.txt", "w");
    fprintf(timing_file, "T(k)\n");
    
    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    // --- Configuração do terminal para leitura não bloqueante ---
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    // ---------------------------------------------------------

    while (current_time < SIMULATION_TIME) {
        // --- Leitura do teclado para alterar alphas ---
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
        printf("--- Simulação Robô Lab 3 ---\n");
        printf("Tempo: %.2f / %.2f s\n\n", t, SIMULATION_TIME);
        printf("Posição Robô (y1, y2):   (%.3f, %.3f)\n", y1, y2);
        printf("Referência   (xref, yref): (%.3f, %.3f)\n", xref, yref);
        printf("Orientação (theta):      %.3f rad\n\n", theta);
        printf("--- Controle ---\n");
        printf("alpha1: %.2f  (q: aumenta | a: diminui)\n", a1_val);
        printf("alpha2: %.2f  (w: aumenta | s: diminui)\n", a2_val);
        fflush(stdout); // Garante que o texto seja impresso imediatamente

        // Grava no arquivo de log
        fprintf(output_file, "%f\t%f\t%f\t%f\t%f\t%f\n", t, y1, y2, theta, xref, yref);
        
        usleep(LOGGER_PERIOD_MS * 1000);
        write_timing_info(timing_file, &last_time);
    }

    // --- Restaura as configurações do terminal ---
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    // -------------------------------------------

    fclose(output_file);
    fclose(timing_file);
    printf("\n\nInterface finalizada. Log salvo.\n");
    return NULL;
}