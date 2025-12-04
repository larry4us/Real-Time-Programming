# Makefile - Lab 5 RTOS
CC = gcc
CFLAGS = -g -Wall -Iinclude
LIBS = -lm -lpthread
PYTHON = python3

# Diretórios
SRC_DIR = src
TEST_DIR = tests
BIN_DIR = bin
OBJ_DIR = obj
DISPLAY_SCRIPT_DIR = displayScripts
OUTPUT_DIR = output

# Fontes e Objetos
LIB_SOURCES = $(SRC_DIR)/matrixOperations.c $(SRC_DIR)/integration.c
LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(LIB_SOURCES))

APP_MAIN_OBJ = $(OBJ_DIR)/main.o
APP_TARGET = $(BIN_DIR)/app_final

MATRIX_TEST_TARGET = $(BIN_DIR)/teste_matriz
INTEGRATION_TEST_TARGET = $(BIN_DIR)/teste_integracao

# Scripts
PLOT_TRAJECTORY = $(DISPLAY_SCRIPT_DIR)/plot_trajectory.py
ANALYZE_TIMING = $(DISPLAY_SCRIPT_DIR)/analyze_timing.py

.SPECIAL: .PRECIOUS
.PRECIOUS: $(APP_TARGET)

.PHONY: all run plot test run-tests clean

# Padrão: Compila e Roda
all: run

# Regra de Execução (SUDO necessário para RTOS)
run: $(APP_TARGET)
	@echo ">>> Executando simulação RTOS (Sudo necessário)..."
	sudo ./$(APP_TARGET)

# --- CORREÇÃO: Plotagem independente ---
# Agora 'plot' não depende mais de 'run'. 
# Ele apenas processa os arquivos já existentes em 'output/'.
plot:
	@echo ">>> Gerando gráficos a partir dos logs existentes..."
	$(PYTHON) $(PLOT_TRAJECTORY)
	$(PYTHON) $(ANALYZE_TIMING)

# Regras de Teste
test: $(MATRIX_TEST_TARGET) $(INTEGRATION_TEST_TARGET)

run-tests: test
	@echo ">>> Rodando Testes..."
	./$(MATRIX_TEST_TARGET)
	./$(INTEGRATION_TEST_TARGET)

# Regras de Compilação
$(APP_TARGET): $(APP_MAIN_OBJ) $(LIB_OBJECTS)
	@mkdir -p $(BIN_DIR) $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo ">>> Build concluído."

$(MATRIX_TEST_TARGET): $(OBJ_DIR)/matrixTests.o $(OBJ_DIR)/matrixOperations.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(INTEGRATION_TEST_TARGET): $(OBJ_DIR)/integrationTests.o $(OBJ_DIR)/integration.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR) $(OUTPUT_DIR) *.txt *.png