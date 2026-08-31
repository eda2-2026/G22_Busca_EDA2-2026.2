#include "homolog.h"
#include "tabela.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Verificador de Homologacao Anatel.
 *
 * Monta a tabela hash (dinamica, hash de Fibonacci — modulo do Esdras)
 * a partir de um registro de numeros validos e responde, por consulta:
 *
 *   GENUINO   -> numero bem formado E presente no registro
 *   FALSO     -> numero bem formado, mas ausente do registro
 *   INVALIDO  -> nem esta no formato HHHHH-AA-FFFFF
 *
 * Uso:
 *   verificador [--demo | --csv ARQUIVO] [numeros...]
 *   sem numeros -> modo interativo ; sem fonte -> registro DEMO embutido.
 */

static const char *DEMO[] = {
    "03340-19-04952", "01234-20-00042", "99999-21-12345",
    "05000-18-04952", "00001-22-00001",
};

static size_t proxima_pot2(size_t x) {
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

static Homolog *carregar_demo(size_t *n_out) {
    size_t n = sizeof(DEMO) / sizeof(DEMO[0]);
    Homolog *reg = malloc(n * sizeof *reg);
    if (reg == NULL) return NULL;
    for (size_t i = 0; i < n; i++) homolog_parse(DEMO[i], &reg[i]);
    *n_out = n;
    return reg;
}

static Homolog *carregar_csv(const char *path, size_t *n_out, size_t *inval_out) {
    FILE *f = fopen(path, "r");
    if (f == NULL) { perror(path); return NULL; }
    size_t cap = 64, n = 0, inval = 0;
    Homolog *reg = malloc(cap * sizeof *reg);
    if (reg == NULL) { fclose(f); return NULL; }
    char linha[256];
    while (fgets(linha, sizeof linha, f)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (linha[0] == '\0') continue;
        if (strcmp(linha, "numero") == 0) continue;
        Homolog h;
        if (!homolog_parse(linha, &h)) { inval++; continue; }
        if (n == cap) {
            cap *= 2;
            Homolog *tmp = realloc(reg, cap * sizeof *reg);
            if (tmp == NULL) { free(reg); fclose(f); return NULL; }
            reg = tmp;
        }
        reg[n++] = h;
    }
    fclose(f);
    *n_out = n; *inval_out = inval;
    return reg;
}

static void verificar(const Tabela *t, const char *s) {
    Homolog h;
    if (!homolog_parse(s, &h)) {
        printf("  \"%s\"  ->  INVALIDO  (fora do formato HHHHH-AA-FFFFF)\n", s);
        return;
    }
    if (tabela_buscar(t, h))
        printf("  \"%s\"  ->  GENUINO   (encontrado no registro)\n", s);
    else
        printf("  \"%s\"  ->  FALSO     (formato ok, mas nao esta no registro)\n", s);
}

int main(int argc, char **argv) {
    const char *csv = NULL;
    int usar_demo = 1, nconsultas = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--demo") == 0) usar_demo = 1;
        else if (strcmp(argv[i], "--csv") == 0) {
            if (i + 1 >= argc) { fprintf(stderr, "erro: --csv exige um arquivo\n"); return 2; }
            csv = argv[++i]; usar_demo = 0;
        } else nconsultas++;
    }

    size_t n = 0, invalidas = 0;
    Homolog *reg = usar_demo ? carregar_demo(&n) : carregar_csv(csv, &n, &invalidas);
    if (reg == NULL) { fprintf(stderr, "erro: nao foi possivel carregar o registro\n"); return 1; }

    size_t cap = proxima_pot2(n * 10 / 7 + 1);
    if (cap < 16) cap = 16;
    Tabela *t = tabela_criar(cap);          /* tabela dinamica + Fibonacci (Esdras) */
    if (t == NULL) { fprintf(stderr, "erro: sem memoria para a tabela\n"); free(reg); return 1; }
    for (size_t i = 0; i < n; i++) tabela_inserir(t, reg[i]);

    /* estatisticas do estado atual (a tabela pode ter crescido via rehashing) */
    size_t ocupados = 0, maior = 0;
    for (size_t i = 0; i < t->capacidade; i++) {
        size_t len = 0;
        for (No *no = t->buckets[i]; no != NULL; no = no->prox) len++;
        if (len) { ocupados++; if (len > maior) maior = len; }
    }
    size_t colisoes = t->total - ocupados;

    printf("Verificador Anatel  |  hash: Fibonacci estruturado (tabela dinamica)\n");
    printf("Registro: %s\n", usar_demo ? "DEMO embutido" : csv);
    printf("  %zu numeros | %zu buckets | fator de carga %.2f | colisoes %zu | maior bucket %zu\n",
           t->total, t->capacidade, tabela_fator_carga(t), colisoes, maior);
    if (!usar_demo && invalidas > 0)
        printf("  (%zu linha(s) ignorada(s) por formato invalido)\n", invalidas);
    if (usar_demo) {
        printf("Numeros GENUINOS no demo (teste estes e variacoes deles):\n");
        for (size_t i = 0; i < sizeof(DEMO)/sizeof(DEMO[0]); i++)
            printf("    %s\n", DEMO[i]);
    }

    if (nconsultas > 0) {
        printf("\n");
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--demo") == 0) continue;
            if (strcmp(argv[i], "--csv") == 0) { i++; continue; }
            verificar(t, argv[i]);
        }
    } else {
        printf("\nDigite um numero (HHHHH-AA-FFFFF) + Enter. Ctrl+Z e Enter (Windows) para sair.\n\n");
        char linha[256];
        while (fgets(linha, sizeof linha, stdin)) {
            linha[strcspn(linha, "\r\n")] = '\0';
            if (linha[0] == '\0') continue;
            verificar(t, linha);
        }
        printf("\nate mais!\n");
    }

    tabela_liberar(t);
    free(reg);
    return 0;
}
