#!/usr/bin/env python3
"""
Gera o grafico do experimento a partir de resultados/colisoes.csv.

Requer: matplotlib  (pip install matplotlib)
Uso:    python resultados/gerar_graficos.py
Saida:  resultados/graficos/baseline_vs_fibonacci.png

Compara BASELINE (k mod m) x FIBONACCI em dois cenarios (real x
adversario). No dado real os dois empatam; no adversario o Fibonacci
vence. O CSV tem o esquema:
    funcao,dataset,n,m,fator_carga,colisoes,maior_bucket
"""
import csv, os
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

AQUI = os.path.dirname(os.path.abspath(__file__))
CSV  = os.path.join(AQUI, "colisoes.csv")
OUT  = os.path.join(AQUI, "graficos")
os.makedirs(OUT, exist_ok=True)

rows = list(csv.DictReader(open(CSV, encoding="utf-8")))
sizes = sorted({int(r["n"]) for r in rows})
def lbl(n): return f"{n//1000} mil" + (" (full)" if n == max(sizes) else "")
labels = [lbl(n) for n in sizes]

def serie(dataset, funcao, campo="colisoes"):
    d = {int(r["n"]): int(r[campo]) for r in rows
         if r["dataset"] == dataset and r["funcao"] == funcao}
    return [d.get(n, 0) for n in sizes]

AZUL="#2a78d6"; LARANJA="#eb6834"; INK="#0b0b0b"; INK2="#52514e"; GRID="#e6e6e3"; SURF="#fcfcfb"
plt.rcParams.update({"font.size":11,"font.family":"DejaVu Sans","text.color":INK,
                     "axes.edgecolor":INK2,"axes.labelcolor":INK2,"xtick.color":INK2,"ytick.color":INK2})

fig,(ax1,ax2)=plt.subplots(1,2,figsize=(11,5.2),facecolor=SURF)
x=np.arange(len(sizes)); w=0.38
def painel(ax,base,fib,titulo):
    ax.set_facecolor(SURF)
    b1=ax.bar(x-w/2-0.012,base,w,label="baseline (k mod m)",color=LARANJA)
    b2=ax.bar(x+w/2+0.012,fib,w,label="Fibonacci",color=AZUL)
    ax.bar_label(b1,padding=2,fontsize=8.5,color=INK,fmt="%d")
    ax.bar_label(b2,padding=2,fontsize=8.5,color=INK,fmt="%d")
    ax.set_title(titulo,fontsize=12,color=INK,pad=8,fontweight="bold")
    ax.set_ylabel("nº de colisões",fontsize=10)
    ax.set_xticks(x); ax.set_xticklabels(labels)
    top=max(max(base),max(fib)); ax.set_ylim(0, top*1.18 if top>0 else 1)
    ax.spines[["top","right"]].set_visible(False); ax.spines[["left","bottom"]].set_color(GRID)
    ax.yaxis.grid(True,color=GRID,linewidth=0.8); ax.set_axisbelow(True); ax.tick_params(length=0)

painel(ax1, serie("real","baseline"), serie("real","fibonacci"),
       "Dado real da Anatel — empatam")
painel(ax2, serie("adversario","baseline"), serie("adversario","fibonacci"),
       "Cenário adversário — Fibonacci vence")

fig.suptitle("Colisões: baseline (k mod m) × Fibonacci estruturado",
             fontsize=13.5,fontweight="bold",color=INK,y=0.985)
h,l=ax1.get_legend_handles_labels()
fig.legend(h,l,loc="upper center",ncol=2,frameon=False,fontsize=10.5,bbox_to_anchor=(0.5,0.90))
fig.text(0.5,0.02,"Mesma tabela (potência de 2, fator de carga ~0,7). No dado real limpo o baseline já espalha bem; "
                  "o ganho do Fibonacci aparece no pior caso.  ·  G22 · EDA2 2026.2",
         ha="center",fontsize=8,color=INK2)
fig.subplots_adjust(left=0.08,right=0.98,top=0.78,bottom=0.13,wspace=0.24)
fig.savefig(os.path.join(OUT,"baseline_vs_fibonacci.png"),dpi=150,facecolor=SURF)
print("gerado:", os.path.join(OUT,"baseline_vs_fibonacci.png"))
