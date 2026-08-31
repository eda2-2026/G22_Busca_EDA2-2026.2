#include "tabela.h"
#include "hash.h"
#include <stdlib.h>

static uint32_t tabela_m(size_t capacidade) {
    return (uint32_t)__builtin_ctzll((unsigned long long)capacidade);
}

static size_t tabela_indice(size_t capacidade, Homolog h) {
    uint64_t chave = chave_estruturada(h);
    return (size_t)hash_fib(chave, tabela_m(capacidade));
}

Tabela *tabela_criar(size_t capacidade) {
    Tabela *t = malloc(sizeof *t);
    if (t == NULL) return NULL;

    t->buckets = calloc(capacidade, sizeof *t->buckets);
    if (t->buckets == NULL) {
        free(t);
        return NULL;
    }

    t->capacidade = capacidade;
    t->total = 0;
    t->colisoes = 0;
    return t;
}

bool tabela_inserir(Tabela *t, Homolog h) {

    if (tabela_deve_crescer(t, TABELA_FATOR_CARGA_LIMITE)) {
        if (!tabela_rehash(t, t->capacidade * 2)) {

        }
    }

    size_t idx = tabela_indice(t->capacidade, h);

    No *no = malloc(sizeof *no);
    if (no == NULL) return false;
    no->dado = h;

    if (t->buckets[idx] != NULL) {
        t->colisoes++;
    }

    no->prox = t->buckets[idx];
    t->buckets[idx] = no;
    t->total++;
    return true;
}

bool tabela_buscar(const Tabela *t, Homolog h) {
    size_t idx = tabela_indice(t->capacidade, h);
    for (No *no = t->buckets[idx]; no != NULL; no = no->prox) {
        if (no->dado.seq == h.seq && no->dado.ano == h.ano && no->dado.fab == h.fab) {
            return true;
        }
    }
    return false;
}

void tabela_liberar(Tabela *t) {
    if (t == NULL) return;
    for (size_t i = 0; i < t->capacidade; i++) {
        No *no = t->buckets[i];
        while (no != NULL) {
            No *prox = no->prox;
            free(no);
            no = prox;
        }
    }
    free(t->buckets);
    free(t);
}

double tabela_fator_carga(const Tabela *t) {
    return (double)t->total / (double)t->capacidade;
}

bool tabela_deve_crescer(const Tabela *t, double limite) {
    return tabela_fator_carga(t) > limite;
}

bool tabela_rehash(Tabela *t, size_t nova_capacidade) {
    No **novos_buckets = calloc(nova_capacidade, sizeof *novos_buckets);
    if (novos_buckets == NULL) return false;

    for (size_t i = 0; i < t->capacidade; i++) {
        No *no = t->buckets[i];
        while (no != NULL) {
            No *prox = no->prox;

            size_t novo_idx = tabela_indice(nova_capacidade, no->dado);
            no->prox = novos_buckets[novo_idx];
            novos_buckets[novo_idx] = no;

            no = prox;
        }
    }

    free(t->buckets);
    t->buckets = novos_buckets;
    t->capacidade = nova_capacidade;

    return true;
}
