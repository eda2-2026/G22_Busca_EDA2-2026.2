# G22_Busca_EDA2-2026.2 — Autenticador de Homologação Anatel

**Disciplina:** Estruturas de Dados 2 (EDA2) - 2026.2
**Professor:** Maurício Serrano
**Trabalho:** T1 - Algoritmos de Busca
**Dupla:** Anna Clara Cardoso Evangelista Brandão (222006354) e Esdras de Sousa Nogueira (222006230)
**Repositório:** `github.com/eda2-2026/G22_Busca_EDA2-2026.2`

---

## 1. Problema

Aparelhos de telecomunicação vendidos no Brasil precisam ser homologados pela Anatel, e cada um recebe um **número de homologação** de 12 dígitos no formato `HHHHH-AA-FFFFF`:

- `HHHHH` - número sequencial do produto no cadastro (5 dígitos);
- `AA` - ano da homologação (2 dígitos);
- `FFFFF` - identificação do fabricante (5 dígitos).

Produtos falsificados ou importados irregularmente frequentemente exibem números de homologação inexistentes ou trocados. Verificar a autenticidade é, na prática, um problema de **busca**: dado um número, ele existe no registro oficial e seus metadados (marca, modelo, fabricante) batem?

## 2. Objetivo e contribuição

Construir um **verificador de homologação** que responde `GENUÍNO` / `FALSO` para um número informado, indexando o registro por uma **tabela hash**.

A contribuição do trabalho não é apenas usar hash, mas **projetar uma função de hash sob medida para o formato da Anatel** (`HHHHH-AA-FFFFF`), que minimiza colisões em relação a uma função genérica, e **comprovar esse ganho com um experimento**. Como o registro não segue uma distribuição uniforme — anos e fabricantes se repetem e o número sequencial é denso, uma função ingênua concentra chaves em poucos buckets. Nosso índice explora a estrutura dos campos para espalhar melhor as chaves.

## 3. O que o software faz

CLI simples:

```
$ ./verificador 03340-19-04952
GENUÍNO  - Fabricante: [nome]  | Ano: 2019  | Bucket: 1274  | Colisões no bucket: 0

$ ./verificador 99999-99-99999
FALSO    - número não encontrado no registro
```

Fluxo interno: (1) parse e validação de formato → (2) hash do número → (3) busca no bucket →
(4) resposta com metadados.

## 4. Núcleo técnico - a função de hash

O número cabe num inteiro de 64 bits (`~10¹² < 2⁴⁰`). Comparamos duas funções sobre a mesma estratégia de resolução de colisão (encadeamento), para que a diferença medida seja **só da função**.

### 4.1 Baseline: módulo simples

`h(k) = k mod m`. Serve de ponto de comparação: por olhar sobretudo os dígitos baixos da chave, tende a deixar a estrutura dos dados (como o agrupamento por fabricante) se refletir nos buckets.

### 4.2 Melhorada: Fibonacci estruturado

Combina os campos com primos distintos e aplica hashing multiplicativo de Fibonacci, que lê os **bits altos** do produto — dependentes de todos os bits da chave. Dissolve sequências e agrupamentos com 1 multiplicação + 1 shift.

```c
#include <stdint.h>

#define FIB64 0x9E3779B97F4A7C15ULL  // 2^64 / phi, ímpar

typedef struct { uint32_t seq; uint16_t ano; uint32_t fab; } Homolog;

// Chave inteira consciente do tipo: separa e mistura os campos
static inline uint64_t chave_estruturada(Homolog h) {
    uint64_t k = (uint64_t)h.seq * 1000003ULL;   // primo
    k ^= (uint64_t)h.ano * 19349663ULL;          // primo
    k ^= (uint64_t)h.fab * 83492791ULL;          // primo
    return k;
}

// Hash de Fibonacci: m = log2(tamanho da tabela), lê os m bits altos
static inline uint64_t hash_fib(uint64_t k, uint32_t m) {
    return (k * FIB64) >> (64 - m);
}

// Baseline ingênuo, para comparação
static inline uint64_t hash_mod(uint64_t k, uint64_t tam) {
    return k % tam;
}
```

### 4.3 Estático × dinâmico

- **Estático:** o registro é um conjunto conhecido, ou seja, dá para construir um hash quase perfeito (colisões ≈ 0).
- **Dinâmico:** novas homologações entram com o tempo, ou seja, tabela crescível com **rehashing** aoultrapassar o fator de carga. Medimos o custo de manter o desempenho conforme a base cresce.

## 5. Experimento

Datasets do mesmo tamanho:

- **real** - amostra de números da Anatel (distribuição agrupada);
- **sintético uniforme** - grupo de controle, para mostrar o teto de qualidade;
- **consultas falsas** - números inexistentes (pior caso: confirmar ausência).

Tamanhos: **10k / 100k / 1M**. Métricas medidas para cada função:

- número total de **colisões**;
- **maior bucket** e **variância** do tamanho dos buckets (o quanto agrupa);
- **fator de carga** na hora da medição;
- **tempo médio** e **tempo de pior caso** de consulta;
- **custo de construção** (e de rehashing, no dinâmico).

Saída: `resultados/colisoes.csv` + gráficos comparando baseline × Fibonacci.

## 6. Análise de complexidade

| Função | Tempo médio | Pior caso | Espalhamento nos dados reais |
| ------ | ----------- | --------- | ---------------------------- |
| `k mod m` (baseline) | O(1) | O(n) | ruim — estrutura vaza p/ buckets |
| Fibonacci estruturado | O(1) | O(n) | bom — difunde ano/fabricante/sequencial |
| Hash perfeito (estático) | O(1) | O(1) | ideal — colisão zero no conjunto conhecido |

Referências para a discussão (por que não usamos): busca binária exige vetor ordenado e dá O(log n); interpolação chega a O(log log n) só em dados uniformes - que a Anatel não é.

## 7. Estrutura de pastas

```
G22_Busca_EDA2-2026.2/
├── README.md
├── dados/
│   ├── registro_real.csv          # homologações reais (amostra Anatel)
│   ├── registro_sintetico.csv     # chaves uniformes (controle)
│   └── consultas_falsas.csv       # números inexistentes (pior caso)
├── src/
│   ├── homolog.h / homolog.c      # parse do número, campos
│   ├── hash.h / hash.c            # hash_mod (baseline) + hash_fib (melhorada)
│   ├── tabela.h / tabela.c        # tabela hash estática + dinâmica (rehashing)
│   ├── verificador.c              # CLI: número -> GENUÍNO/FALSO + metadados
│   └── benchmark.c                # mede colisões, buckets, tempo
├── resultados/
│   ├── colisoes.csv
│   └── graficos/
└── docs/
    └── roteiro_video.md
```

## 8. Bibliografia

- Drozdek, A. *Data Structures and Algorithms in C++*, 2ª ed., Brooks/Cole, 2001.
- Weiss, M. A. *Data Structures and Algorithm Analysis in C++*, 3ª ed., Addison Wesley, 2006.
- Cormen, T. H. et al. *Introduction to Algorithms*, 3ª ed., MIT Press, 2009. (cap. de hashing)
