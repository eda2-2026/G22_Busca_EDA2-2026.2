#ifndef HOMOLOG_H
#define HOMOLOG_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t seq;
    uint16_t ano;
    uint32_t fab;
} Homolog;

bool homolog_parse(const char *s, Homolog *out);

void homolog_format(Homolog h, char *buf);

#endif
