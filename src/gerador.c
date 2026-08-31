#include "gerador.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define GERADOR_SEQ_MAX 100000u
#define GERADOR_ANO_MAX 100u
#define GERADOR_FAB_MAX 100000u

#define GERADOR_NUMERO_LEN 15

static int str_contem_ci(const char *palheiro, const char *agulha) {
    if (palheiro == NULL || agulha == NULL) return 0;
    if (agulha[0] == '\0') return 1;
    for (const char *base = palheiro; *base != '\0'; base++) {
        size_t k = 0;
        while (agulha[k] != '\0' &&
               tolower((unsigned char)base[k]) == tolower((unsigned char)agulha[k])) {
            k++;
        }
        if (agulha[k] == '\0') return 1;
    }
    return 0;
}

static char *str_duplicar(const char *s) {
    size_t tam = strlen(s) + 1;
    char *copia = malloc(tam);
    if (copia != NULL) memcpy(copia, s, tam);
    return copia;
}

static unsigned gerador_rand_ate(unsigned limite) {
    unsigned long r = ((unsigned long)(rand() & 0x7FFF) << 15)
                    | (unsigned long)(rand() & 0x7FFF);
    return (unsigned)(r % limite);
}

static void gerador_numero_aleatorio(char *buf, size_t tam) {
    unsigned seq = gerador_rand_ate(GERADOR_SEQ_MAX);
    unsigned ano = gerador_rand_ate(GERADOR_ANO_MAX);
    unsigned fab = gerador_rand_ate(GERADOR_FAB_MAX);
    snprintf(buf, tam, "%05u-%02u-%05u", seq, ano, fab);
}

int gerador_sintetico(const char *caminho_saida, size_t n) {
    FILE *f = fopen(caminho_saida, "w");
    if (f == NULL) {
        perror("gerador_sintetico: fopen");
        return -1;
    }

    fprintf(f, "numero\n");

    for (size_t i = 0; i < n; i++) {
        char buf[GERADOR_NUMERO_LEN];
        gerador_numero_aleatorio(buf, sizeof buf);
        fprintf(f, "%s\n", buf);
    }

    if (fclose(f) != 0) {
        perror("gerador_sintetico: fclose");
        return -1;
    }

    return 0;
}

static char gerador_detectar_separador(const char *linha) {
    int pv = 0, pc = 0, pt = 0;
    for (const char *p = linha; *p != '\0'; p++) {
        if (*p == ';') pv++;
        else if (*p == ',') pc++;
        else if (*p == '\t') pt++;
    }
    if (pt > 0 && pt >= pv && pt >= pc) return '\t';
    return (pv >= pc) ? ';' : ',';
}

static char *gerador_campo_indice(char *linha, char sep, int indice) {
    int atual = 0;
    char *inicio = linha;
    for (char *p = linha; *p != '\0'; p++) {
        if (*p == sep) {
            if (atual == indice) {
                *p = '\0';
                return inicio;
            }
            atual++;
            inicio = p + 1;
        } else if (*p == '\n' || *p == '\r') {
            *p = '\0';
        }
    }
    return (atual == indice) ? inicio : NULL;
}

static char *gerador_aparar(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '"') s++;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '"')) {
        s[--len] = '\0';
    }
    return s;
}

static int gerador_formato_valido(const char *s) {
    if (s == NULL) return 0;
    size_t i = 0;
    for (int d = 0; d < 5; d++, i++) if (!isdigit((unsigned char)s[i])) return 0;
    if (s[i++] != '-') return 0;
    for (int d = 0; d < 2; d++, i++) if (!isdigit((unsigned char)s[i])) return 0;
    if (s[i++] != '-') return 0;
    for (int d = 0; d < 5; d++, i++) if (!isdigit((unsigned char)s[i])) return 0;
    return s[i] == '\0';
}

static int gerador_indice_coluna_numero(const char *cabecalho, char sep) {
    char copia[4096];
    snprintf(copia, sizeof copia, "%s", cabecalho);

    int indice = 0;
    int reserva = -1;
    char *inicio = copia;
    for (char *p = copia; ; p++) {
        if (*p == sep || *p == '\0' || *p == '\n' || *p == '\r') {
            char terminou = (*p == '\0');
            *p = '\0';
            if (str_contem_ci(inicio, "homolog")) {
                if (reserva < 0) reserva = indice;
                if (!str_contem_ci(inicio, "data")) {
                    return indice;
                }
            }
            if (terminou) break;
            indice++;
            inicio = p + 1;
        }
    }
    return reserva;
}

static int gerador_normalizar(const char *campo, char *saida) {
    if (campo == NULL) return 0;
    if (gerador_formato_valido(campo)) {
        snprintf(saida, GERADOR_NUMERO_LEN, "%s", campo);
        return 1;
    }
    size_t i = 0;
    for (; campo[i] != '\0'; i++) {
        if (i >= 12 || !isdigit((unsigned char)campo[i])) return 0;
    }
    if (i != 12) return 0;
    snprintf(saida, GERADOR_NUMERO_LEN, "%.5s-%.2s-%.5s", campo, campo + 5, campo + 7);
    return 1;
}

static int gerador_cmp_str(const void *a, const void *b);

int gerador_importar_real(const char *caminho_entrada, const char *caminho_saida) {
    FILE *in = fopen(caminho_entrada, "r");
    if (in == NULL) {
        perror("gerador_importar_real: fopen entrada");
        return -1;
    }

    char linha[4096];
    if (fgets(linha, sizeof linha, in) == NULL) {
        fprintf(stderr, "gerador_importar_real: arquivo de entrada vazio\n");
        fclose(in);
        return -1;
    }

    char sep = gerador_detectar_separador(linha);
    int idx_numero = gerador_indice_coluna_numero(linha, sep);
    if (idx_numero < 0) {
        fprintf(stderr,
                "gerador_importar_real: nao encontrei coluna com \"homolog\" no cabecalho\n");
        fclose(in);
        return -1;
    }

    FILE *out = fopen(caminho_saida, "w");
    if (out == NULL) {
        perror("gerador_importar_real: fopen saida");
        fclose(in);
        return -1;
    }
    fprintf(out, "numero\n");

    size_t cap = 1024, nv = 0, descartados = 0;
    char **vals = malloc(cap * sizeof *vals);
    if (vals == NULL) { fclose(in); fclose(out); return -1; }
    while (fgets(linha, sizeof linha, in) != NULL) {
        char *campo = gerador_campo_indice(linha, sep, idx_numero);
        if (campo != NULL) campo = gerador_aparar(campo);

        char numero[GERADOR_NUMERO_LEN];
        if (!gerador_normalizar(campo, numero)) { descartados++; continue; }

        if (nv == cap) {
            cap *= 2;
            char **tmp = realloc(vals, cap * sizeof *vals);
            if (tmp == NULL) break;
            vals = tmp;
        }
        vals[nv] = str_duplicar(numero);
        if (vals[nv] == NULL) break;
        nv++;
    }

    qsort(vals, nv, sizeof *vals, gerador_cmp_str);
    size_t validos = 0, duplicados = 0;
    for (size_t i = 0; i < nv; i++) {
        if (i > 0 && strcmp(vals[i], vals[i-1]) == 0) duplicados++;
        else { fprintf(out, "%s\n", vals[i]); validos++; }
    }
    for (size_t i = 0; i < nv; i++) free(vals[i]);
    free(vals);

    fclose(in);
    if (fclose(out) != 0) {
        perror("gerador_importar_real: fclose saida");
        return -1;
    }

    fprintf(stderr, "gerador_importar_real: %zu numeros distintos, %zu duplicados removidos, %zu descartados\n",
            validos, duplicados, descartados);
    return 0;
}

static int gerador_cmp_str(const void *a, const void *b) {
    const char *const *pa = a;
    const char *const *pb = b;
    return strcmp(*pa, *pb);
}

static char **gerador_carregar_validos(const char *caminho, size_t *out_n) {
    FILE *f = fopen(caminho, "r");
    if (f == NULL) {
        perror("gerador_consultas_falsas: fopen registro valido");
        *out_n = 0;
        return NULL;
    }

    size_t cap = 1024, n = 0;
    char **vet = malloc(cap * sizeof *vet);
    if (vet == NULL) {
        fclose(f);
        *out_n = 0;
        return NULL;
    }

    char linha[256];

    if (fgets(linha, sizeof linha, f) == NULL) {
        fclose(f);
        *out_n = 0;
        return vet;
    }

    while (fgets(linha, sizeof linha, f) != NULL) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (linha[0] == '\0') continue;

        if (n == cap) {
            cap *= 2;
            char **novo = realloc(vet, cap * sizeof *vet);
            if (novo == NULL) break;
            vet = novo;
        }
        vet[n] = str_duplicar(linha);
        if (vet[n] == NULL) break;
        n++;
    }
    fclose(f);

    qsort(vet, n, sizeof *vet, gerador_cmp_str);
    *out_n = n;
    return vet;
}

static int gerador_existe(char *const *vet, size_t n, const char *chave) {
    return bsearch(&chave, vet, n, sizeof *vet, gerador_cmp_str) != NULL;
}

static void gerador_liberar_validos(char **vet, size_t n) {
    for (size_t i = 0; i < n; i++) free(vet[i]);
    free(vet);
}

int gerador_consultas_falsas(const char *caminho_registro_valido,
                              const char *caminho_saida,
                              size_t n) {
    size_t n_validos = 0;
    char **validos = gerador_carregar_validos(caminho_registro_valido, &n_validos);
    if (validos == NULL) return -1;

    FILE *out = fopen(caminho_saida, "w");
    if (out == NULL) {
        perror("gerador_consultas_falsas: fopen saida");
        gerador_liberar_validos(validos, n_validos);
        return -1;
    }
    fprintf(out, "numero\n");

    const size_t MAX_TENTATIVAS = 1000;
    size_t gerados = 0;
    for (size_t i = 0; i < n; i++) {
        char buf[GERADOR_NUMERO_LEN];
        size_t tentativas = 0;
        do {
            gerador_numero_aleatorio(buf, sizeof buf);
            tentativas++;
        } while (gerador_existe(validos, n_validos, buf) && tentativas < MAX_TENTATIVAS);

        if (tentativas >= MAX_TENTATIVAS) {
            fprintf(stderr,
                    "gerador_consultas_falsas: nao achei numero inexistente apos %zu tentativas (parei em %zu/%zu)\n",
                    MAX_TENTATIVAS, gerados, n);
            break;
        }

        fprintf(out, "%s\n", buf);
        gerados++;
    }

    gerador_liberar_validos(validos, n_validos);

    if (fclose(out) != 0) {
        perror("gerador_consultas_falsas: fclose saida");
        return -1;
    }

    if (gerados < n) return -1;
    return 0;
}
