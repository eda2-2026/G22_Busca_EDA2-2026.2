#include "gerador.h"
#include <stdio.h>

int gerador_sintetico(const char *caminho_saida, size_t n) {
    (void)caminho_saida;
    (void)n;
    
    fprintf(stderr, "gerador_sintetico: ainda nao implementado\n");
    return -1;
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