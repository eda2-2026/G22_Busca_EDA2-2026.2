#!/usr/bin/env python3
"""
Gera os graficos do experimento a partir de resultados/colisoes.csv.

Requer: matplotlib  (pip install matplotlib)
Uso:    python resultados/gerar_graficos.py
Saida:  resultados/graficos/baseline_real_vs_uniforme.png

O CSV tem o esquema:
    funcao,dataset,n,m,fator_carga,colisoes,maior_bucket
Hoje so tem linhas 'baseline'. Quando o benchmark do hash de Fibonacci
rodar, basta acrescentar linhas com funcao='fibonacci' que este script
ganha as series novas sem alteracao.
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
sizes  = sorted({int(r["n"]) for r in rows})
def rotulo(n): return f"{n//1000} mil" + (" (full)" if n == max(sizes) else "")
labels = [rotulo(n) for n in sizes]

def serie(dataset, campo, funcao="baseline"):
    d = {int(r["n"]): int(r[campo]) for r in rows
         if r["dataset"] == dataset and r["funcao"] == funcao}
    return [d.get(n, 0) for n in sizes]

AZUL="#2a78d6"; LARANJA="#eb6834"; INK="#0b0b0b"; INK2="#52514e"; GRID="#e6e6e3"; SURF="#fcfcfb"
plt.rcParams.update({"font.size":11,"font.family":"DejaVu Sans","text.color":INK,
                     "axes.edgecolor":INK2,"axes.labelcolor":INK2,"xtick.color":INK2,"ytick.color":INK2})

fig,(ax1,ax2)=plt.subplots(1,2,figsize=(11,5.2),facecolor=SURF)
x=np.arange(len(sizes)); w=0.38
def painel(ax,real,unif,titulo,ylab):
    ax.set_facecolor(SURF)
    b1=ax.bar(x-w/2-0.012,real,w,label="Real (Anatel)",color=AZUL)
    b2=ax.bar(x+w/2+0.012,unif,w,label="Sintetico uniforme",color=LARANJA)
    ax.bar_label(b1,padding=2,fontsize=9,color=INK,fmt="%d")
    ax.bar_label(b2,padding=2,fontsize=9,color=INK,fmt="%d")
    ax.set_title(titulo,fontsize=12,color=INK,pad=8,fontweight="bold")
    ax.set_ylabel(ylab,fontsize=10); ax.set_xticks(x); ax.set_xticklabels(labels)
    ax.set_ylim(0,max(real)*1.18 if max(real)>0 else 1)
    ax.spines[["top","right"]].set_visible(False); ax.spines[["left","bottom"]].set_color(GRID)
    ax.yaxis.grid(True,color=GRID,linewidth=0.8); ax.set_axisbelow(True); ax.tick_params(length=0)

painel(ax1,serie("real","colisoes"),serie("uniforme","colisoes"),"Colisoes","no de colisoes")
painel(ax2,serie("real","maior_bucket"),serie("uniforme","maior_bucket"),
       "Maior bucket (pior caso da busca)","tamanho do maior bucket")
fig.suptitle("Hash baseline (k mod m): dado real da Anatel x sintetico uniforme",
             fontsize=13.5,fontweight="bold",color=INK,y=0.985)
h,l=ax1.get_legend_handles_labels()
fig.legend(h,l,loc="upper center",ncol=2,frameon=False,fontsize=10.5,bbox_to_anchor=(0.5,0.90))
fig.text(0.5,0.02,"Mesma tabela e mesmo fator de carga (~0,36); so muda a natureza do dado.  -  G22 - EDA2 2026.2",
         ha="center",fontsize=8.5,color=INK2)
fig.subplots_adjust(left=0.075,right=0.98,top=0.78,bottom=0.12,wspace=0.22)
saida=os.path.join(OUT,"baseline_real_vs_uniforme.png")
fig.savefig(saida,dpi=150,facecolor=SURF)
print("gerado:", saida)
