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
CYCLIC_DIR = cyclictest_results

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
PLOT_CYCLIC = $(DISPLAY_SCRIPT_DIR)/plot_cyclic.py
RUN_CYCLIC = $(shell pwd)/run_cyclic.sh

.PHONY: all run plot test run-tests clean cyclic_sem cyclic_com plot_cyclic full_test

all: run

run: $(APP_TARGET)
	@echo ">>> Executando simulação RTOS..."
	sudo ./$(APP_TARGET)

plot:
	@echo ">>> Gerando gráficos a partir dos logs existentes..."
	$(PYTHON) $(PLOT_TRAJECTORY)
	$(PYTHON) $(ANALYZE_TIMING)
	$(PYTHON) $(PLOT_CYCLIC)

cyclic_sem:
	@mkdir -p $(CYCLIC_DIR)
	@echo ">>> Executando cyclictest SEM carga..."
	$(RUN_CYCLIC) sem_carga

cyclic_com:
	@mkdir -p $(CYCLIC_DIR)
	@echo ">>> Executando cyclictest COM carga..."
	$(RUN_CYCLIC) com_carga

plot_cyclic:
	$(PYTHON) $(PLOT_CYCLIC)

# ⭐ NOVO: fluxograma completo exigido no trabalho
full_test: $(APP_TARGET)
	@mkdir -p $(CYCLIC_DIR)

	@echo "\n==============================="
	@echo ">>> 1) Rodando programa SEM carga + cyclictest"
	@echo "==============================="

	# programa sem carga paralelizado
	sudo ./$(APP_TARGET) > output/prog_sem_carga.txt & \
	PROG_PID=$$!; \
	$(RUN_CYCLIC) sem_carga; \
	wait $$PROG_PID

	@echo "\n==============================="
	@echo ">>> 2) Rodando programa COM carga + cyclictest"
	@echo "==============================="

	# inicia carga externa
	stress-ng --cpu 8 --vm 4 --vm-bytes 50% --timeout 20s & \
	STRESS_PID=$$!

	# programa com carga paralelizado
	sudo ./$(APP_TARGET) > output/prog_com_carga.txt & \
	PROG_PID=$$!; \
	$(RUN_CYCLIC) com_carga; \
	wait $$PROG_PID; \
	wait $$STRESS_PID

	@echo "\n==============================="
	@echo ">>> 3) Gerando todos os gráficos e relatórios"
	@echo "==============================="

	$(PYTHON) $(PLOT_TRAJECTORY)
	$(PYTHON) $(ANALYZE_TIMING)
	$(PYTHON) $(PLOT_CYCLIC)

	@echo "\n>>> Teste completo finalizado!"

test: $(MATRIX_TEST_TARGET) $(INTEGRATION_TEST_TARGET)

run-tests: test
	@echo ">>> Rodando Testes..."
	./$(MATRIX_TEST_TARGET)
	./$(INTEGRATION_TEST_TARGET)

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
	rm -rf $(BIN_DIR) $(OBJ_DIR) $(OUTPUT_DIR) $(CYCLIC_DIR) *.txt *.png
	@echo ">>> Limpeza concluída."