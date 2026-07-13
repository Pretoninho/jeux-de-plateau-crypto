# Makefile minimal — Phase 1 (D1 : C99, zéro warning toléré).
# Aucune dépendance externe.
#   src/ (hors ui.c/main.c) = moteur pur, sans I/O.
#   src/ui.c, src/main.c    = interface terminal (seule couche d'I/O).

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2
ENGINE  = src/hex.c src/board.c src/rng.c src/setup.c src/turn.c src/build.c src/score.c src/game.c
UI      = src/ui.c
HEADERS = $(wildcard src/*.h)
BIN     = crypto-board
TESTS   = tests/test_board tests/test_setup tests/test_turn tests/test_build \
          tests/test_score tests/test_ui

.PHONY: all test run clean

all: test $(BIN)

# Compile et exécute tous les tests.
test: $(TESTS)
	./tests/test_board
	./tests/test_setup
	./tests/test_turn
	./tests/test_build
	./tests/test_score
	./tests/test_ui

# Binaire de jeu (interface terminal).
$(BIN): src/main.c $(ENGINE) $(UI) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ src/main.c $(ENGINE) $(UI)

# Démonstration automatique.
run: $(BIN)
	./$(BIN) --seed 1 --players 3 --demo 40

tests/test_board: tests/test_board.c $(ENGINE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_board.c $(ENGINE)

tests/test_setup: tests/test_setup.c $(ENGINE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_setup.c $(ENGINE)

tests/test_turn: tests/test_turn.c $(ENGINE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_turn.c $(ENGINE)

tests/test_build: tests/test_build.c $(ENGINE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_build.c $(ENGINE)

tests/test_score: tests/test_score.c $(ENGINE) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_score.c $(ENGINE)

tests/test_ui: tests/test_ui.c $(ENGINE) $(UI) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ tests/test_ui.c $(ENGINE) $(UI)

clean:
	rm -f $(TESTS) $(BIN)
