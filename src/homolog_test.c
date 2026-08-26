#include "homolog.h"
#include <stdio.h>
#include <string.h>

/*
 * Testes do modulo homolog (parse/format).
 *
 * Cada assercao imprime [ok] ou [FALHOU] e incrementa os contadores.
 * O programa devolve 0 se tudo passou e 1 se houve qualquer falha,
 * para servir tanto ao alvo "make test" quanto a uma futura CI.
 */

static int total  = 0;
static int falhas = 0;

/* Espera que o parse de `s` TENHA SUCESSO e que os campos batam. */
static void espera_valido(const char *rotulo, const char *s,
                          uint32_t seq, uint16_t ano, uint32_t fab) {
    total++;
    Homolog h;
    if (!homolog_parse(s, &h)) {
        falhas++;
        printf("  [FALHOU] %-16s \"%s\" deveria ser VALIDO, mas foi rejeitado\n",
               rotulo, s);
        return;
    }
    if (h.seq != seq || h.ano != ano || h.fab != fab) {
        falhas++;
        printf("  [FALHOU] %-16s \"%s\" -> seq=%u ano=%u fab=%u "
               "(esperado seq=%u ano=%u fab=%u)\n",
               rotulo, s, h.seq, h.ano, h.fab, seq, ano, fab);
        return;
    }
    printf("  [ok]     %-16s \"%s\" -> seq=%u ano=%u fab=%u\n",
           rotulo, s, h.seq, h.ano, h.fab);
}

/* Espera que o parse de `s` FALHE (formato invalido). */
static void espera_invalido(const char *rotulo, const char *s) {
    total++;
    const char *disp = s ? s : "(NULL)";
    Homolog h;
    if (homolog_parse(s, &h)) {
        falhas++;
        printf("  [FALHOU] %-16s \"%s\" deveria ser INVALIDO, mas passou\n",
               rotulo, disp);
        return;
    }
    printf("  [ok]     %-16s \"%s\" rejeitado como esperado\n", rotulo, disp);
}

/* Espera que parse -> format devolva exatamente a string original. */
static void espera_roundtrip(const char *s) {
    total++;
    Homolog h;
    char buf[16];
    if (!homolog_parse(s, &h)) {
        falhas++;
        printf("  [FALHOU] round-trip     \"%s\" nao parseou\n", s);
        return;
    }
    homolog_format(h, buf);
    if (strcmp(buf, s) != 0) {
        falhas++;
        printf("  [FALHOU] round-trip     \"%s\" -> \"%s\"\n", s, buf);
        return;
    }
    printf("  [ok]     round-trip     \"%s\" preservado\n", s);
}

int main(void) {
    printf("== Testes de homolog_parse / homolog_format ==\n");

    printf("\n-- Casos validos --\n");
    espera_valido("caso base",   "03340-19-04952",  3340, 19,  4952);
    espera_valido("limite alto", "99999-99-99999", 99999, 99, 99999);
    espera_valido("todos zeros", "00000-00-00000",     0,  0,     0);

    printf("\n-- Casos invalidos --\n");
    espera_invalido("tamanho curto",  "123-45-6789");
    espera_invalido("letras no seq",  "abcde-19-04952");
    espera_invalido("string vazia",   "");
    espera_invalido("ponteiro nulo",  NULL);
    espera_invalido("separador ruim", "03340_19_04952");
    espera_invalido("letra no ano",   "03340-1a-04952");
    espera_invalido("digito a mais",  "033400-19-04952");

    printf("\n-- Round-trip parse->format (zeros a esquerda preservados) --\n");
    espera_roundtrip("03340-19-04952");
    espera_roundtrip("00000-00-00000");
    espera_roundtrip("99999-99-99999");

    printf("\n== Resumo: %d/%d passaram ==\n", total - falhas, total);
    if (falhas > 0) {
        printf("RESULTADO: FALHOU (%d teste(s) com erro)\n", falhas);
        return 1;
    }
    printf("RESULTADO: SUCESSO (todos os testes passaram)\n");
    return 0;
}
