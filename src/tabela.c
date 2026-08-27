#include "tabela.h"
#include <stdlib.h>

static bool homolog_igual(Homolog a, Homolog b) {
    return a.seq == b.seq && a.ano == b.ano && a.fab == b.fab;
}

TabelaHash *tabela_criar(size_t m, FuncaoHash hash) {
    if (m == 0 || hash == NULL) return NULL;
    TabelaHash *t = malloc(sizeof *t);
    if (t == NULL) return NULL;
    t->buckets = calloc(m, sizeof *t->buckets);   /* todos NULL */
    if (t->buckets == NULL) { free(t); return NULL; }
    t->m = m;
    t->n = 0;
    t->hash = hash;
    return t;
}

void tabela_destruir(TabelaHash *t) {
    if (t == NULL) return;
    for (size_t i = 0; i < t->m; i++) {
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

int tabela_inserir(TabelaHash *t, Homolog chave) {
    size_t i = t->hash(chave, t->m);
    No *no = malloc(sizeof *no);
    if (no == NULL) return -1;
    no->chave = chave;
    no->prox = t->buckets[i];    /* insere na cabeca da lista */
    t->buckets[i] = no;
    t->n++;
    return 0;
}

bool tabela_buscar(const TabelaHash *t, Homolog chave) {
    size_t i = t->hash(chave, t->m);
    for (const No *no = t->buckets[i]; no != NULL; no = no->prox) {
        if (homolog_igual(no->chave, chave)) return true;
    }
    return false;
}

EstatisticasTabela tabela_estatisticas(const TabelaHash *t) {
    EstatisticasTabela e = {0};
    e.m = t->m;
    e.n = t->n;
    e.fator_carga = t->m ? (double)t->n / (double)t->m : 0.0;

    for (size_t i = 0; i < t->m; i++) {
        size_t len = 0;
        for (const No *no = t->buckets[i]; no != NULL; no = no->prox) len++;
        if (len > 0) {
            e.buckets_ocupados++;
            if (len > e.maior_bucket) e.maior_bucket = len;
        }
    }
    e.buckets_vazios = t->m - e.buckets_ocupados;
    e.colisoes = t->n - e.buckets_ocupados;   /* chaves alem da 1a por bucket */

    /* variancia do comprimento dos buckets (contando os vazios) */
    double media = e.fator_carga, soma2 = 0.0;
    for (size_t i = 0; i < t->m; i++) {
        size_t len = 0;
        for (const No *no = t->buckets[i]; no != NULL; no = no->prox) len++;
        double d = (double)len - media;
        soma2 += d * d;
    }
    e.variancia = t->m ? soma2 / (double)t->m : 0.0;
    return e;
}
