import pandas as pd
import numpy as np

def analyze(filename, nominal_period):
    """Calcula as estatísticas para um dado arquivo de tempos."""
    try:
        df = pd.read_csv(filename)
        if df.empty or 'T(k)' not in df.columns:
            print(f"Arquivo '{filename}' está vazio ou não tem a coluna 'T(k)'.")
            return None
            
        T_k = df['T(k)'][df['T(k)'] > 1] # Ignora o primeiro valor que costuma ser pequeno
        
        if T_k.empty:
            print(f"Não há dados válidos de T(k) em '{filename}'.")
            return None

        J_k = T_k - nominal_period
        
        stats = {
            'T(k)': {
                'Média': T_k.mean(), 'Variância': T_k.var(),
                'Desvio Padrão': T_k.std(), 'Mínimo': T_k.min(), 'Máximo': T_k.max(),
            },
            'J(k)': {
                'Média': J_k.mean(), 'Variância': J_k.var(),
                'Desvio Padrão': J_k.std(), 'Mínimo': J_k.min(), 'Máximo': J_k.max(),
            }
        }
        return stats
    except FileNotFoundError:
        print(f"Arquivo de análise '{filename}' não encontrado.")
        return None

def print_table(stats, title):
    """Imprime a tabela de estatísticas de forma formatada."""
    print(f"\n--- {title} ---")
    print("-" * 75)
    print(f"{'Métrica':<15} | {'Média (ms)':>12} | {'Variância':>12} | {'Desv. Padrão':>15} | {'Mín/Máx (ms)':>18}")
    print("-" * 75)
    
    for metric, values in stats.items():
        min_max_str = f"{values['Mínimo']:.2f} / {values['Máximo']:.2f}"
        print(f"{metric:<15} | {values['Média']:>12.2f} | {values['Variância']:>12.2f} | {values['Desvio Padrão']:>15.2f} | {min_max_str:>18}")
    print("-" * 75)

if __name__ == '__main__':
    # Dicionário com os arquivos de timing e seus períodos nominais
    timing_files = {
        "Análise da Geração de Referência (120ms)": ('output/ref_gen_timing.txt', 120.0),
        "Análise do Modelo de Referência X (50ms)": ('output/ref_model_x_timing.txt', 50.0),
        "Análise do Modelo de Referência Y (50ms)": ('output/ref_model_y_timing.txt', 50.0),
        "Análise do Controle (50ms)": ('output/control_timing.txt', 50.0),
        "Análise da Linearização (40ms)": ('output/linearization_timing.txt', 40.0),
        "Análise da Simulação do Robô (30ms)": ('output/robot_sim_timing.txt', 30.0),
        "Análise do Logger (100ms)": ('output/logger_timing.txt', 100.0)
    }

    for title, (filename, period) in timing_files.items():
        stats = analyze(filename, period)
        if stats:
            print_table(stats, title)