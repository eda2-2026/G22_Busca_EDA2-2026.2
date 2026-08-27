#include "homolog.h"
#include "hash.h"
#include "tabela.h"
#include <stdio.h>

/* Testes da tabela hash estatica usando o hash baseline. */

static int total = 0, falhas = 0;

static void checa(const char *rotulo, int cond) {
    total++;
    if (cond) {
        printf("  [ok]     %s\n", rotulo);
    } else {
        falhas++;
        printf("  [FALHOU] %s\n", rotulo);
    }
}

static Homolog H(uint32_t seq, uint16_t ano, uint32_t fab) {
    Homolog h = { seq, ano, fab };
    return h;
}

int main(void) {
    printf("== Testes da tabela hash estatica (hash baseline) ==\n\n");

    TabelaHash *t = tabela_criar(16, hash_baseline);
    checa("tabela_criar devolve nao-NULL", t != NULL);
    checa("tabela_criar(0, ...) devolve NULL", tabela_criar(0, hash_baseline) == NULL);

    Homolog a = H(3340, 19, 4952);
    Homolog b = H(1, 1, 0);
    Homolog c = H(1, 1, 16);   /* chave_naive difere de b por 16 -> colide em m=16 */
    Homolog x = H(9, 9, 9);    /* nunca inserido */

    checa("busca em tabela vazia -> false", tabela_buscar(t, a) == false);

    tabela_inserir(t, a);
    tabela_inserir(t, b);
    tabela_inserir(t, c);

    checa("busca de chave inserida (a) -> true", tabela_buscar(t, a) == true);
    checa("busca de chave inserida (b) -> true", tabela_buscar(t, b) == true);
    checa("busca de chave inserida (c) -> true", tabela_buscar(t, c) == true);
    checa("busca de chave ausente (x) -> false", tabela_buscar(t, x) == false);

    EstatisticasTabela e = tabela_estatisticas(t);
    checa("n == 3", e.n == 3);
    checa("b e c colidiram: maior_bucket >= 2", e.maior_bucket >= 2);
    checa("colisoes >= 1", e.colisoes >= 1);
    checa("fator de carga ~ 3/16", e.fator_carga > 0.18 && e.fator_carga < 0.19);

    /* duplicata exata nao quebra a busca */
    tabela_inserir(t, a);
    checa("apos duplicata, busca(a) ainda true", tabela_buscar(t, a) == true);
    checa("n incrementa para 4", tabela_estatisticas(t).n == 4);

    tabela_destruir(t);

    printf("\n== Resumo: %d/%d passaram ==\n", total - falhas, total);
    if (falhas > 0) {
        printf("RESULTADO: FALHOU (%d com erro)\n", falhas);
        return 1;
    }
    printf("RESULTADO: SUCESSO (todos os testes passaram)\n");
    return 0;
}
