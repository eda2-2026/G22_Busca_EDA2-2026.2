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
    size_t capacidade;
    size_t total;
    unsigned long colisoes;
} Tabela;

Tabela *tabela_criar(size_t capacidade);
bool tabela_inserir(Tabela *t, Homolog h);
bool tabela_buscar(const Tabela *t, Homolog h);
void tabela_liberar(Tabela *t);

double tabela_fator_carga(const Tabela *t);

bool tabela_deve_crescer(const Tabela *t, double limite);

#define TABELA_FATOR_CARGA_LIMITE 0.75

bool tabela_rehash(Tabela *t, size_t nova_capacidade);

#endif
