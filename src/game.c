/* game.c — initialisation de l'état de jeu (cf. game.h). */
#include "game.h"
#include "setup.h"
#include "build.h"

#include <assert.h>
#include <string.h>

void game_init(GameState *g, int n_players,
               unsigned int board_seed, unsigned int dice_seed) {
    assert(n_players >= 2 && n_players <= MAX_PLAYERS);

    memset(g, 0, sizeof(*g));
    g->n_players = n_players;
    g->current = 0;
    g->seed = board_seed;
    g->dice_seed = dice_seed;

    /* Génération du plateau sur un flux LOCAL, jetable : elle ne touche pas au
     * flux des dés (la production du plateau ne script pas la partie). */
    Rng board_rng;
    rng_seed(&board_rng, board_seed);
    board_build(&g->board);
    board_generate(&g->board, &board_rng);

    /* Flux des dés : indépendant, seedé séparément. */
    rng_seed(&g->rng, dice_seed);

    for (int i = 0; i < MAX_VERTICES; i++) {
        g->vertex_building[i] = BUILD_NONE;
        g->vertex_owner[i] = -1;
    }
    for (int i = 0; i < MAX_EDGES; i++) {
        g->edge_owner[i] = -1;
    }
}

void game_place_initial(GameState *g, int per_player) {
    int nv = g->board.n_vertices;
    int total = g->n_players * per_player;
    int step = (total > 0) ? nv / total : 1;
    if (step < 1) step = 1;

    /* Placement réparti (curseur balayant le plateau) pour diversifier les
     * ressources, et en alternance entre joueurs pour l'équité. */
    int cursor = 0;
    for (int round = 0; round < per_player; round++) {
        for (int p = 0; p < g->n_players; p++) {
            for (int s = 0; s < nv; s++) {
                int v = (cursor + s) % nv;
                if (place_position_free(g, p, v) == BUILD_OK) {
                    cursor = (v + step) % nv;
                    break;
                }
            }
        }
    }
}
