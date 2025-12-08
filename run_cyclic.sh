#!/bin/bash

# ===========================================================
# Script para medir latência com cyclictest no modelo:
#   - sem carga
#   - com carga (stress-ng)
# ===========================================================

RESULT_DIR="cyclictest_results"
mkdir -p "$RESULT_DIR"

CYCLIC_PARAMS="-Sp90 -i100 -h400 -l500000"

timestamp=$(date +"%Y%m%d_%H%M%S")

# Função: teste sem carga
teste_sem_carga() {
    echo "▶ Executando CYCLICTEST SEM CARGA..."
    sudo cyclictest $CYCLIC_PARAMS > "$RESULT_DIR/sem_carga_$timestamp.txt"
    echo "✔ Resultado salvo em: $RESULT_DIR/sem_carga_$timestamp.txt"
}

# Função: teste com carga
teste_com_carga() {
    echo "▶ Executando CYCLICTEST COM CARGA..."
    
    # Inicia carga paralela
    stress-ng --cpu 8 --vm 4 --vm-bytes 50% --timeout 20s &
    STRESS_PID=$!
    
    sudo cyclictest $CYCLIC_PARAMS > "$RESULT_DIR/com_carga_$timestamp.txt"
    
    wait $STRESS_PID
    echo "✔ Resultado salvo em: $RESULT_DIR/com_carga_$timestamp.txt"
}

# ------------------------ PARÂMETROS ------------------------

case "$1" in
    sem_carga)
        teste_sem_carga
        ;;
    com_carga)
        teste_com_carga
        ;;
    *)
        echo "Uso correto:"
        echo "  ./run_test.sh sem_carga"
        echo "  ./run_test.sh com_carga"
        ;;
esac
