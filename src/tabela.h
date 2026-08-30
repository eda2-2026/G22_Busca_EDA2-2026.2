#ifndef TABELA_H
#define TABELA_H

#include <stddef.h>
#include <stdbool.h>
#include "homolog.h"


typedef struct No {
    Homolog dado;
    struct No *prox;
} No;

typedef struct {
    No **buckets;
    size_t capacidade;      /* numero de buckets — precisa ser potencia de 2 (hash_fib usa m = log2) */
    size_t total;           /* numero de elementos inseridos */
    unsigned long colisoes; /* insercoes que cairam num bucket ja ocupado */
} Tabela;

Tabela *tabela_criar(size_t capacidade);
bool tabela_inserir(Tabela *t, Homolog h);
bool tabela_buscar(const Tabela *t, Homolog h);
void tabela_liberar(Tabela *t);


/* Fator de carga atual: total de elementos / numero de buckets. */
double tabela_fator_carga(const Tabela *t);

/* Verdadeiro quando o fator de carga passou de "limite" e a tabela
 * deveria crescer antes da proxima insercao. */
bool tabela_deve_crescer(const Tabela *t, double limite);

/* Limite padrao de fator de carga usado por tabela_inserir para
 * disparar o rehashing automatico (commit 19). */
#define TABELA_FATOR_CARGA_LIMITE 0.75



/*
 * Realoca o array de buckets para "nova_capacidade" (deve ser
 * potencia de 2) e reinsere todos os elementos existentes, recalculando
 * o indice de cada um para o novo tamanho. Os nos existentes sao
 * reaproveitados (relinkados), nao recriados. Retorna false em falha
 * de alocacao (tabela fica inalterada nesse caso).
 */
bool tabela_rehash(Tabela *t, size_t nova_capacidade);

#endif /* TABELA_H */