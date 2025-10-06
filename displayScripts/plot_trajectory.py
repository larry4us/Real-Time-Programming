import pandas as pd
import matplotlib.pyplot as plt

def plot_robot_trajectory():
    """
    Lê os dados da simulação e plota a trajetória 2D do robô e a referência.
    """
    try:
        df = pd.read_csv('output/simulation_output.txt', sep='\t')

        required_cols = ['x', 'y', 'xref', 'yref']
        if not all(col in df.columns for col in required_cols):
            print(f"Erro: O arquivo de saída não contém as colunas necessárias: {required_cols}")
            return

        plt.figure(figsize=(10, 8))

        # Plota a trajetória real do robô
        plt.plot(df['x'], df['y'], marker='.', linestyle='-', label='Trajetória do Robô (y(t))')
        
        # Plota a trajetória de referência
        plt.plot(df['xref'], df['yref'], linestyle='--', color='gray', label='Referência (ref)')

        plt.scatter(df['x'].iloc[0], df['y'].iloc[0], color='green', s=100, zorder=5, label='Início')
        plt.scatter(df['x'].iloc[-1], df['y'].iloc[-1], color='red', s=100, zorder=5, label='Fim')

        plt.title('Trajetória do Robô vs. Referência (Visão Superior)')
        plt.xlabel('Posição X (metros)')
        plt.ylabel('Posição Y (metros)')
        plt.grid(True)
        plt.axhline(0, color='grey', lw=0.5)
        plt.axvline(0, color='grey', lw=0.5)
        plt.gca().set_aspect('equal', adjustable='box')
        plt.legend()

        plt.savefig('output/trajetoria_robo_lab3.png')
        print("Gráfico da trajetória salvo com sucesso como 'trajetoria_robo_lab3.png'")

    except FileNotFoundError:
        print("Erro: Arquivo 'simulation_output.txt' não encontrado. Execute a simulação primeiro.")
    except Exception as e:
        print(f"Ocorreu um erro inesperado ao gerar o gráfico: {e}")

if __name__ == '__main__':
    plot_robot_trajectory()