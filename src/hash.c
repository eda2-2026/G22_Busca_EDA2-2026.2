#include "hash.h"

#define FIB64 0x9E3779B97F4A7C15ULL

uint64_t hash_mod(uint64_t chave, uint64_t tam) {
    return chave % tam;
}

uint64_t chave_estruturada(Homolog h) {
    uint64_t k = (uint64_t)h.seq * 1000003ULL;
    k ^= (uint64_t)h.ano * 19349663ULL;
    k ^= (uint64_t)h.fab * 83492791ULL;
    return k;
}

uint64_t hash_fib(uint64_t chave, uint32_t m) {
    return (chave * FIB64) >> (64 - m);
}
