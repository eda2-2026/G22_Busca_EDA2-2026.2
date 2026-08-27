#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>
#include "homolog.h"

/*
 * Funcoes de hash para o numero de homologacao.
 *
 * BASELINE (este modulo): h(k) = k mod m, onde k e' a "chave numerica
 * ingenua" -- os tres campos concatenados como um unico inteiro. Quando
 * m e' potencia de 2, "mod m" enxerga apenas os BITS BAIXOS de k, que
 * sao dominados pelo fabricante (FFFFF). Como fabricantes se repetem no
 * registro real, as chaves se concentram em poucos buckets. Isso e' de
 * proposito: e' o "antes" contra o qual a versao melhorada e' medida.
 *
 * A versao MELHORADA (hash de Fibonacci sobre uma chave estruturada com
 * primos) tem a MESMA assinatura de FuncaoHash (ver tabela.h) e mora no
 * modulo do parceiro; as duas plugam na mesma TabelaHash. Trocar o hash
 * do experimento e', literalmente, trocar um ponteiro de funcao.
 */

/* Chave numerica "ingenua": concatena os campos HHHHH AA FFFFF num unico
 * inteiro de 64 bits -> seq*10^7 + ano*10^5 + fab. */
uint64_t chave_naive(Homolog h);

/* Hash baseline: chave_naive(h) % m. Assume m > 0. */
size_t hash_baseline(Homolog h, size_t m);

#endif /* HASH_H */
