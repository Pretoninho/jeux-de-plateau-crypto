/* turn.c — boucle de tour et production (cf. turn.h). */
#include "turn.h"

#include <assert.h>

int roll_2d6(Rng *r) {
    int d1 = rng_range(r, 6) + 1;
    int d2 = rng_range(r, 6) + 1;
    return d1 + d2;
}

void game_produce(GameState *g, int roll) {
    if (roll == 7) {
        return;   /* Margin Call : hors scope Phase 1, aucun effet */
    }

    Board *b = &g->board;
    for (int i = 0; i < b->n_tiles; i++) {
        const Tile *t = &b->tiles[i];
        if (t->resource == RES_NONE || t->number != roll) {
            continue;   /* case Rekt ou numéro non concerné */
        }
        /* Chaque construction adjacente à la tuile produit. */
        for (int d = 0; d < 6; d++) {
            int v = t->vertices[d];
            Building bd = g->vertex_building[v];
            if (bd == BUILD_NONE) {
                continue;
            }
            int owner = g->vertex_owner[v];
            assert(owner >= 0 && owner < g->n_players);
            int units = (bd == BUILD_DESK) ? 2 : 1;
            g->players[owner].resources[t->resource] += units;
        }
    }
}

int game_turn(GameState *g) {
    int roll = roll_2d6(&g->rng);
    game_produce(g, roll);
    g->current = (g->current + 1) % g->n_players;
    return roll;
}
