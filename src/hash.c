#include "hash.h"

/* 2^64 / phi, impar — constante de Fibonacci de 64 bits (Secao 4.2). */
#define FIB64 0x9E3779B97F4A7C15ULL


uint64_t hash_mod(uint64_t chave, uint64_t tam) {
    return chave % tam;
}

/* --- commit 13: hash: chave estruturada por campo --- */
uint64_t chave_estruturada(Homolog h) {
    uint64_t k = (uint64_t)h.seq * 1000003ULL;  /* primo */
    k ^= (uint64_t)h.ano * 19349663ULL;         /* primo */
    k ^= (uint64_t)h.fab * 83492791ULL;         /* primo */
    return k;
}

/* --- commit 14: hash: funcao de Fibonacci estruturada --- */
uint64_t hash_fib(uint64_t chave, uint32_t m) {
    return (chave * FIB64) >> (64 - m);
}