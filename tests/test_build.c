/* test_build.c — construction & validation (T4 + règle A).
 *
 * Règle A : une Position construite en jeu doit être reliée à une Ligne du
 * joueur ; le placement initial gratuit (place_position_free) en est exempt.
 * Les ids d'intersections/arêtes sont dérivés de la topologie réelle.
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
    game_init(&g, 4, 42u, 42u);
    const Board *b = &g.board;

    const int P = 0, Q = 1;

    /* Repères topologiques. */
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

    /* --- Mise en place : Position initiale gratuite (exempte de connexion) --- */
    int before[RES_COUNT];
    assert(place_position_free(&g, P, v0) == BUILD_OK);
    assert(g.vertex_building[v0] == BUILD_POSITION);
    assert(g.vertex_owner[v0] == P);
    assert(g.players[P].victory_points == 1);

    /* --- Position en jeu : distance + connexion --------------------------- */
    give(&g, P, 5);

    /* Règle de distance : voisine immédiate interdite. */
    assert(can_build_position(&g, P, v_adj) == BUILD_ERR_ADJACENT);

    /* Règle A : loin de v0, distance OK mais AUCUNE Ligne → refus, sans effet. */
    snapshot(&g.players[P], before);
    assert(can_build_position(&g, P, v_far) == BUILD_ERR_CONNECT);
    assert(build_position(&g, P, v_far) == BUILD_ERR_CONNECT);
    assert(g.vertex_building[v_far] == BUILD_NONE);
    assert_debit(before, &g.players[P], (const int[RES_COUNT]){ 0 });

    /* --- Ligne ------------------------------------------------------------ */
    /* Arête sans lien avec P (aucune extrémité sur v0). */
    int e_far = -1;
    for (int e = 0; e < b->n_edges; e++) {
        if (b->edges[e].v[0] != v0 && b->edges[e].v[1] != v0) { e_far = e; break; }
    }
    assert(e_far >= 0);
    assert(can_build_line(&g, P, e_far) == BUILD_ERR_CONNECT);

    /* Connectée via la Position en v0. */
    give(&g, P, 5);
    snapshot(&g.players[P], before);
    assert(build_line(&g, P, e_inc) == BUILD_OK);   /* v0 — w */
    assert(g.edge_owner[e_inc] == P);
    assert_debit(before, &g.players[P], C_LINE);

    /* Arête déjà prise. */
    assert(build_line(&g, P, e_inc) == BUILD_ERR_OCCUPIED);

    /* Connexion route→route : arête incidente à w (sans ouvrage, mais route de P). */
    const Vertex *W = &b->vertices[w];
    int e_next = -1;
    for (int k = 0; k < W->n_edges; k++) {
        if (W->edges[k] != e_inc) { e_next = W->edges[k]; break; }
    }
    assert(e_next >= 0);
    give(&g, P, 5);
    assert(build_line(&g, P, e_next) == BUILD_OK);   /* w — x */

    /* --- Position reliée au réseau --------------------------------------- */
    int x = (b->edges[e_next].v[0] == w) ? b->edges[e_next].v[1]
                                         : b->edges[e_next].v[0];
    assert(x != v0 && g.vertex_building[x] == BUILD_NONE);

    /* Reliée (arête e_next) et distance OK, mais ressources nulles → COST. */
    give(&g, P, 0);
    assert(can_build_position(&g, P, x) == BUILD_ERR_COST);

    /* Avec ressources : construite. */
    give(&g, P, 5);
    snapshot(&g.players[P], before);
    assert(build_position(&g, P, x) == BUILD_OK);
    assert(g.vertex_building[x] == BUILD_POSITION);
    assert(g.players[P].victory_points == 2);
    assert_debit(before, &g.players[P], C_POSITION);

    /* --- Desk ------------------------------------------------------------- */
    int v_empty = -1;
    for (int v = 0; v < b->n_vertices; v++) {
        if (g.vertex_building[v] == BUILD_NONE) { v_empty = v; break; }
    }
    assert(v_empty >= 0);
    give(&g, P, 5);
    assert(build_desk(&g, P, v_empty) == BUILD_ERR_NOT_POSITION);   /* vide */
    assert(build_desk(&g, Q, v0) == BUILD_ERR_PLAYER);              /* pas la sienne */

    snapshot(&g.players[P], before);
    assert(build_desk(&g, P, v0) == BUILD_OK);                     /* upgrade */
    assert(g.vertex_building[v0] == BUILD_DESK);
    assert(g.vertex_owner[v0] == P);
    assert(g.players[P].victory_points == 3);   /* 2 Positions (2) + upgrade (+1) */
    assert_debit(before, &g.players[P], C_DESK);

    printf("OK construction : Position (distance/connexion/cout), Ligne,\n");
    printf("  route->route, Desk (upgrade), debits et points coherents.\n");
    return 0;
}
