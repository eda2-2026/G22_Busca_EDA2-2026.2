#include "gerador.h"
#include <stdio.h>

/*
 * Driver do importador do dado real.
 *
 * Le o CSV bruto baixado do Portal de Dados Abertos da Anatel e escreve
 * o registro no formato do projeto (header "numero", uma chave
 * HHHHH-AA-FFFFF por linha), pronto pra ser lido pelo verificador e
 * pelo benchmark.
 *
 *   uso: importar <csv_bruto> [csv_saida]
 *   csv_saida padrao: dados/registro_real.csv
 */
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
