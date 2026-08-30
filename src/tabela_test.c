#include "tabela.h"
#include <stdio.h>
#include <assert.h>

#define CAPACIDADE_INICIAL 8u
#define N_INSERCOES 500u

int main(void) {
    Tabela *t = tabela_criar(CAPACIDADE_INICIAL);
    assert(t != NULL);

    for (unsigned i = 0; i < N_INSERCOES; i++) {
        Homolog h = { .seq = i, .ano = (uint16_t)(i % 30), .fab = i * 7 };
        bool ok = tabela_inserir(t, h);
        assert(ok);
    }

    printf("capacidade final: %zu (inicial era %u)\n", t->capacidade, CAPACIDADE_INICIAL);
    printf("total inserido: %zu\n", t->total);
    printf("fator de carga final: %.3f\n", tabela_fator_carga(t));
    assert(t->capacidade > CAPACIDADE_INICIAL); /* cresceu pelo menos uma vez */
    assert(t->total == N_INSERCOES);

    size_t achados = 0;
    for (unsigned i = 0; i < N_INSERCOES; i++) {
        Homolog h = { .seq = i, .ano = (uint16_t)(i % 30), .fab = i * 7 };
        if (tabela_buscar(t, h)) achados++;
    }
    printf("achados apos rehash: %zu / %u\n", achados, N_INSERCOES);
    assert(achados == N_INSERCOES);

    Homolog inexistente = { .seq = 999999, .ano = 99, .fab = 999999 };
    assert(!tabela_buscar(t, inexistente));

    tabela_liberar(t);
    printf("OK: rehashing preservou todos os elementos.\n");
    return 0;
}