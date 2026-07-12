# Makefile minimal — Phase 1 (D1 : C99, zéro warning toléré).
# Aucune dépendance externe. Le moteur (src/) reste sans I/O ;
# le test (tests/) est le seul point d'affichage pour l'instant.

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2
ENGINE  = src/hex.c src/board.c src/game.c
TESTBIN = tests/test_board

.PHONY: all test clean

all: test

# Compile et exécute les tests topologiques.
test: $(TESTBIN)
	./$(TESTBIN)

$(TESTBIN): tests/test_board.c $(ENGINE) $(wildcard src/*.h)
	$(CC) $(CFLAGS) -o $@ tests/test_board.c $(ENGINE)

clean:
	rm -f $(TESTBIN)
