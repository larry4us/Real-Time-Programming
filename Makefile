CC = gcc
CFLAGS = -g -Wall -Iinclude
# Adicione -lpthread para linkar a biblioteca de threads
LIBS = -lm -lpthread

# Adicione o interpretador Python
PYTHON = python3

SRC_DIR = src
TEST_DIR = tests
BIN_DIR = bin
OBJ_DIR = obj

# --- Fontes da Biblioteca ---
LIB_SOURCES = $(SRC_DIR)/matrixOperations.c $(SRC_DIR)/integration.c
LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(LIB_SOURCES))

# --- Aplicação Principal ---
APP_MAIN_SRC = $(SRC_DIR)/main.c
APP_MAIN_OBJ = $(OBJ_DIR)/main.o
APP_TARGET = $(BIN_DIR)/app_final

# --- Teste de Matrizes ---
MATRIX_TEST_SRC = $(TEST_DIR)/matrixTests.c
MATRIX_TEST_OBJ = $(OBJ_DIR)/matrixTests.o
MATRIX_TEST_TARGET = $(BIN_DIR)/teste_matriz

# --- Teste de Integração ---
INTEGRATION_TEST_SRC = $(TEST_DIR)/integrationTests.c
INTEGRATION_TEST_OBJ = $(OBJ_DIR)/integrationTests.o
INTEGRATION_TEST_TARGET = $(BIN_DIR)/teste_integracao

# --- Regras ---

.PHONY: all test run-tests clean plot

all: $(APP_TARGET)

# NOVA REGRA: Roda a simulação e depois o script de plotagem
plot: $(APP_TARGET)
	@echo "--- Gerando o gráfico da trajetória ---"
	$(PYTHON) plot_trajectory.py

test: $(MATRIX_TEST_TARGET) $(INTEGRATION_TEST_TARGET)

run-tests: test
	@echo "--- Rodando Testes de Matriz ---"
	./$(MATRIX_TEST_TARGET)
	@echo "\n--- Rodando Testes de Integracao ---"
	./$(INTEGRATION_TEST_TARGET)

$(APP_TARGET): $(APP_MAIN_OBJ) $(LIB_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "--- Executando a simulação...---"
	./$(APP_TARGET)

$(MATRIX_TEST_TARGET): $(MATRIX_TEST_OBJ) $(OBJ_DIR)/matrixOperations.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(INTEGRATION_TEST_TARGET): $(INTEGRATION_TEST_OBJ) $(OBJ_DIR)/integration.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR) simulation_output.txt *.png