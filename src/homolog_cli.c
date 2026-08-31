#include "homolog.h"
#include <stdio.h>
#include <string.h>

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
        linha[strcspn(linha, "\r\n")] = '\0';
        if (linha[0] == '\0') continue;
        avalia(linha);
    }
    printf("\nate mais!\n");
    return 0;
}
