import pandas as pd
import numpy as np
import os
import platform
import subprocess
from reportlab.lib.pagesizes import A4
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet

# Caminho absoluto da pasta onde este script está
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# Caminho da pasta "output" (um nível acima)
OUTPUT_DIR = os.path.join(BASE_DIR, "..", "output")

def analyze(filename, nominal_period):
    """Calcula as estatísticas para um dado arquivo de tempos."""
    try:
        df = pd.read_csv(filename)
        if df.empty or 'T(k)' not in df.columns:
            print(f"Arquivo '{filename}' está vazio ou não tem a coluna 'T(k)'.")
            return None
            
        T_k = df['T(k)'][df['T(k)'] > 1]  # Ignora o primeiro valor que costuma ser pequeno
        
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

def gerar_pdf(relatorio_dados, output_path):
    """Gera o PDF formatado com as tabelas de estatísticas."""
    styles = getSampleStyleSheet()
    doc = SimpleDocTemplate(output_path, pagesize=A4)
    elementos = []
    
    elementos.append(Paragraph("<b>Relatório de Análise de Tempos</b>", styles["Title"]))
    elementos.append(Spacer(1, 20))
    
    for title, stats in relatorio_dados:
        elementos.append(Paragraph(f"<b>{title}</b>", styles["Heading2"]))
        elementos.append(Spacer(1, 10))
        
        data = [["Métrica", "Média (ms)", "Variância", "Desv. Padrão", "Mín/Máx (ms)"]]
        for metric, values in stats.items():
            min_max_str = f"{values['Mínimo']:.2f} / {values['Máximo']:.2f}"
            data.append([
                metric,
                f"{values['Média']:.2f}",
                f"{values['Variância']:.2f}",
                f"{values['Desvio Padrão']:.2f}",
                min_max_str,
            ])
        
        tabela = Table(data, hAlign="LEFT")
        tabela.setStyle(TableStyle([
            ('BACKGROUND', (0, 0), (-1, 0), colors.lightgrey),
            ('GRID', (0, 0), (-1, -1), 0.5, colors.black),
            ('ALIGN', (1, 1), (-1, -1), 'CENTER'),
            ('FONTNAME', (0, 0), (-1, 0), 'Helvetica-Bold'),
        ]))
        
        elementos.append(tabela)
        elementos.append(Spacer(1, 20))
    
    doc.build(elementos)

if __name__ == '__main__':
    timing_files = {
        "Análise da Geração de Referência (120ms)": (
            os.path.join(OUTPUT_DIR, "ref_gen_timing.txt"), 120.0
        ),
        "Análise do Modelo de Referência X (50ms)": (
            os.path.join(OUTPUT_DIR, "ref_model_x_timing.txt"), 50.0
        ),
        "Análise do Modelo de Referência Y (50ms)": (
            os.path.join(OUTPUT_DIR, "ref_model_y_timing.txt"), 50.0
        ),
        "Análise do Controle (50ms)": (
            os.path.join(OUTPUT_DIR, "control_timing.txt"), 50.0
        ),
        "Análise da Linearização (40ms)": (
            os.path.join(OUTPUT_DIR, "linearization_timing.txt"), 40.0
        ),
        "Análise da Simulação do Robô (30ms)": (
            os.path.join(OUTPUT_DIR, "robot_sim_timing.txt"), 30.0
        ),
        "Análise do Logger (100ms)": (
            os.path.join(OUTPUT_DIR, "logger_timing.txt"), 100.0
        ),
    }

    relatorio = []
    for title, (filename, period) in timing_files.items():
        stats = analyze(filename, period)
        if stats:
            relatorio.append((title, stats))

    if not relatorio:
        print("Nenhum dado válido para gerar o relatório.")
    else:
        pdf_path = os.path.join(OUTPUT_DIR, "analise_timing_resultados.pdf")
        gerar_pdf(relatorio, pdf_path)
        print(f"\n✅ Relatório PDF gerado em: {pdf_path}")

        # Abre automaticamente o PDF no visualizador padrão
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
