import pandas as pd
import matplotlib.pyplot as plt
import os
import platform
import subprocess
from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Image
from reportlab.lib.styles import getSampleStyleSheet

# Caminho absoluto da pasta onde este script está
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_DIR = os.path.join(BASE_DIR, "..", "output")

def plot_robot_trajectory():
    """
    Lê os dados da simulação e plota a trajetória 2D do robô e a referência.
    Retorna o caminho da imagem gerada.
    """
    try:
        df = pd.read_csv(os.path.join(OUTPUT_DIR, "simulation_output.txt"), sep='\t')

        required_cols = ['x', 'y', 'xref', 'yref']
        if not all(col in df.columns for col in required_cols):
            print(f"Erro: O arquivo de saída não contém as colunas necessárias: {required_cols}")
            return None

        plt.figure(figsize=(10, 8))

        # Trajetória do robô e da referência
        plt.plot(df['x'], df['y'], marker='.', linestyle='-', label='Trajetória do Robô (y(t))')
        plt.plot(df['xref'], df['yref'], linestyle='--', color='gray', label='Referência (ref)')
        plt.scatter(df['x'].iloc[0], df['y'].iloc[0], color='green', s=100, zorder=5, label='Início')
        plt.scatter(df['x'].iloc[-1], df['y'].iloc[-1], color='red', s=100, zorder=5, label='Fim')

        plt.title('Trajetória do Robô vs. Referência (Visão Superior)')
        plt.xlabel('Posição X (m)')
        plt.ylabel('Posição Y (m)')
        plt.grid(True)
        plt.axhline(0, color='grey', lw=0.5)
        plt.axvline(0, color='grey', lw=0.5)
        plt.gca().set_aspect('equal', adjustable='box')
        plt.legend()

        image_path = os.path.join(OUTPUT_DIR, "trajetoria_robo_lab3.png")
        plt.savefig(image_path, bbox_inches='tight')
        plt.close()

        print(f"✅ Gráfico salvo em: {image_path}")
        return image_path

    except FileNotFoundError:
        print("Erro: Arquivo 'simulation_output.txt' não encontrado. Execute a simulação primeiro.")
        return None
    except Exception as e:
        print(f"Ocorreu um erro inesperado ao gerar o gráfico: {e}")
        return None


def gerar_pdf(image_path):
    """
    Gera um relatório PDF contendo o gráfico da trajetória.
    """
    styles = getSampleStyleSheet()
    pdf_path = os.path.join(OUTPUT_DIR, "relatorio_trajetoria_robo.pdf")

    doc = SimpleDocTemplate(pdf_path, pagesize=A4)
    elementos = []

    elementos.append(Paragraph("<b>Relatório da Trajetória do Robô</b>", styles["Title"]))
    elementos.append(Spacer(1, 20))
    elementos.append(Paragraph(
        "Trajetória 2D obtida na simulação em comparação com a trajetória de referência.",
        styles["BodyText"]
    ))
    elementos.append(Spacer(1, 20))

    if image_path and os.path.exists(image_path):
        elementos.append(Image(image_path, width=480, height=400))
    else:
        elementos.append(Paragraph("❌ Imagem da trajetória não disponível.", styles["Normal"]))

    doc.build(elementos)

    print(f"📄 Relatório PDF gerado em: {pdf_path}")
    return pdf_path


def abrir_pdf(pdf_path):
    """
    Abre o PDF automaticamente no sistema operacional.
    """
    try:
        system_name = platform.system()
        if system_name == "Linux":
            subprocess.run(["xdg-open", pdf_path])
        elif system_name == "Darwin":  # macOS
            subprocess.run(["open", pdf_path])
        elif system_name == "Windows":
            os.startfile(pdf_path)
    except Exception as e:
        print(f"Não foi possível abrir o arquivo automaticamente: {e}")


if __name__ == "__main__":
    image_path = plot_robot_trajectory()
    if image_path:
        pdf_path = gerar_pdf(image_path)
        abrir_pdf(pdf_path)
