#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Painel de resultados no terminal.
 *
 * Le resultados/colisoes.csv e imprime um resumo formatado (cabecalho,
 * tabela e uma barra comparando o maior bucket real x uniforme), com
 * cores ANSI. Rode:  make painel   (ou ./bin/painel [arquivo.csv]).
 * Defina NO_COLOR=1 no ambiente para sair sem cores.
 */

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
static void preparar_terminal(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    if (GetConsoleMode(h, &modo))
        SetConsoleMode(h, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(65001); /* UTF-8, pra desenhar as caixas e barras */
}
#else
static void preparar_terminal(void) {}
#endif

static const char *RESET="",*BOLD="",*DIM="",*VERM="",*VERD="",*CIANO="",*AMAR="";
static void iniciar_cores(void) {
    if (getenv("NO_COLOR") != NULL) return;
    RESET="\033[0m"; BOLD="\033[1m"; DIM="\033[2m";
    VERM="\033[91m"; VERD="\033[92m"; CIANO="\033[96m"; AMAR="\033[93m";
}

static void repete(const char *s, int n) { for (int i=0;i<n;i++) fputs(s, stdout); }

/* insere '.' a cada 3 digitos: 109211 -> 109.211 */
static void fmt_milhar(long v, char *out) {
    char tmp[32]; snprintf(tmp,sizeof tmp,"%ld",v);
    int len=(int)strlen(tmp), j=0;
    for (int i=0;i<len;i++) {
        if (i>0 && (len-i)%3==0) out[j++]='.';
        out[j++]=tmp[i];
    }
    out[j]='\0';
}

#define MAXL 64
typedef struct { char funcao[24], dataset[24]; long n,m,col,maior; double fc; } Linha;

int main(int argc, char **argv) {
    preparar_terminal();
    iniciar_cores();

    const char *caminho = (argc>=2) ? argv[1] : "resultados/colisoes.csv";
    FILE *f = fopen(caminho,"r");
    if (!f) { fprintf(stderr,"painel: nao consegui abrir '%s'\n",caminho); return 1; }

    Linha L[MAXL]; int n=0; char linha[256];
    if (fgets(linha,sizeof linha,f)) { /* descarta cabecalho */ }
    while (n<MAXL && fgets(linha,sizeof linha,f)) {
        Linha *x=&L[n];
        if (sscanf(linha,"%23[^,],%23[^,],%ld,%ld,%lf,%ld,%ld",
                   x->funcao,x->dataset,&x->n,&x->m,&x->fc,&x->col,&x->maior)==7)
            n++;
    }
    fclose(f);
    if (n==0){ fprintf(stderr,"painel: nenhum dado em '%s'\n",caminho); return 1; }

    long nmax=0; for(int i=0;i<n;i++) if(L[i].n>nmax) nmax=L[i].n;

    /* ---------- cabecalho ---------- */
    printf("\n  %s%s",BOLD,CIANO); repete("═",62); printf("%s\n",RESET);
    printf("  %s%s  AUTENTICADOR DE HOMOLOGAÇÃO ANATEL   ·   G22 · EDA2 2026.2%s\n",BOLD,CIANO,RESET);
    printf("  %s  Experimento: hash baseline  (k mod m)%s\n",DIM,RESET);
    printf("  %s%s",BOLD,CIANO); repete("═",62); printf("%s\n\n",RESET);

    printf("  Colisões e maior bucket — dado %sREAL%s da Anatel  ×  sintético %sUNIFORME%s\n",VERM,RESET,VERD,RESET);
    printf("  %s(mesma tabela e fator de carga; só muda a natureza do dado)%s\n\n",DIM,RESET);

    /* ---------- tabela ---------- */
    printf("  ┌"); repete("─",16); printf("┬"); repete("─",12); printf("┬"); repete("─",14); printf("┬"); repete("─",14); printf("┐\n");
    printf("  │ %-14s │ %-10s │ %12s │ %12s │\n","tamanho","dataset","colisoes","maior bucket");
    printf("  ├"); repete("─",16); printf("┼"); repete("─",12); printf("┼"); repete("─",14); printf("┼"); repete("─",14); printf("┤\n");
    for (int i=0;i<n;i++) {
        const char *cor = (strcmp(L[i].dataset,"real")==0)? VERM : VERD;
        char c1[32],c2[32]; fmt_milhar(L[i].col,c1); fmt_milhar(L[i].maior,c2);
        char tam[32];
        if (L[i].n==nmax) snprintf(tam,sizeof tam,"%ld mil (full)",L[i].n/1000);
        else              snprintf(tam,sizeof tam,"%ld mil",L[i].n/1000);
        printf("  │ %-14s │ %s%-10s%s │ %12s │ %12s │\n",tam,cor,L[i].dataset,RESET,c1,c2);
    }
    printf("  └"); repete("─",16); printf("┴"); repete("─",12); printf("┴"); repete("─",14); printf("┴"); repete("─",14); printf("┘\n\n");

    /* ---------- barra: maior bucket no tamanho full ---------- */
    long real_full=0, unif_full=0;
    for (int i=0;i<n;i++) {
        if (L[i].n==nmax && strcmp(L[i].dataset,"real")==0)     real_full=L[i].maior;
        if (L[i].n==nmax && strcmp(L[i].dataset,"uniforme")==0) unif_full=L[i].maior;
    }
    long maxv = real_full>unif_full? real_full: unif_full;
    const int LARG=42;
    printf("  %sMaior bucket (pior caso da busca) — base completa%s\n",BOLD,RESET);
    int nb = maxv? (int)((double)real_full/maxv*LARG+0.5):0; if(real_full>0&&nb<1)nb=1;
    printf("    %-9s ","real"); printf("%s",VERM); repete("█",nb); printf("%s  %s%ld%s\n",RESET,BOLD,real_full,RESET);
    nb = maxv? (int)((double)unif_full/maxv*LARG+0.5):0; if(unif_full>0&&nb<1)nb=1;
    printf("    %-9s ","uniforme"); printf("%s",VERD); repete("█",nb); printf("%s  %s%ld%s\n\n",RESET,BOLD,unif_full,RESET);

    printf("  %s→ No dado real o baseline concentra: um bucket de %ld faz a busca varrer\n",AMAR,real_full);
    printf("    %ld nós (pior caso ~O(n)). É isso que o hash de Fibonacci vem reduzir.%s\n\n",real_full,RESET);
    return 0;
}
