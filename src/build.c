/* build.c — construction et validation des règles (cf. build.h). */
#include "build.h"

#include <assert.h>

/* Coûts par actif (initialiseurs désignés C99, cf. spec). */
static const int COST_LINE[RES_COUNT]     = { [RES_SOL] = 1, [RES_ETH] = 1 };
static const int COST_POSITION[RES_COUNT] = { [RES_SOL] = 1, [RES_ETH] = 1,
                                              [RES_STABLE] = 1, [RES_ALT] = 1 };
static const int COST_DESK[RES_COUNT]     = { [RES_STABLE] = 2, [RES_BTC] = 3 };

/* --- ressources ----------------------------------------------------------- */

static int has_resources(const Player *pl, const int *cost) {
    for (int k = 0; k < RES_COUNT; k++) {
        if (pl->resources[k] < cost[k]) return 0;
    }
    return 1;
}

static void pay_resources(Player *pl, const int *cost) {
    for (int k = 0; k < RES_COUNT; k++) {
        pl->resources[k] -= cost[k];
    }
}

/* --- helpers de validation ------------------------------------------------ */

static int valid_player(const GameState *g, int player) {
    return player >= 0 && player < g->n_players;
}

/* Une Ligne est connectée si une extrémité porte une construction du joueur,
 * ou si une arête adjacente (partageant une extrémité) lui appartient. */
static int line_connected(const GameState *g, int player, int edge) {
    const Edge *ed = &g->board.edges[edge];
    for (int s = 0; s < 2; s++) {
        int v = ed->v[s];
        if (g->vertex_building[v] != BUILD_NONE && g->vertex_owner[v] == player) {
            return 1;
        }
        const Vertex *vt = &g->board.vertices[v];
        for (int k = 0; k < vt->n_edges; k++) {
            int e2 = vt->edges[k];
            if (e2 != edge && g->edge_owner[e2] == player) return 1;
        }
    }
    return 0;
}

/* --- validation ----------------------------------------------------------- */

/* Validité d'emplacement d'une Position, hors coût (libre + règle de distance).
 * Sert au placement initial gratuit et de socle à can_build_position. */
BuildResult can_place_position_free(const GameState *g, int player, int vertex) {
    if (!valid_player(g, player)) return BUILD_ERR_PLAYER;
    if (vertex < 0 || vertex >= g->board.n_vertices) return BUILD_ERR_BOUNDS;
    if (g->vertex_building[vertex] != BUILD_NONE) return BUILD_ERR_OCCUPIED;

    /* Règle de distance : aucune intersection voisine occupée. */
    const Vertex *vt = &g->board.vertices[vertex];
    for (int k = 0; k < vt->n_adj; k++) {
        if (g->vertex_building[vt->adj[k]] != BUILD_NONE) return BUILD_ERR_ADJACENT;
    }
    return BUILD_OK;
}

BuildResult can_build_position(const GameState *g, int player, int vertex) {
    BuildResult r = can_place_position_free(g, player, vertex);
    if (r != BUILD_OK) return r;
    if (!has_resources(&g->players[player], COST_POSITION)) return BUILD_ERR_COST;
    return BUILD_OK;
}

BuildResult can_build_desk(const GameState *g, int player, int vertex) {
    if (!valid_player(g, player)) return BUILD_ERR_PLAYER;
    if (vertex < 0 || vertex >= g->board.n_vertices) return BUILD_ERR_BOUNDS;
    if (g->vertex_building[vertex] != BUILD_POSITION) return BUILD_ERR_NOT_POSITION;
    if (g->vertex_owner[vertex] != player) return BUILD_ERR_PLAYER;
    if (!has_resources(&g->players[player], COST_DESK)) return BUILD_ERR_COST;
    return BUILD_OK;
}

BuildResult can_build_line(const GameState *g, int player, int edge) {
    if (!valid_player(g, player)) return BUILD_ERR_PLAYER;
    if (edge < 0 || edge >= g->board.n_edges) return BUILD_ERR_BOUNDS;
    if (g->edge_owner[edge] != -1) return BUILD_ERR_OCCUPIED;
    if (!line_connected(g, player, edge)) return BUILD_ERR_CONNECT;
    if (!has_resources(&g->players[player], COST_LINE)) return BUILD_ERR_COST;
    return BUILD_OK;
}

/* --- construction --------------------------------------------------------- */

BuildResult place_position_free(GameState *g, int player, int vertex) {
    BuildResult r = can_place_position_free(g, player, vertex);
    if (r != BUILD_OK) return r;

    g->vertex_building[vertex] = BUILD_POSITION;   /* gratuit : aucun débit */
    g->vertex_owner[vertex] = player;
    g->players[player].victory_points += 1;
    return BUILD_OK;
}

BuildResult build_position(GameState *g, int player, int vertex) {
    BuildResult r = can_build_position(g, player, vertex);
    if (r != BUILD_OK) return r;

    pay_resources(&g->players[player], COST_POSITION);
    g->vertex_building[vertex] = BUILD_POSITION;
    g->vertex_owner[vertex] = player;
    g->players[player].victory_points += 1;
    return BUILD_OK;
}

BuildResult build_desk(GameState *g, int player, int vertex) {
    BuildResult r = can_build_desk(g, player, vertex);
    if (r != BUILD_OK) return r;

    pay_resources(&g->players[player], COST_DESK);
    g->vertex_building[vertex] = BUILD_DESK;   /* Position → Desk */
    g->players[player].victory_points += 1;    /* 1 pt → 2 pts */
    return BUILD_OK;
}

BuildResult build_line(GameState *g, int player, int edge) {
    BuildResult r = can_build_line(g, player, edge);
    if (r != BUILD_OK) return r;

    pay_resources(&g->players[player], COST_LINE);
    g->edge_owner[edge] = player;
    return BUILD_OK;
}
