# Makefile - G22 Busca EDA2 2026.2
# Cross-platform: Linux/macOS (make) e Windows/MinGW (mingw32-make).
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O2
SRC     := src
BIN     := bin

ifeq ($(OS),Windows_NT)
  SHELL := cmd.exe
  EXE   := .exe
  RUN    = $(subst /,\,$1)
  RMBIN  = rmdir /s /q $(BIN)
else
  EXE   :=
  RUN    = ./$1
  RMBIN  = rm -rf $(BIN)
endif

TEST  := $(BIN)/homolog_test$(EXE)
TTEST := $(BIN)/tabela_test$(EXE)
CLI   := $(BIN)/homolog_cli$(EXE)
VER   := $(BIN)/verificador$(EXE)
IMP   := $(BIN)/importar$(EXE)
PAIN  := $(BIN)/painel$(EXE)
BENCH := $(BIN)/benchmark$(EXE)

.PHONY: all test cli verificar importar painel benchmark gerador clean

# 'make' sem argumento compila todos os binarios em bin/.
all: $(TEST) $(TTEST) $(CLI) $(VER) $(IMP) $(PAIN) $(BENCH)

# ---- Testes automaticos ----
$(TEST): $(SRC)/homolog_test.c $(SRC)/homolog.c $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/homolog_test.c $(SRC)/homolog.c

$(TTEST): $(SRC)/tabela_test.c $(SRC)/tabela.c $(SRC)/hash.c $(SRC)/homolog.c $(SRC)/tabela.h $(SRC)/hash.h $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/tabela_test.c $(SRC)/tabela.c $(SRC)/hash.c $(SRC)/homolog.c

test: $(TEST) $(TTEST)
	$(call RUN,$(TEST))
	$(call RUN,$(TTEST))

# ---- Verificador GENUINO/FALSO (parse + tabela estatica + hash baseline) ----
$(VER): $(SRC)/verificador.c $(SRC)/tabela.c $(SRC)/hash.c $(SRC)/homolog.c $(SRC)/tabela.h $(SRC)/hash.h $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/verificador.c $(SRC)/tabela.c $(SRC)/hash.c $(SRC)/homolog.c

verificar: $(VER)
	$(call RUN,$(VER))

# ---- Importador do dado real da Anatel (driver do gerador_importar_real) ----
$(IMP): $(SRC)/importar.c $(SRC)/gerador.c $(SRC)/gerador.h $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/importar.c $(SRC)/gerador.c

importar: $(IMP)

# ---- Painel de resultados no terminal ----
$(PAIN): $(SRC)/painel.c | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/painel.c

painel: $(PAIN)
	$(call RUN,$(PAIN))

# ---- Benchmark do experimento (baseline x Fibonacci) ----
$(BENCH): $(SRC)/benchmark.c $(SRC)/hash.c $(SRC)/homolog.c $(SRC)/hash.h $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/benchmark.c $(SRC)/hash.c $(SRC)/homolog.c

benchmark: $(BENCH)
	$(call RUN,$(BENCH))

# ---- CLI de teste manual do parse ----
$(CLI): $(SRC)/homolog_cli.c $(SRC)/homolog.c $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/homolog_cli.c $(SRC)/homolog.c

cli: $(CLI)
	$(call RUN,$(CLI))

# ---- Gerador (modulo do Esdras): sem main proprio ainda ----
gerador: $(SRC)/gerador.c $(SRC)/gerador.h $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -c $(SRC)/gerador.c -o $(BIN)/gerador.o

$(BIN):
	mkdir $(BIN)

clean:
	-$(RMBIN)
