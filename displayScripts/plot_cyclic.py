import os
import re
import matplotlib.pyplot as plt
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Image
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import getSampleStyleSheet

RESULT_DIR = "cyclictest_results"

number_regex = re.compile(r"^-?\d+(\.\d+)?$")

# ---------------------------------------------------------
# PARSER DE ESTATÍSTICA
# ---------------------------------------------------------
def parse_stats(filepath):
    mins, avgs, maxs = [], [], []

    with open(filepath, "r") as f:
        for line in f:
            if "Min:" in line and "Avg:" in line and "Max:" in line:
                tokens = line.split()
                i = 0
                while i < len(tokens):
                    tok = tokens[i]

                    if tok.startswith("Min:"):
                        j = i + 1
                        while j < len(tokens) and not number_regex.match(tokens[j]):
                            j += 1
                        if j < len(tokens):
                            mins.append(float(tokens[j]))

                    if tok.startswith("Avg:"):
                        j = i + 1
                        while j < len(tokens) and not number_regex.match(tokens[j]):
                            j += 1
                        if j < len(tokens):
                            avgs.append(float(tokens[j]))

                    if tok.startswith("Max:"):
                        j = i + 1
                        while j < len(tokens) and not number_regex.match(tokens[j]):
                            j += 1
                        if j < len(tokens):
                            maxs.append(float(tokens[j]))
                    i += 1

    return {
        "min": min(mins),
        "avg": sum(avgs) / len(avgs),
        "max": max(maxs)
    }

# ---------------------------------------------------------
# PARSER TEMPORAL (Act)
# ---------------------------------------------------------
def parse_time_series(filepath):
    series = []

    with open(filepath, "r") as f:
        for line in f:
            if "Act:" in line:
                tokens = line.split()
                for i, tok in enumerate(tokens):
                    if tok.startswith("Act:"):
                        j = i + 1
                        while j < len(tokens) and not number_regex.match(tokens[j]):
                            j += 1
                        if j < len(tokens):
                            series.append(float(tokens[j]))
    return series

# ---------------------------------------------------------
# GRÁFICO 1 — CURVA TEMPORAL INDIVIDUAL
# ---------------------------------------------------------
def plot_single_series(series, title, filename):
    plt.figure(figsize=(12, 6))
    plt.plot(series, alpha=0.9)
    plt.title(title)
    plt.xlabel("Amostra")
    plt.ylabel("Latência (us)")
    plt.grid(True)

    out_path = os.path.join(RESULT_DIR, filename)
    plt.tight_layout()
    plt.savefig(out_path, dpi=200)
    plt.close()
    return out_path

# ---------------------------------------------------------
# GRÁFICO 2 — BARRAS DE MIN/AVG/MAX
# ---------------------------------------------------------
def plot_bar_comparison(sem_stats, com_stats):
    labels = ["Min", "Avg", "Max"]
    sem_vals = [sem_stats["min"], sem_stats["avg"], sem_stats["max"]]
    com_vals = [com_stats["min"], com_stats["avg"], com_stats["max"]]

    x = range(len(labels))
    width = 0.35

    plt.figure(figsize=(10, 6))
    plt.bar([p - width/2 for p in x], sem_vals, width, label="Sem carga")
    plt.bar([p + width/2 for p in x], com_vals, width, label="Com carga")

    plt.xticks(x, labels)
    plt.ylabel("Latência (us)")
    plt.title("Comparação de Min / Avg / Max")
    plt.legend()
    plt.grid(axis="y", linestyle="--", alpha=0.5)

    out_path = os.path.join(RESULT_DIR, "cyclictest_bar_stats.png")
    plt.tight_layout()
    plt.savefig(out_path, dpi=200)
    plt.close()

    return out_path

# ---------------------------------------------------------
# GERAR PDF
# ---------------------------------------------------------
def generate_pdf(sem_stats, com_stats, p_sem, p_com, p_bar):
    pdf_path = os.path.join(RESULT_DIR, "cyclictest_report.pdf")
    styles = getSampleStyleSheet()
    doc = SimpleDocTemplate(pdf_path, pagesize=A4)
    elems = []

    elems.append(Paragraph("<b>Relatório de Latência - Cyclictest</b>", styles["Title"]))
    elems.append(Spacer(1, 20))

    elems.append(Paragraph("<b>Resultados:</b>", styles["Heading2"]))
    elems.append(Paragraph(
        f"<b>Sem carga:</b> Min={sem_stats['min']} us | "
        f"Avg={sem_stats['avg']:.2f} us | Max={sem_stats['max']} us",
        styles["Normal"]
    ))
    elems.append(Paragraph(
        f"<b>Com carga:</b> Min={com_stats['min']} us | "
        f"Avg={com_stats['avg']:.2f} us | Max={com_stats['max']} us",
        styles["Normal"]
    ))
    elems.append(Spacer(1, 20))

    elems.append(Paragraph("<b>Curva temporal — Sem carga</b>", styles["Heading2"]))
    elems.append(Image(p_sem, width=400, height=250))
    elems.append(Spacer(1, 20))

    elems.append(Paragraph("<b>Curva temporal — Com carga</b>", styles["Heading2"]))
    elems.append(Image(p_com, width=400, height=250))
    elems.append(Spacer(1, 20))

    elems.append(Paragraph("<b>Comparação Min / Avg / Max</b>", styles["Heading2"]))
    elems.append(Image(p_bar, width=400, height=250))

    doc.build(elems)
    print(f"PDF gerado em: {pdf_path}")

# ---------------------------------------------------------
# MAIN
# ---------------------------------------------------------
if __name__ == "__main__":
    files = sorted([f for f in os.listdir(RESULT_DIR) if f.endswith(".txt")])

    if not files:
        print("Nenhum arquivo encontrado.")
        exit()

    sem_file = next(f for f in files if "sem" in f)
    com_file = next(f for f in files if "com" in f)

    sem_stats = parse_stats(os.path.join(RESULT_DIR, sem_file))
    com_stats = parse_stats(os.path.join(RESULT_DIR, com_file))

    sem_series = parse_time_series(os.path.join(RESULT_DIR, sem_file))
    com_series = parse_time_series(os.path.join(RESULT_DIR, com_file))

    p_sem = plot_single_series(sem_series, "Latência ao longo do tempo — Sem carga", "sem_carga_time_series.png")
    p_com = plot_single_series(com_series, "Latência ao longo do tempo — Com carga", "com_carga_time_series.png")
    p_bar = plot_bar_comparison(sem_stats, com_stats)

    generate_pdf(sem_stats, com_stats, p_sem, p_com, p_bar)

    print("✔ Análises concluídas com sucesso!")
