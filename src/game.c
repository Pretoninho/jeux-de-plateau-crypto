/* game.c — initialisation de l'état de jeu (cf. game.h). */
#include "game.h"
#include "setup.h"

#include <assert.h>
#include <string.h>

void game_init(GameState *g, int n_players, unsigned int seed) {
    assert(n_players >= 2 && n_players <= MAX_PLAYERS);

    memset(g, 0, sizeof(*g));
    g->n_players = n_players;
    g->current = 0;
    g->seed = seed;
    rng_seed(&g->rng, seed);

    /* Topologie (T1) puis génération ressources/jetons (T2), même flux RNG. */
    board_build(&g->board);
    board_generate(&g->board, &g->rng);

    for (int i = 0; i < MAX_VERTICES; i++) {
        g->vertex_building[i] = BUILD_NONE;
        g->vertex_owner[i] = -1;
    }
    for (int i = 0; i < MAX_EDGES; i++) {
        g->edge_owner[i] = -1;
    }
}
