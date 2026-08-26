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

TEST := $(BIN)/homolog_test$(EXE)
CLI  := $(BIN)/homolog_cli$(EXE)

.PHONY: all test cli gerador clean

# Alvo padrao: compila e roda a bateria de testes automaticos.
all: test

# --- Modulo homolog (parse/format) + testes automaticos ---
$(TEST): $(SRC)/homolog_test.c $(SRC)/homolog.c $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/homolog_test.c $(SRC)/homolog.c

test: $(TEST)
	$(call RUN,$(TEST))

# --- CLI para teste MANUAL do parse (interativo ou por argumentos) ---
$(CLI): $(SRC)/homolog_cli.c $(SRC)/homolog.c $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)/homolog_cli.c $(SRC)/homolog.c

cli: $(CLI)
	$(call RUN,$(CLI))

# --- Gerador (modulo do Esdras): sem main proprio ainda ---
gerador: $(SRC)/gerador.c $(SRC)/gerador.h $(SRC)/homolog.h | $(BIN)
	$(CC) $(CFLAGS) -c $(SRC)/gerador.c -o $(BIN)/gerador.o

$(BIN):
	mkdir $(BIN)

clean:
	-$(RMBIN)
