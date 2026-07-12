# Makefile minimal — Phase 1 (D1 : C99, zéro warning toléré).
# Aucune dépendance externe. Le moteur (src/) reste sans I/O ;
# les tests (tests/) sont le seul point d'affichage pour l'instant.

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2
ENGINE  = src/hex.c src/board.c src/rng.c src/setup.c src/turn.c src/build.c src/game.c
HEADERS = $(wildcard src/*.h)
TESTS   = tests/test_board tests/test_setup tests/test_turn tests/test_build

.PHONY: all test clean

all: test

# Compile et exécute tous les tests.
test: $(TESTS)
	./tests/test_board
	./tests/test_setup
	./tests/test_turn
	./tests/test_build

tests/test_board: tests/test_board.c $(ENGINE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_board.c $(ENGINE)

tests/test_setup: tests/test_setup.c $(ENGINE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_setup.c $(ENGINE)

tests/test_turn: tests/test_turn.c $(ENGINE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_turn.c $(ENGINE)

tests/test_build: tests/test_build.c $(ENGINE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_build.c $(ENGINE)

clean:
	rm -f $(TESTS)
