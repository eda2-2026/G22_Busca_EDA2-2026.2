#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include "homolog.h"

/* Baseline ingenuo, para comparacao (commit 12, Anna). h(k) = k mod m. */
uint64_t hash_mod(uint64_t chave, uint64_t tam);

/*
 * Chave inteira consciente do tipo (commit 13, Esdras): separa e
 * mistura os campos de Homolog com primos distintos, em vez de tratar
 * o numero como uma unica sequencia de digitos. Isso evita que a
 * estrutura do dado (sequencial denso, ano/fabricante repetidos) vaze
 * direto pra chave.
 */
uint64_t chave_estruturada(Homolog h);

/*
 * Hash de Fibonacci / multiplicative-shift (commit 14, Esdras): lê os
 * "m" bits altos de (chave * constante de Fibonacci de 64 bits), que
 * dependem de TODOS os bits da chave de entrada — dissolve sequencias
 * e agrupamentos que uma funcao ingenua deixaria passar.
 * "m" e log2(tamanho da tabela), ou seja a tabela deve ter tamanho
 * potencia de 2.
 */
uint64_t hash_fib(uint64_t chave, uint32_t m);

#endif /* HASH_H */