#include "hash.h"

uint64_t chave_naive(Homolog h) {
    /* fab ocupa as 5 casas decimais baixas, ano as 2 seguintes,
     * seq as 5 mais altas: "HHHHHAAFFFFF". Cabe folgado em 64 bits. */
    return (uint64_t)h.seq * 10000000ULL
         + (uint64_t)h.ano * 100000ULL
         + (uint64_t)h.fab;
}

size_t hash_baseline(Homolog h, size_t m) {
    return (size_t)(chave_naive(h) % (uint64_t)m);
}
