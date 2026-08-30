#include "hash.h"
#include "homolog.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 *  teste standalone (nao depende de tabela.h) so pra validar
 * que hash_fib espalha melhor que hash_mod ANTES de plugar na tabela
 * de verdade (Fase 4). Quando a tabela existir, o benchmark.c oficial
 * do projeto assume essa medicao.
 */

#define N_AMOSTRA 10000u
#define TAM_TABELA 1024u /* potencia de 2, m = 10 */
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
    srand(42); /* reproduzivel */

    uint64_t *idx_mod = malloc(N_AMOSTRA * sizeof *idx_mod);
    uint64_t *idx_fib = malloc(N_AMOSTRA * sizeof *idx_fib);

    for (size_t i = 0; i < N_AMOSTRA; i++) {
        /* Simula o agrupamento real: poucos anos (10) e poucos
         * fabricantes (20), sequencial denso e crescente — o "pior
         * caso" que a proposta descreve pra funcao ingenua. */
        Homolog h;
        h.seq = (uint32_t)i;               /* denso, sequencial */
        h.ano = (uint16_t)(i % 10);        /* so 10 anos distintos */
        h.fab = (uint32_t)((i % 20) * 137); /* so 20 fabricantes */

        /* Baseline (Secao 4.1): k e o numero BRUTO, os 12 digitos
         * decimais concatenados (HHHHH-AA-FFFFF -> HHHHHAAFFFFF),
         * sem nenhuma mistura — e por isso que a estrutura "vaza". */
        uint64_t k_bruto = (uint64_t)h.seq * 10000000ULL
                          + (uint64_t)h.ano * 100000ULL
                          + (uint64_t)h.fab;
        idx_mod[i] = hash_mod(k_bruto, TAM_TABELA);

        /* Melhorada (Secao 4.2): mistura estruturada + Fibonacci. */
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