#include "homolog.h"
#include <stdio.h>
#include <string.h>

/*
 * CLI para testar o parse do numero de homologacao MANUALMENTE.
 *
 * Uso:
 *   ./bin/homolog_cli 03340-19-04952 99999-99-99999   -> avalia cada argumento
 *   ./bin/homolog_cli                                  -> modo interativo (digite e Enter)
 *
 * Para cada entrada mostra os campos (seq/ano/fab) e o round-trip do
 * format, ou "INVALIDO" se o formato HHHHH-AA-FFFFF nao for reconhecido.
 * Ainda NAO diz GENUINO/FALSO: isso e' a busca na tabela hash, que ainda
 * nao foi implementada. Aqui so' se testa a leitura/validacao do numero.
 */
static void avalia(const char *s) {
    Homolog h;
    if (homolog_parse(s, &h)) {
        char buf[16];
        homolog_format(h, buf);
        printf("  \"%s\"  ->  VALIDO    seq=%u  ano=%u  fab=%u   (re-format: %s)\n",
               s, h.seq, h.ano, h.fab, buf);
    } else {
        printf("  \"%s\"  ->  INVALIDO  (formato HHHHH-AA-FFFFF nao reconhecido)\n", s);
    }
}

int main(int argc, char **argv) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++) avalia(argv[i]);
        return 0;
    }
    char linha[256];
    printf("Numero de homologacao (HHHHH-AA-FFFFF) + Enter. Ctrl+D (Ctrl+Z no Windows) para sair.\n\n");
    while (fgets(linha, sizeof linha, stdin)) {
        linha[strcspn(linha, "\r\n")] = '\0';   /* remove o Enter, aceita LF e CRLF */
        if (linha[0] == '\0') continue;
        avalia(linha);
    }
    printf("\nate mais!\n");
    return 0;
}
