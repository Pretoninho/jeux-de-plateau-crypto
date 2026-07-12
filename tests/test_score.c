/* test_score.c — valorisation (T5) : score recalculé, cohérence avec le
 * compteur maintenu à la construction, classement.
 */
#include <assert.h>
#include <stdio.h>

#include "../src/board.h"
#include "../src/game.h"
#include "../src/build.h"
#include "../src/score.h"

static void give(GameState *g, int p, int amount) {
    for (int k = 0; k < RES_COUNT; k++) g->players[p].resources[k] = amount;
}

/* Construit une Position sur le premier emplacement valide ; retourne l'id. */
static int build_one_position(GameState *g, int p) {
    give(g, p, 10);
    for (int v = 0; v < g->board.n_vertices; v++) {
        if (can_build_position(g, p, v) == BUILD_OK) {
            build_position(g, p, v);
            return v;
        }
    }
    return -1;
}

int main(void) {
    GameState g;
    game_init(&g, 4, 42u);

    /* Scénario : p0 = 2 Positions dont 1 upgradée en Desk ; p1 = 1 Position. */
    int a = build_one_position(&g, 0);
    int b = build_one_position(&g, 0);
    assert(a >= 0 && b >= 0);
    give(&g, 0, 10);
    assert(build_desk(&g, 0, b) == BUILD_OK);

    int c = build_one_position(&g, 1);
    assert(c >= 0);

    /* Score recalculé depuis le plateau. */
    assert(game_score(&g, 0) == 3);   /* Position (1) + Desk (2) */
    assert(game_score(&g, 1) == 1);
    assert(game_score(&g, 2) == 0);
    assert(game_score(&g, 3) == 0);

    /* Cohérence avec le compteur maintenu à la construction. */
    for (int p = 0; p < g.n_players; p++) {
        assert(game_score(&g, p) == g.players[p].victory_points);
    }

    /* Classement : p0 en tête. */
    assert(game_leader(&g) == 0);

    /* Égalité → plus petit indice. Constructions posées directement. */
    GameState h;
    game_init(&h, 2, 1u);
    int v0 = 0;
    const Vertex *V0 = &h.board.vertices[v0];
    int far = -1;
    for (int v = 0; v < h.board.n_vertices; v++) {
        if (v == v0) continue;
        int is_adj = 0;
        for (int k = 0; k < V0->n_adj; k++) if (V0->adj[k] == v) is_adj = 1;
        if (!is_adj) { far = v; break; }
    }
    assert(far >= 0);
    h.vertex_building[v0]  = BUILD_POSITION; h.vertex_owner[v0]  = 0;
    h.vertex_building[far] = BUILD_POSITION; h.vertex_owner[far] = 1;
    assert(game_score(&h, 0) == 1 && game_score(&h, 1) == 1);
    assert(game_leader(&h) == 0);   /* égalité → indice 0 */

    /* Le moteur ne bloque pas au-delà de 10 : score simplement cumulable. */
    GameState big;
    game_init(&big, 2, 2u);
    int placed = 0;
    for (int v = 0; v < big.board.n_vertices && placed < 12; v++) {
        give(&big, 0, 10);
        if (build_position(&big, 0, v) == BUILD_OK) placed++;
    }
    assert(game_score(&big, 0) == placed);
    assert(placed >= 10);   /* dépasse 10 sans que le moteur s'y oppose */

    printf("OK score : Position=1/Desk=2, coherence compteur, classement,\n");
    printf("  pas de blocage au-dela de 10 (score=%d).\n", game_score(&big, 0));
    return 0;
}
