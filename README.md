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

## 2. Como compilar e rodar

> **Pré-requisitos:** `gcc` e `make` — nada além da biblioteca padrão de C. No **Windows com MinGW**, o comando é `mingw32-make` no lugar de `make`.

**Verificação rápida** (compila tudo e roda os testes automáticos):

```
make test            # Linux / macOS
mingw32-make test    # Windows (MinGW)
```

Deve terminar com `RESULTADO: SUCESSO` nas duas baterias (parse + tabela).

**Alvos disponíveis:**

| Alvo | O que faz |
| --- | --- |
| `test` | compila e roda os testes automáticos (parse + tabela) |
| `verificar` | verificador interativo (GENUÍNO / FALSO / INVÁLIDO) |
| `cli` | CLI de teste do parser do número |
| `importar` | driver de importação do dado real da Anatel |
| `clean` | limpa a pasta `bin/` |

**Exemplos de uso** (no Windows os executáveis têm sufixo `.exe`, ex.: `.\bin\verificador.exe`):

```
# verificar numeros contra o registro real
./bin/verificador --csv dados/registro_real.csv 03340-19-04952 99999-99-99999

# importar o CSV bruto da Anatel -> registro no formato do projeto
./bin/importar dados/anatel_bruto.csv dados/registro_real.csv

# gerar os graficos a partir de resultados/colisoes.csv (requer matplotlib)
python resultados/gerar_graficos.py
```

## 3. O que o software faz

Verificador em linha de comando. Ele carrega o registro (um CSV de números válidos) numa tabela hash e responde, para cada consulta:

```
$ ./verificador --csv dados/registro_real.csv 03340-19-04952 99999-99-99999 abcde-19-04952
  "03340-19-04952"  ->  GENUINO   (encontrado no registro)
  "99999-99-99999"  ->  FALSO     (formato ok, mas nao esta no registro)
  "abcde-19-04952"  ->  INVALIDO  (fora do formato HHHHH-AA-FFFFF)
```

Três respostas: **GENUÍNO** (bem formado e presente no registro), **FALSO** (bem formado, mas ausente) e **INVÁLIDO** (nem está no formato). Há também um modo interativo (rodar sem números digita à mão; sem `--csv`, usa um registro de demonstração embutido).

Fluxo interno: (1) parse e validação de formato → (2) hash do número → (3) busca no bucket → (4) resposta. (Exibir metadados como fabricante/ano é uma extensão natural, ainda não implementada.)

## 4. Objetivo e contribuição

Construir um **verificador de homologação** que responde `GENUÍNO` / `FALSO` para um número informado, indexando o registro por uma **tabela hash**.

A contribuição do trabalho não é apenas usar hash, mas **projetar uma função de hash sob medida para o formato da Anatel** (`HHHHH-AA-FFFFF`), que minimiza colisões em relação a uma função genérica, e **comprovar esse ganho com um experimento**. Como o registro não segue uma distribuição uniforme — anos e fabricantes se repetem e o número sequencial é denso, uma função ingênua concentra chaves em poucos buckets. Nosso índice explora a estrutura dos campos para espalhar melhor as chaves.

## 5. Núcleo técnico - a função de hash

O número cabe num inteiro de 64 bits (`~10¹² < 2⁴⁰`). Comparamos duas funções sobre a mesma estratégia de resolução de colisão (encadeamento), para que a diferença medida seja **só da função**.

> **Status:** o baseline (5.1) está implementado e medido (Seção 7). A função de Fibonacci (5.2) e a tabela dinâmica (5.3) estão em implementação e plugam na mesma tabela.

### 5.1 Baseline: módulo simples

`h(k) = k mod m`. Serve de ponto de comparação: por olhar sobretudo os dígitos baixos da chave, tende a deixar a estrutura dos dados (como o agrupamento por fabricante) se refletir nos buckets.

### 5.2 Melhorada: Fibonacci estruturado

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

### 5.3 Estático × dinâmico

- **Estático:** o registro é um conjunto conhecido, ou seja, dá para construir um hash quase perfeito (colisões ≈ 0).
- **Dinâmico:** novas homologações entram com o tempo, ou seja, tabela crescível com **rehashing** ao ultrapassar o fator de carga. Medimos o custo de manter o desempenho conforme a base cresce.

## 6. Experimento

Datasets do mesmo tamanho:

- **real** - amostra de números da Anatel (distribuição agrupada);
- **sintético uniforme** - grupo de controle, para mostrar o teto de qualidade;
- **consultas falsas** - números inexistentes, gerados contra o próprio registro real (pior caso: confirmar ausência).

Tamanhos: **10k / 100k / 189k** (a base real completa tem 189.317 números). Métricas medidas para cada função:

- número total de **colisões**;
- **maior bucket** e **variância** do tamanho dos buckets (o quanto agrupa);
- **fator de carga** na hora da medição;
- **tempo médio** e **tempo de pior caso** de consulta;
- **custo de construção** (e de rehashing, no dinâmico).

Saída: `resultados/colisoes.csv` + gráficos comparando baseline × Fibonacci.

## 7. Resultados (parciais)

Baseline implementado: chave inteira por concatenação dos campos (`chave_naive`) e `h(k) = k mod m`, com `m` potência de 2 e resolução por encadeamento. Comparação no **dado real da Anatel** contra o **sintético uniforme** de mesmo tamanho e mesma tabela:

| tamanho | dataset | colisões | maior bucket |
| --- | --- | ---: | ---: |
| 10 mil | real | 3.581 | 19 |
| 10 mil | uniforme | 2.526 | 5 |
| 100 mil | real | 50.673 | 240 |
| 100 mil | uniforme | 16.849 | 7 |
| 189 mil (full) | real | **109.211** | **401** |
| 189 mil (full) | uniforme | 30.473 | 5 |

![Baseline: dado real da Anatel × sintético uniforme](resultados/graficos/baseline_real_vs_uniforme.png)

**Leitura:** com a mesma tabela e o mesmo fator de carga (~0,36), mudando só a natureza do dado, o baseline no dado real colide ~3,6× mais e o **maior bucket chega a 401** — contra 5 no uniforme. Um número que caia nesse bucket de 401 faz a busca varrer 401 nós encadeados: é o pior caso O(n) se manifestando. E o efeito **piora com a escala** (maior bucket 19 → 240 → 401), confirmando que o registro real agrupa (anos e fabricantes se repetem) e que o `k mod m` deixa essa estrutura vazar para poucos buckets. É exatamente o problema que a função de Fibonacci (Seção 5.2) vem atacar.

Os dados brutos estão em `resultados/colisoes.csv` (esquema `funcao,dataset,n,m,fator_carga,colisoes,maior_bucket`) e o gráfico é gerado por `resultados/gerar_graficos.py`. Quando o benchmark do hash de Fibonacci rodar, ele acrescenta linhas `funcao=fibonacci` ao mesmo CSV e o gráfico ganha as barras comparativas — sem retrabalho.

> O registro real vem do [Portal de Dados Abertos](https://dados.gov.br/dados/conjuntos-dados/produtos-de-telecomunicacoes-homologados-pela-anatel) (189.317 produtos homologados). No arquivo bruto o número vem como 12 dígitos sem traços; o importador (`gerador_importar_real` + `bin/importar`) reconstrói o formato `HHHHH-AA-FFFFF`.

## 8. Análise de complexidade

| Função | Tempo médio | Pior caso | Espalhamento nos dados reais |
| ------ | ----------- | --------- | ---------------------------- |
| `k mod m` (baseline) | O(1) | O(n) | ruim — estrutura vaza p/ buckets |
| Fibonacci estruturado | O(1) | O(n) | bom — difunde ano/fabricante/sequencial |
| Hash perfeito (estático) | O(1) | O(1) | ideal — colisão zero no conjunto conhecido |

Referências para a discussão (por que não usamos): busca binária exige vetor ordenado e dá O(log n); interpolação chega a O(log log n) só em dados uniformes - que a Anatel não é.

## 9. Estrutura de pastas

```
G22_Busca_EDA2-2026.2/
├── README.md
├── Makefile                       # cross-platform (make / mingw32-make)
├── .gitattributes                 # normaliza fim de linha (LF)
├── dados/
│   ├── registro_real.csv          # 189.317 homologacoes reais (Anatel)
│   ├── registro_sintetico_pequeno.csv   # amostra uniforme (controle)
│   └── consultas_falsas_pequeno.csv     # amostra de numeros inexistentes
├── src/
│   ├── homolog.h / homolog.c      # parse e formatacao do numero
│   ├── homolog_test.c             # testes do parse
│   ├── homolog_cli.c              # CLI de teste do parse
│   ├── hash.h / hash.c            # chave_naive + hash_baseline (k mod m)
│   ├── tabela.h / tabela.c        # tabela hash estatica (encadeamento) + estatisticas
│   ├── tabela_test.c              # testes da tabela
│   ├── verificador.c              # CLI: numero -> GENUINO/FALSO/INVALIDO
│   ├── gerador.h / gerador.c      # datasets: sintetico, importacao real, consultas falsas
│   └── importar.c                 # driver do importador do dado real
├── resultados/
│   ├── colisoes.csv               # metricas medidas
│   ├── gerar_graficos.py          # gera os graficos a partir do CSV
│   └── graficos/                  # PNGs gerados
└── docs/
```

Ainda por integrar (Esdras): `hash_fib`, a tabela dinâmica com rehashing e o `benchmark.c` (tempo).

## 10. Status

- [x] Parse e validação do número (`homolog`) + testes
- [x] Tabela hash estática (encadeamento) + estatísticas + testes
- [x] Hash baseline `k mod m`
- [x] Verificador GENUÍNO / FALSO / INVÁLIDO
- [x] Importação do dado real da Anatel (189.317 números)
- [x] Gráficos do baseline (real × uniforme)
- [ ] Hash de Fibonacci
- [ ] Tabela dinâmica com rehashing
- [ ] Benchmark de tempo + gráfico final baseline × Fibonacci
- [ ] Vídeo (5 min)

## 11. Bibliografia

- Drozdek, A. *Data Structures and Algorithms in C++*, 2ª ed., Brooks/Cole, 2001.
- Weiss, M. A. *Data Structures and Algorithm Analysis in C++*, 3ª ed., Addison Wesley, 2006.
- Cormen, T. H. et al. *Introduction to Algorithms*, 3ª ed., MIT Press, 2009. (cap. de hashing)
