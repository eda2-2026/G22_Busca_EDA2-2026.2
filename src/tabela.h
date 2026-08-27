#ifndef TABELA_H
#define TABELA_H

#include <stddef.h>
#include <stdbool.h>
#include "homolog.h"

/*
 * Tabela hash ESTATICA por encadeamento separado (separate chaining).
 *
 * "Estatica" no sentido do item 1.5 da ementa: o numero de buckets (m)
 * e' fixado na criacao e nao cresce. Como o conjunto de chaves e'
 * conhecido, da' pra dimensionar m e chegar a pouquissimas (ou zero)
 * colisoes com um bom hash. A versao DINAMICA (rehashing ao passar do
 * fator de carga) fica a cargo do outro modulo.
 *
 * A funcao de hash e' um PARAMETRO: a mesma tabela recebe tanto o hash
 * baseline (hash.h) quanto o de Fibonacci. E' isso que torna o
 * experimento "baseline x Fibonacci" uma troca de ponteiro, medindo as
 * duas na mesma estrutura.
 */

/* Uma funcao de hash mapeia uma chave para um indice em [0, m). */
typedef size_t (*FuncaoHash)(Homolog chave, size_t m);

/* No da lista encadeada de um bucket. */
typedef struct No {
    Homolog      chave;
    struct No   *prox;
} No;

typedef struct {
    No       **buckets;  /* vetor de m listas encadeadas */
    size_t     m;        /* numero de buckets            */
    size_t     n;        /* numero de chaves inseridas   */
    FuncaoHash hash;     /* funcao de hash em uso        */
} TabelaHash;

/* Estatisticas de ocupacao -- alimentam os graficos do experimento. */
typedef struct {
    size_t m;
    size_t n;
    double fator_carga;      /* n / m                                    */
    size_t buckets_ocupados;
    size_t buckets_vazios;
    size_t maior_bucket;     /* pior caso de busca (comprimento maximo)  */
    size_t colisoes;         /* n - buckets_ocupados                     */
    double variancia;        /* variancia do comprimento dos buckets     */
} EstatisticasTabela;

/* Cria uma tabela com m buckets e a funcao de hash dada.
 * Retorna NULL em falha de alocacao ou parametro invalido. */
TabelaHash *tabela_criar(size_t m, FuncaoHash hash);

/* Libera toda a memoria da tabela (inclusive as listas). */
void tabela_destruir(TabelaHash *t);

/* Insere uma chave (nao remove duplicatas: o registro e' um conjunto e
 * duplicatas exatas nao mudam a resposta da busca). Retorna 0 em
 * sucesso, -1 em falha de alocacao. */
int tabela_inserir(TabelaHash *t, Homolog chave);

/* Busca uma chave. Retorna true se existe (=> GENUINO). */
bool tabela_buscar(const TabelaHash *t, Homolog chave);

/* Calcula as estatisticas de ocupacao da tabela. */
EstatisticasTabela tabela_estatisticas(const TabelaHash *t);

#endif /* TABELA_H */
