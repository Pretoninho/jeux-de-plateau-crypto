/* test_build.c — construction & validation (T4).
 *
 * Les ids d'intersections/arêtes sont dérivés de la topologie réelle, pour ne
 * pas dépendre d'une numérotation particulière.
 */
#include <assert.h>
#include <stdio.h>

#include "../src/board.h"
#include "../src/game.h"
#include "../src/build.h"

/* Coûts attendus (dupliqués ici volontairement, pour recouper build.c). */
static const int C_LINE[RES_COUNT]     = { [RES_SOL] = 1, [RES_ETH] = 1 };
static const int C_POSITION[RES_COUNT] = { [RES_SOL] = 1, [RES_ETH] = 1,
                                           [RES_STABLE] = 1, [RES_ALT] = 1 };
static const int C_DESK[RES_COUNT]     = { [RES_STABLE] = 2, [RES_BTC] = 3 };

static void give(GameState *g, int p, int amount) {
    for (int k = 0; k < RES_COUNT; k++) g->players[p].resources[k] = amount;
}

static void assert_debit(const int before[RES_COUNT], const Player *pl,
                         const int cost[RES_COUNT]) {
    for (int k = 0; k < RES_COUNT; k++) {
        assert(pl->resources[k] == before[k] - cost[k]);
    }
}

static void snapshot(const Player *pl, int out[RES_COUNT]) {
    for (int k = 0; k < RES_COUNT; k++) out[k] = pl->resources[k];
}

int main(void) {
    GameState g;
    game_init(&g, 4, 42u);
    const Board *b = &g.board;

    const int P = 0, Q = 1;

    /* Repères topologiques dérivés du plateau. */
    int v0 = 0;
    const Vertex *V0 = &b->vertices[v0];
    int v_adj = V0->adj[0];                 /* voisine de v0 (règle de distance) */
    int v_far = -1;                         /* non voisine de v0 */
    for (int v = 0; v < b->n_vertices; v++) {
        if (v == v0) continue;
        int is_adj = 0;
        for (int k = 0; k < V0->n_adj; k++) if (V0->adj[k] == v) is_adj = 1;
        if (!is_adj) { v_far = v; break; }
    }
    assert(v_adj >= 0 && v_far >= 0);

    int e_inc = V0->edges[0];               /* arête incidente à v0 */
    int w = (b->edges[e_inc].v[0] == v0) ? b->edges[e_inc].v[1]
                                         : b->edges[e_inc].v[0];

    /* --- Position --------------------------------------------------------- */
    give(&g, P, 5);
    int before[RES_COUNT];
    snapshot(&g.players[P], before);

    /* can_* ne mute pas. */
    assert(can_build_position(&g, P, v0) == BUILD_OK);
    assert(g.vertex_building[v0] == BUILD_NONE);

    assert(build_position(&g, P, v0) == BUILD_OK);
    assert(g.vertex_building[v0] == BUILD_POSITION);
    assert(g.vertex_owner[v0] == P);
    assert(g.players[P].victory_points == 1);
    assert_debit(before, &g.players[P], C_POSITION);

    /* Emplacement déjà pris. */
    assert(build_position(&g, P, v0) == BUILD_ERR_OCCUPIED);

    /* Règle de distance : voisine immédiate interdite, sans débit. */
    snapshot(&g.players[P], before);
    assert(build_position(&g, P, v_adj) == BUILD_ERR_ADJACENT);
    assert(g.vertex_building[v_adj] == BUILD_NONE);
    assert_debit(before, &g.players[P], (const int[RES_COUNT]){ 0 });

    /* Loin de v0 : autorisé. */
    assert(build_position(&g, P, v_far) == BUILD_OK);
    assert(g.players[P].victory_points == 2);

    /* Ressources insuffisantes. */
    give(&g, Q, 0);
    int v_free = -1;
    for (int v = 0; v < b->n_vertices; v++) {
        if (can_build_position(&g, Q, v) == BUILD_ERR_COST) { v_free = v; break; }
    }
    assert(v_free >= 0);   /* un emplacement libre mais impayable existe */

    /* --- Ligne ------------------------------------------------------------ */
    /* Arête loin de tout ouvrage de P (aucune extrémité sur v0 ni v_far). */
    int e_far = -1;
    for (int e = 0; e < b->n_edges; e++) {
        int a = b->edges[e].v[0], c = b->edges[e].v[1];
        if (a != v0 && c != v0 && a != v_far && c != v_far) { e_far = e; break; }
    }
    assert(e_far >= 0);

    /* Non connectée. */
    assert(can_build_line(&g, P, e_far) == BUILD_ERR_CONNECT);

    /* Connectée via la Position en v0. */
    give(&g, P, 5);
    snapshot(&g.players[P], before);
    assert(build_line(&g, P, e_inc) == BUILD_OK);
    assert(g.edge_owner[e_inc] == P);
    assert_debit(before, &g.players[P], C_LINE);

    /* Arête déjà prise. */
    assert(build_line(&g, P, e_inc) == BUILD_ERR_OCCUPIED);

    /* Connexion route→route : arête incidente à w (qui n'a pas d'ouvrage,
     * mais porte désormais une Ligne de P). */
    const Vertex *W = &b->vertices[w];
    int e_next = -1;
    for (int k = 0; k < W->n_edges; k++) {
        if (W->edges[k] != e_inc) { e_next = W->edges[k]; break; }
    }
    assert(e_next >= 0);
    assert(can_build_line(&g, P, e_next) == BUILD_OK);
    assert(build_line(&g, P, e_next) == BUILD_OK);

    /* --- Desk ------------------------------------------------------------- */
    /* Desk sur une intersection vide : refusé. */
    int v_empty = -1;
    for (int v = 0; v < b->n_vertices; v++) {
        if (g.vertex_building[v] == BUILD_NONE) { v_empty = v; break; }
    }
    assert(v_empty >= 0);
    give(&g, P, 5);
    assert(build_desk(&g, P, v_empty) == BUILD_ERR_NOT_POSITION);

    /* Desk sur la Position d'un autre joueur : refusé. */
    assert(build_desk(&g, Q, v0) == BUILD_ERR_PLAYER);

    /* Upgrade de sa propre Position en v0. */
    snapshot(&g.players[P], before);
    assert(build_desk(&g, P, v0) == BUILD_OK);
    assert(g.vertex_building[v0] == BUILD_DESK);
    assert(g.vertex_owner[v0] == P);
    assert(g.players[P].victory_points == 3);   /* 2 Positions (2) + upgrade (+1) */
    assert_debit(before, &g.players[P], C_DESK);

    printf("OK construction : Position (distance/cout/occupe), Ligne (connexion),\n");
    printf("  Desk (upgrade), debits et points de victoire coherents.\n");
    return 0;
}
