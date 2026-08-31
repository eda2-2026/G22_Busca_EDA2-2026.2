#include "hash.h"
#include "homolog.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N_AMOSTRA 10000u
#define TAM_TABELA 1024u
#define M_BITS 10u

static void medir(const char *nome, uint64_t buckets_idx[], size_t n, size_t tam) {
    unsigned long *contagem = calloc(tam, sizeof *contagem);
    for (size_t i = 0; i < n; i++) {
        contagem[buckets_idx[i]]++;
    }

    unsigned long maior = 0;
    double soma = 0.0;
    for (size_t i = 0; i < tam; i++) {
        if (contagem[i] > maior) maior = contagem[i];
        soma += (double)contagem[i];
    }
    double media = soma / (double)tam;

    double soma_var = 0.0;
    for (size_t i = 0; i < tam; i++) {
        double d = (double)contagem[i] - media;
        soma_var += d * d;
    }
    double variancia = soma_var / (double)tam;

    printf("%-10s maior_bucket=%-6lu variancia=%.4f\n", nome, maior, variancia);
    free(contagem);
}

int main(void) {
    srand(42);

    uint64_t *idx_mod = malloc(N_AMOSTRA * sizeof *idx_mod);
    uint64_t *idx_fib = malloc(N_AMOSTRA * sizeof *idx_fib);

    for (size_t i = 0; i < N_AMOSTRA; i++) {

        Homolog h;
        h.seq = (uint32_t)i;
        h.ano = (uint16_t)(i % 10);
        h.fab = (uint32_t)((i % 20) * 137);

        uint64_t k_bruto = (uint64_t)h.seq * 10000000ULL
                          + (uint64_t)h.ano * 100000ULL
                          + (uint64_t)h.fab;
        idx_mod[i] = hash_mod(k_bruto, TAM_TABELA);

        uint64_t k_estruturada = chave_estruturada(h);
        idx_fib[i] = hash_fib(k_estruturada, M_BITS);
    }

    printf("Amostra agrupada (%u chaves, tabela=%u buckets):\n", N_AMOSTRA, TAM_TABELA);
    medir("hash_mod", idx_mod, N_AMOSTRA, TAM_TABELA);
    medir("hash_fib", idx_fib, N_AMOSTRA, TAM_TABELA);

    free(idx_mod);
    free(idx_fib);
    return 0;
}
