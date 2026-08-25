#ifndef HOMOLOG_H
#define HOMOLOG_H

#include <stdint.h>
#include <stdbool.h>

/* Número de homologação HHHHH-AA-FFFFF decomposto em campos. */
typedef struct {
    uint32_t seq;   /* HHHHH: sequencial do produto (0..99999)       */
    uint16_t ano;   /* AA:    ano da homologação   (0..99)           */
    uint32_t fab;   /* FFFFF: identificação do fabricante (0..99999) */
} Homolog;

/* Faz o parse de "HHHHH-AA-FFFFF".
 * Retorna true e preenche *out em caso de sucesso;
 * retorna false se o formato for inválido. */
bool homolog_parse(const char *s, Homolog *out);

/* Formata um Homolog de volta para "HHHHH-AA-FFFFF".
 * buf deve ter espaço para pelo menos 15 bytes. */
void homolog_format(Homolog h, char *buf);

#endif /* HOMOLOG_H */