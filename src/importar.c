#include "gerador.h"
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
                "uso: %s <csv_bruto_anatel> [csv_saida]\n"
                "  csv_saida padrao: dados/registro_real.csv\n",
                argv[0]);
        return 2;
    }
    const char *entrada = argv[1];
    const char *saida = (argc >= 3) ? argv[2] : "dados/registro_real.csv";

    if (gerador_importar_real(entrada, saida) != 0) {
        fprintf(stderr, "importar: falhou\n");
        return 1;
    }
    printf("OK: registro real escrito em %s\n", saida);
    return 0;
}
