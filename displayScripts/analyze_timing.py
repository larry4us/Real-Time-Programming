import pandas as pd
import numpy as np

def analyze(filename, nominal_period):
    """Calcula as estatísticas para um dado arquivo de tempos."""
    try:
        df = pd.read_csv(filename)
        # O primeiro valor de T(k) pode ser 0.0, então vamos ignorá-lo se for o caso.
        T_k = df['T(k)'][df['T(k)'] > 0]
        
        # Jitter J(k) = T(k) - T_nominal
        J_k = T_k - nominal_period
        
        stats = {
            'T(k)': {
                'Média': T_k.mean(),
                'Variância': T_k.var(),
                'Desvio Padrão': T_k.std(),
                'Mínimo': T_k.min(),
                'Máximo': T_k.max(),
            },
            'J(k)': {
                'Média': J_k.mean(),
                'Variância': J_k.var(),
                'Desvio Padrão': J_k.std(),
                'Mínimo': J_k.min(),
                'Máximo': J_k.max(),
            }
        }
        return stats
    except FileNotFoundError:
        print(f"Arquivo de análise '{filename}' não encontrado.")
        return None

def print_table(stats, title):
    """Imprime a tabela de estatísticas de forma formatada."""
    print(f"\n--- {title} ---")
    print("-" * 65)
    print(f"{'Métrica':<15} | {'Média':>10} | {'Variância':>10} | {'Desv. Padrão':>12} | {'Mín/Máx':>15}")
    print("-" * 65)
    
    for metric, values in stats.items():
        min_max_str = f"{values['Mínimo']:.2f} / {values['Máximo']:.2f}"
        print(f"{metric:<15} | {values['Média']:>10.2f} | {values['Variância']:>10.2f} | {values['Desvio Padrão']:>12.2f} | {min_max_str:>15}")
    print("-" * 65)


if __name__ == '__main__':
    # Analisa os dados da Thread 1 (Período Nominal: 30ms)
    stats1 = analyze('output/thread1_timing.txt', 30.0)
    if stats1:
        print_table(stats1, "Análise da Thread 1 (30ms)")

    # Analisa os dados da Thread 2 (Período Nominal: 50ms)
    stats2 = analyze('output/thread2_timing.txt', 50.0)
    if stats2:
        print_table(stats2, "Análise da Thread 2 (50ms)")