#include "gerador.h"
#include <stdio.h>
#include <stdlib.h>

/* Limites de cada campo no formato HHHHH-AA-FFFFF. */
#define GERADOR_SEQ_MAX 100000u /* HHHHH: 00000-99999 (5 digitos) */
#define GERADOR_ANO_MAX 100u    /* AA:    00-99       (2 digitos) */
#define GERADOR_FAB_MAX 100000u /* FFFFF: 00000-99999 (5 digitos) */
 
/*
 * Grupo de controle: sorteia cada campo (seq/ano/fab) de forma
 * independente e uniforme dentro dos limites do formato. Ao contrario
 * do dado real, aqui nao ha agrupamento por ano/fabricante nem
 * sequencial denso, entao o "teto de qualidade" do experimento (Secao 5)
 * vem daqui.
 *
 * rand() nao e semeado propositalmente: sem srand() explicito em algum
 * ponto do programa (main de benchmark/verificador), a sequencia e
 * deterministica (seed = 1), o que mantem os experimentos reprodutiveis
 * entre execucoes. Se quiserem variar entre rodadas, chamem
 * srand(time(NULL)) antes, no chamador.
 *
 * Duplicatas exatas sao estatisticamente possiveis (espaco de chaves e
 * grande, mas finito) e nao invalidam o experimento: o que importa e a
 * distribuicao de colisoes na tabela hash, nao a unicidade da chave.
 */
int gerador_sintetico(const char *caminho_saida, size_t n) {
    FILE *f = fopen(caminho_saida, "w");
    if (f == NULL) {
        perror("gerador_sintetico: fopen");
        return -1;
    }
 
    fprintf(f, "numero\n");
 
    for (size_t i = 0; i < n; i++) {
        unsigned seq = (unsigned)(rand() % GERADOR_SEQ_MAX);
        unsigned ano = (unsigned)(rand() % GERADOR_ANO_MAX);
        unsigned fab = (unsigned)(rand() % GERADOR_FAB_MAX);
        fprintf(f, "%05u-%02u-%05u\n", seq, ano, fab);
    }
 
    if (fclose(f) != 0) {
        perror("gerador_sintetico: fclose");
        return -1;
    }
 
    return 0;
}


int gerador_importar_real(const char *caminho_entrada, const char *caminho_saida) {
    (void)caminho_entrada;
    (void)caminho_saida;
    
    fprintf(stderr, "gerador_importar_real: ainda nao implementado\n");
    return -1;
}

int gerador_consultas_falsas(const char *caminho_registro_valido,
                              const char *caminho_saida,
                              size_t n) {
    (void)caminho_registro_valido;
    (void)caminho_saida;
    (void)n;

    fprintf(stderr, "gerador_consultas_falsas: ainda nao implementado\n");
    return -1;
}