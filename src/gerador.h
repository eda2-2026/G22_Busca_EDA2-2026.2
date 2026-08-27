#ifndef GERADOR_H
#define GERADOR_H

#include <stddef.h>
#include "homolog.h"

/*
 * Modulo gerador: produz os tres datasets usados no experimento
 *
 *   registro_real.csv       -> gerador_importar_real()
 *   registro_sintetico.csv  -> gerador_sintetico()
 *   consultas_falsas.csv    -> gerador_consultas_falsas()
 *
 * Contrato comum: toda funcao devolve 0 em sucesso e -1 em erro
 * (arquivo nao pode ser aberto/escrito, parametro invalido etc.).
 */

/*
 * Gera "n" registros sinteticos com chaves uniformemente distribuidas
 * (grupo de controle, sem os agrupamentos do dado real) e escreve em
 * formato CSV no caminho indicado.
 */
int gerador_sintetico(const char *caminho_saida, size_t n);

/*
 * Le a amostra bruta baixada do Portal de Dados Abertos da Anatel
 * (dados.gov.br) e converte/mapeia as colunas de origem para o
 * formato interno do projeto (Homolog), escrevendo o resultado em
 * dados/registro_real.csv.
 */
int gerador_importar_real(const char *caminho_entrada, const char *caminho_saida);

/*
 * Gera "n" consultas falsas: numeros de homologacao fora do conjunto
 * definido em "caminho_registro_valido", usados para medir o pior
 * caso de busca (consulta por algo que nao existe no registro).
 */
int gerador_consultas_falsas(const char *caminho_registro_valido,
                              const char *caminho_saida,
                              size_t n);

#endif /* GERADOR_H */