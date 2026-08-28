#include "gerador.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Limites de cada campo no formato HHHHH-AA-FFFFF (Secao 1 da proposta). */
#define GERADOR_SEQ_MAX 100000u /* HHHHH: 00000-99999 (5 digitos) */
#define GERADOR_ANO_MAX 100u    /* AA:    00-99       (2 digitos) */
#define GERADOR_FAB_MAX 100000u /* FFFFF: 00000-99999 (5 digitos) */

/* "HHHHH-AA-FFFFF\0" = 5+1+2+1+5+1 = 15 bytes */
#define GERADOR_NUMERO_LEN 15

/* Substring case-insensitive portatil: o strcasestr do glibc nao existe
 * no MinGW. Retorna 1 se "agulha" aparece em "palheiro", ignorando caixa. */
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

/* strdup portatil: nao e' declarado no MinGW sob -std=c11.
 * Retorna NULL em falha de alocacao. */
static char *str_duplicar(const char *s) {
    size_t tam = strlen(s) + 1;
    char *copia = malloc(tam);
    if (copia != NULL) memcpy(copia, s, tam);
    return copia;
}

/* rand() garante apenas ate RAND_MAX (32767 no MinGW; ~2 bilhoes no
 * glibc). Para sortear uniformemente em intervalos maiores que 32767
 * (seq e fab vao ate 99999), juntamos dois blocos de 15 bits, o que
 * cobre o intervalo em qualquer plataforma. */
static unsigned gerador_rand_ate(unsigned limite) {
    unsigned long r = ((unsigned long)(rand() & 0x7FFF) << 15)
                    | (unsigned long)(rand() & 0x7FFF);
    return (unsigned)(r % limite);
}

/*
 * Sorteia um numero no formato HHHHH-AA-FFFFF, com cada campo uniforme
 * e independente. Extraida do commit 7 para ser reaproveitada tambem
 * pelo commit 9 (consultas falsas), sem mudar a interface publica.
 */
static void gerador_numero_aleatorio(char *buf, size_t tam) {
    unsigned seq = gerador_rand_ate(GERADOR_SEQ_MAX);
    unsigned ano = gerador_rand_ate(GERADOR_ANO_MAX);
    unsigned fab = gerador_rand_ate(GERADOR_FAB_MAX);
    snprintf(buf, tam, "%05u-%02u-%05u", seq, ano, fab);
}

/*
 * Grupo de controle: sorteia cada campo (seq/ano/fab) de forma
 * independente e uniforme dentro dos limites do formato. Ao contrario
 * do dado real, aqui nao ha agrupamento por ano/fabricante nem
 * sequencial denso, entao o "teto de qualidade" do experimento (Secao 5)
 * vem daqui.
 *
 * rand() nao e semeado propositalmente: sem srand() explicito em algum
 * ponto do programa (main de benchmark/verificador), a sequencia e
 * deterministica (seed = 1), o que mantem os experimentos reprodutiveis
 * entre execucoes. Se quiserem variar entre rodadas, chamem
 * srand(time(NULL)) antes, no chamador.
 *
 * Duplicatas exatas sao estatisticamente possiveis (espaco de chaves e
 * grande, mas finito) e nao invalidam o experimento: o que importa e a
 * distribuicao de colisoes na tabela hash, nao a unicidade da chave.
 */
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

/* Descobre o separador do CSV de origem. Dados publicos brasileiros
 * aparecem tanto com ';'/',' quanto com TAB (caso do export real da
 * Anatel) — conta as tres ocorrencias na linha de cabecalho e usa a
 * mais frequente, com tab tendo prioridade quando presente (separador
 * inequivoco, ao contrario de ';'/',' que podem aparecer dentro de
 * texto livre tambem). */
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

/* Retorna o campo de indice "indice" (0-based) da linha, terminando-o
 * com '\0' no lugar do separador (a linha e modificada). Tambem corta
 * '\r'/'\n' do fim. Nao trata separador dentro de aspas (limitacao
 * conhecida: se o CSV real vier com campos entre aspas contendo ';'
 * ou ',', ajustar aqui). Retorna NULL se a linha nao tiver esse indice. */
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

/* Remove espacos, tabs e aspas das pontas do campo, em place. */
static char *gerador_aparar(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '"') s++;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '"')) {
        s[--len] = '\0';
    }
    return s;
}

/* Confere se "s" esta exatamente no formato HHHHH-AA-FFFFF. */
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

/* Acha, no cabecalho, a coluna do numero de homologacao. O export
 * real da Anatel tem DUAS colunas com "Homolog" no nome ("Data da
 * Homologação" e "Número de Homologação"), entao "contem homolog" nao
 * basta — preferimos a que NAO contem "data" (afasta "Data da
 * Homologação" e "Data de Validade do Certificado"). Se nenhuma
 * bater nesse criterio mais estrito, cai pro primeiro "homolog" como
 * ultimo recurso. Retorna o indice (0-based) ou -1 se nao achar nada. */
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

/*
 * Le a amostra bruta baixada do Portal de Dados Abertos e produz um
 * CSV no mesmo formato usado por gerador_sintetico (header "numero",
 * uma chave HHHHH-AA-FFFFF por linha), pronto pra ser lido junto com
 * o resto do pipeline.
 *
 * Como o nome exato da coluna de origem varia (e eu nao tive acesso
 * ao arquivo real da Anatel pra confirmar), a busca e por conteudo
 * ("contem 'homolog'", case-insensitive) em vez de indice fixo. Se o
 * arquivo de voces usar um nome que nao bate com isso, so ajustar a
 * string em gerador_indice_coluna_numero.
 */
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

    size_t validos = 0, descartados = 0;
    while (fgets(linha, sizeof linha, in) != NULL) {
        char *campo = gerador_campo_indice(linha, sep, idx_numero);
        if (campo != NULL) campo = gerador_aparar(campo);

        if (!gerador_formato_valido(campo)) {
            descartados++;
            continue;
        }

        fprintf(out, "%s\n", campo);
        validos++;
    }

    fclose(in);
    if (fclose(out) != 0) {
        perror("gerador_importar_real: fclose saida");
        return -1;
    }

    fprintf(stderr, "gerador_importar_real: %zu validos, %zu descartados (linha fora do formato)\n",
            validos, descartados);
    return 0;
}

static int gerador_cmp_str(const void *a, const void *b) {
    const char *const *pa = a;
    const char *const *pb = b;
    return strcmp(*pa, *pb);
}

/* Carrega a coluna "numero" de um CSV (mesmo formato de saida dos
 * outros geradores) num vetor de strings ordenado, pronto pra busca
 * binaria. Preenche *out_n com a quantidade lida (0 em caso de erro,
 * junto com retorno NULL). */
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
    /* descarta o cabecalho ("numero"); arquivo vazio => sem registros */
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
            if (novo == NULL) break; /* segue com o que deu pra carregar */
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

/*
 * Gera "n" numeros no formato HHHHH-AA-FFFFF que NAO aparecem em
 * caminho_registro_valido — o pior caso de busca (Secao 5 da
 * proposta): consultar algo que nao existe precisa varrer o bucket
 * inteiro sem achar nada.
 *
 * Estrategia: carrega o registro valido inteiro em memoria e ordena
 * (O(m log m)), depois sorteia candidatos e confere cada um com busca
 * binaria (O(log m)) ate juntar "n" que nao colidam com o registro.
 * Cada candidato tem um limite de tentativas pra nao entrar em loop
 * infinito se "n" for maior que o espaco de chaves livre.
 */
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