#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Painel de resultados no terminal.
 * Le resultados/colisoes.csv e mostra baseline (k mod m) x Fibonacci nos
 * cenarios real e adversario, na base completa. Rode: make painel.
 * NO_COLOR=1 no ambiente desliga as cores.
 */

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
static void preparar_terminal(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    if (GetConsoleMode(h, &modo)) SetConsoleMode(h, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(65001);
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
static void fmt_milhar(long v, char *out) {
    char tmp[32]; snprintf(tmp,sizeof tmp,"%ld",v);
    int len=(int)strlen(tmp), j=0;
    for (int i=0;i<len;i++){ if(i>0 && (len-i)%3==0) out[j++]='.'; out[j++]=tmp[i]; }
    out[j]='\0';
}

#define MAXL 64
typedef struct { char funcao[24], dataset[24]; long n,m,col,maior; double fc; } Linha;

static long valor(Linha *L, int nl, long nmax, const char *ds, const char *fn, int maior) {
    for (int i=0;i<nl;i++)
        if (L[i].n==nmax && !strcmp(L[i].dataset,ds) && !strcmp(L[i].funcao,fn))
            return maior ? L[i].maior : L[i].col;
    return -1;
}

int main(int argc, char **argv) {
    preparar_terminal(); iniciar_cores();
    const char *caminho = (argc>=2)? argv[1] : "resultados/colisoes.csv";
    FILE *f = fopen(caminho,"r");
    if (!f) { fprintf(stderr,"painel: nao consegui abrir '%s'\n",caminho); return 1; }
    Linha L[MAXL]; int nl=0; char linha[256];
    if (fgets(linha,sizeof linha,f)) {}
    while (nl<MAXL && fgets(linha,sizeof linha,f)) {
        Linha *x=&L[nl];
        if (sscanf(linha,"%23[^,],%23[^,],%ld,%ld,%lf,%ld,%ld",
                   x->funcao,x->dataset,&x->n,&x->m,&x->fc,&x->col,&x->maior)==7) nl++;
    }
    fclose(f);
    if (nl==0){ fprintf(stderr,"painel: nenhum dado em '%s'\n",caminho); return 1; }
    long nmax=0; for(int i=0;i<nl;i++) if(L[i].n>nmax) nmax=L[i].n;

    printf("\n  %s%s",BOLD,CIANO); repete("═",64); printf("%s\n",RESET);
    printf("  %s%s  AUTENTICADOR DE HOMOLOGAÇÃO ANATEL   ·   G22 · EDA2 2026.2%s\n",BOLD,CIANO,RESET);
    printf("  %s  Experimento: baseline (k mod m)  ×  Fibonacci estruturado%s\n",DIM,RESET);
    printf("  %s%s",BOLD,CIANO); repete("═",64); printf("%s\n\n",RESET);

    char c1[32],c2[32];
    fmt_milhar(nmax,c1);
    printf("  Colisões e maior bucket na base completa (%s números distintos):\n\n",c1);

    printf("  ┌"); repete("─",14); printf("┬"); repete("─",13); printf("┬"); repete("─",14); printf("┬"); repete("─",14); printf("┐\n");
    printf("  │ %-12s │ %-11s │ %12s │ %12s │\n","cenario","funcao","colisoes","maior bucket");
    printf("  ├"); repete("─",14); printf("┼"); repete("─",13); printf("┼"); repete("─",14); printf("┼"); repete("─",14); printf("┤\n");
    struct { const char *ds, *fn; } linhas[] = {
        {"real","baseline"},{"real","fibonacci"},{"adversario","baseline"},{"adversario","fibonacci"}
    };
    for (int i=0;i<4;i++) {
        long col=valor(L,nl,nmax,linhas[i].ds,linhas[i].fn,0);
        long mai=valor(L,nl,nmax,linhas[i].ds,linhas[i].fn,1);
        const char *cor = strcmp(linhas[i].fn,"baseline")==0 ? VERM : VERD;
        fmt_milhar(col,c1); fmt_milhar(mai,c2);
        printf("  │ %-12s │ %s%-11s%s │ %12s │ %12s │\n",linhas[i].ds,cor,linhas[i].fn,RESET,c1,c2);
    }
    printf("  └"); repete("─",14); printf("┴"); repete("─",13); printf("┴"); repete("─",14); printf("┴"); repete("─",14); printf("┘\n\n");

    long ab=valor(L,nl,nmax,"adversario","baseline",0);
    long af=valor(L,nl,nmax,"adversario","fibonacci",0);
    long mv = ab>af?ab:af; const int LARG=40;
    printf("  %sCenário adversário (pior caso) — nº de colisões%s\n",BOLD,RESET);
    int nb = mv? (int)((double)ab/mv*LARG+0.5):0; if(ab>0&&nb<1)nb=1;
    fmt_milhar(ab,c1); printf("    %-10s %s","baseline",VERM); repete("█",nb); printf("%s  %s%s%s\n",RESET,BOLD,c1,RESET);
    nb = mv? (int)((double)af/mv*LARG+0.5):0; if(af>0&&nb<1)nb=1;
    fmt_milhar(af,c2); printf("    %-10s %s","Fibonacci",VERD); repete("█",nb); printf("%s  %s%s%s\n\n",RESET,BOLD,c2,RESET);

    double fator = af>0? (double)ab/af : 0.0;
    printf("  %s→ No dado real limpo, baseline e Fibonacci empatam (o dado nao agrupa).\n",AMAR);
    printf("    No pior caso agrupado, o Fibonacci reduz as colisoes ~%.1fx. Escolhemos\n",fator);
    printf("    o Fibonacci pela robustez ao pior caso, sem custo extra.%s\n\n",RESET);
    return 0;
}
