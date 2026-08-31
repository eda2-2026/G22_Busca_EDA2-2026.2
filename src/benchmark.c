#include "homolog.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Benchmark do experimento (baseline x Fibonacci).
 *
 * Mede colisoes e maior bucket de duas funcoes de hash:
 *   BASELINE  : numero bruto concatenado, h(k) = k mod m (hash_mod);
 *   FIBONACCI : chave estruturada + multiplicative-shift (hash_fib);
 * em dois cenarios:
 *   real       : registro real da Anatel, ja deduplicado;
 *   adversario : dados agrupados de proposito (poucos anos/fabricantes,
 *                sequencial denso) — o pior caso da funcao ingenua.
 *
 * Ambos rodam na mesma capacidade de tabela (potencia de 2, fator de
 * carga ~0,7). Escreve resultados/colisoes.csv, lido por
 * gerar_graficos.py e pelo painel.
 *
 * Uso: benchmark [registro_real.csv]
 */

static uint64_t chave_bruta(Homolog h) {
    return (uint64_t)h.seq * 10000000ULL + (uint64_t)h.ano * 100000ULL + (uint64_t)h.fab;
}
static size_t pot2(size_t x){ size_t p=1; while(p<x) p<<=1; return p; }
static uint32_t log2u(size_t m){ uint32_t b=0; while(((size_t)1<<b)<m) b++; return b; }

static void medir(const Homolog *v, size_t n, int fib,
                  size_t *m_out, size_t *col_out, size_t *maior_out) {
    size_t m = pot2(n*10/7+1); if (m<16) m=16;
    uint32_t mb = log2u(m);
    size_t *cont = calloc(m, sizeof *cont);
    if (!cont) { *m_out=m; *col_out=0; *maior_out=0; return; }
    for (size_t i=0;i<n;i++) {
        size_t idx = fib ? (size_t)hash_fib(chave_estruturada(v[i]), mb)
                         : (size_t)hash_mod(chave_bruta(v[i]), m);
        cont[idx]++;
    }
    size_t ocup=0, maior=0;
    for (size_t i=0;i<m;i++) if (cont[i]) { ocup++; if (cont[i]>maior) maior=cont[i]; }
    free(cont);
    *m_out=m; *col_out=n-ocup; *maior_out=maior;
}

static size_t carregar(const char *path, Homolog *v, size_t max) {
    FILE *f=fopen(path,"r"); if(!f){perror(path);return 0;}
    char linha[256]; size_t n=0;
    if(fgets(linha,sizeof linha,f)){}
    while(n<max && fgets(linha,sizeof linha,f)){
        linha[strcspn(linha,"\r\n")]='\0';
        if(linha[0]=='\0') continue;
        Homolog h; if(homolog_parse(linha,&h)) v[n++]=h;
    }
    fclose(f); return n;
}

static void gerar_adversario(Homolog *v, size_t n) {
    for(size_t i=0;i<n;i++){
        v[i].seq=(uint32_t)(i % 100000);
        v[i].ano=(uint16_t)(i % 10);
        v[i].fab=(uint32_t)((i % 20) * 137);
    }
}

int main(int argc, char **argv) {
    const char *reg = (argc>=2) ? argv[1] : "dados/registro_real.csv";
    const size_t CAP = 300000;
    Homolog *real = malloc(CAP*sizeof *real);
    Homolog *sub  = malloc(CAP*sizeof *sub);
    Homolog *adv  = malloc(CAP*sizeof *adv);
    if(!real||!sub||!adv){ fprintf(stderr,"benchmark: sem memoria\n"); return 1; }

    size_t nreal = carregar(reg, real, CAP);
    if (nreal==0) { fprintf(stderr,"benchmark: nao carregou %s\n", reg); return 1; }

    size_t tamanhos[] = {10000, 50000, nreal};
    FILE *out = fopen("resultados/colisoes.csv","w");
    if(!out){ perror("resultados/colisoes.csv"); return 1; }
    fprintf(out,"funcao,dataset,n,m,fator_carga,colisoes,maior_bucket\n");

    for (size_t ti=0; ti<sizeof(tamanhos)/sizeof(tamanhos[0]); ti++) {
        size_t n = tamanhos[ti];
        if (n>nreal) n=nreal;
        /* real: amostra espalhada (o registro esta ordenado) */
        for (size_t i=0;i<n;i++) sub[i] = real[(size_t)((double)i*nreal/n)];
        gerar_adversario(adv, n);
        for (int fib=0; fib<2; fib++) {
            const char *fn = fib ? "fibonacci" : "baseline";
            size_t m,col,mai;
            medir(sub, n, fib, &m,&col,&mai);
            fprintf(out,"%s,real,%zu,%zu,%.3f,%zu,%zu\n", fn,n,m,(double)n/m,col,mai);
            medir(adv, n, fib, &m,&col,&mai);
            fprintf(out,"%s,adversario,%zu,%zu,%.3f,%zu,%zu\n", fn,n,m,(double)n/m,col,mai);
        }
    }
    fclose(out); free(real); free(sub); free(adv);
    printf("OK: resultados/colisoes.csv gerado (baseline x fibonacci; real x adversario)\n");
    return 0;
}
