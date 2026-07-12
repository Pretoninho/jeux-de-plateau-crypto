/* score.c — valorisation / points de victoire (cf. score.h). */
#include "score.h"

int game_score(const GameState *g, int player) {
    int s = 0;
    for (int v = 0; v < g->board.n_vertices; v++) {
        if (g->vertex_owner[v] != player) continue;
        if (g->vertex_building[v] == BUILD_POSITION) {
            s += 1;
        } else if (g->vertex_building[v] == BUILD_DESK) {
            s += 2;
        }
    }
    return s;
}

int game_leader(const GameState *g) {
    int best = 0;
    int best_score = game_score(g, 0);
    for (int p = 1; p < g->n_players; p++) {
        int s = game_score(g, p);
        if (s > best_score) {   /* strict → l'égalité garde le plus petit indice */
            best_score = s;
            best = p;
        }
    }
    return best;
}
