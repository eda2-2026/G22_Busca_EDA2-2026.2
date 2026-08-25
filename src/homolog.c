#include "homolog.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Lê exatamente n dígitos a partir de s e monta o valor inteiro.
 * Retorna false se algum caractere não for dígito. */
static bool le_digitos(const char *s, int n, uint32_t *out) {
    uint32_t v = 0;
    for (int i = 0; i < n; i++) {
        if (!isdigit((unsigned char)s[i])) return false;
        v = v * 10 + (uint32_t)(s[i] - '0');
    }
    *out = v;
    return true;
}

bool homolog_parse(const char *s, Homolog *out) {
    /* Formato: 5 dígitos, '-', 2 dígitos, '-', 5 dígitos = 14 caracteres. */
    if (s == NULL || out == NULL) return false;
    if (strlen(s) != 14) return false;
    if (s[5] != '-' || s[8] != '-') return false;

    uint32_t seq, ano, fab;
    if (!le_digitos(s + 0, 5, &seq)) return false;
    if (!le_digitos(s + 6, 2, &ano)) return false;
    if (!le_digitos(s + 9, 5, &fab)) return false;

    out->seq = seq;
    out->ano = (uint16_t)ano;
    out->fab = fab;
    return true;
}

void homolog_format(Homolog h, char *buf) {
    sprintf(buf, "%05u-%02u-%05u", h.seq, h.ano, h.fab);
}
