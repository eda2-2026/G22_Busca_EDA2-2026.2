#ifndef GERADOR_H
#define GERADOR_H

#include <stddef.h>
#include "homolog.h"

int gerador_sintetico(const char *caminho_saida, size_t n);

int gerador_importar_real(const char *caminho_entrada, const char *caminho_saida);

int gerador_consultas_falsas(const char *caminho_registro_valido,
                              const char *caminho_saida,
                              size_t n);

#endif
