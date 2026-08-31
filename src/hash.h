#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include "homolog.h"

uint64_t hash_mod(uint64_t chave, uint64_t tam);

uint64_t chave_estruturada(Homolog h);

uint64_t hash_fib(uint64_t chave, uint32_t m);

#endif
