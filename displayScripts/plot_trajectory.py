import pandas as pd
import matplotlib.pyplot as plt

def plot_robot_trajectory():
    """
    Lê os dados da simulação e plota a trajetória 2D do robô.
    """
    try:
        # Lê o arquivo de dados. O separador '\t' indica que as colunas são separadas por tabs.
        df = pd.read_csv('output/simulation_output.txt', sep='\t')

        # Verifica se o DataFrame tem as colunas necessárias
        if 'x' not in df.columns or 'y' not in df.columns:
            print("Erro: O arquivo 'simulation_output.txt' não contém as colunas 'x' e 'y'.")
            return

        # Cria a figura para o gráfico
        plt.figure(figsize=(10, 8))

        # Plota a trajetória (y no eixo vertical, x no eixo horizontal)
        plt.plot(df['x'], df['y'], marker='.', linestyle='-', label='Trajetória do Robô')

        # Adiciona marcadores para o início e o fim da trajetória
        plt.scatter(df['x'].iloc[0], df['y'].iloc[0], color='green', s=100, zorder=5, label='Início')
        plt.scatter(df['x'].iloc[-1], df['y'].iloc[-1], color='red', s=100, zorder=5, label='Fim')

        # Configurações do gráfico para melhor visualização
        plt.title('Trajetória do Robô (Visão Superior)')
        plt.xlabel('Posição X (metros)')
        plt.ylabel('Posição Y (metros)')
        plt.grid(True)
        plt.axhline(0, color='grey', lw=0.5)
        plt.axvline(0, color='grey', lw=0.5)
        plt.gca().set_aspect('equal', adjustable='box') # Garante que as escalas de X e Y sejam iguais
        plt.legend()

        # Salva o gráfico em um arquivo de imagem
        plt.savefig('output/trajetoria_robo.png')
        print("Gráfico da trajetória salvo com sucesso como 'trajetoria_robo.png'")

    except FileNotFoundError:
        print("Erro: Arquivo 'simulation_output.txt' não encontrado. Execute a simulação em C primeiro.")
    except Exception as e:
        print(f"Ocorreu um erro inesperado ao gerar o gráfico: {e}")

if __name__ == '__main__':
    plot_robot_trajectory()