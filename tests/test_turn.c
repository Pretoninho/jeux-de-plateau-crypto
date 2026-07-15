/* test_turn.c — boucle de tour et production (T3).
 *
 * Stratégie : la production (parcours tuile → intersections) est recoupée par
 * un calcul indépendant (parcours intersection → tuiles), ce qui valide aussi
 * la cohérence inverse des adjacences T1. Plus une simulation par lots (DoD).
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/board.h"
#include "../src/game.h"
#include "../src/turn.h"

/* Production attendue, calculée côté intersections (dual de game_produce). */
static void expected_production(const GameState *g, int roll,
                                long exp[MAX_PLAYERS][RES_COUNT]) {
    for (int p = 0; p < MAX_PLAYERS; p++) {
        for (int k = 0; k < RES_COUNT; k++) exp[p][k] = 0;
    }
    if (roll == 7) return;

    const Board *b = &g->board;
    for (int v = 0; v < b->n_vertices; v++) {
        Building bd = g->vertex_building[v];
        if (bd == BUILD_NONE) continue;
        int o = g->vertex_owner[v];
        int units = (bd == BUILD_DESK) ? 2 : 1;
        const Vertex *vt = &b->vertices[v];
        for (int k = 0; k < vt->n_tiles; k++) {
            const Tile *t = &b->tiles[vt->tiles[k]];
            if (t->resource != RES_NONE && t->number == roll) {
                exp[o][t->resource] += units;
            }
        }
    }
}

static void zero_resources(GameState *g) {
    for (int p = 0; p < MAX_PLAYERS; p++) {
        for (int k = 0; k < RES_COUNT; k++) g->players[p].resources[k] = 0;
    }
}

/* Place un jeu de constructions fixe (ids arbitraires : validation = T4). */
static void place_fixture(GameState *g) {
    g->vertex_building[5]  = BUILD_POSITION; g->vertex_owner[5]  = 0;
    g->vertex_building[10] = BUILD_POSITION; g->vertex_owner[10] = 1;
    g->vertex_building[20] = BUILD_DESK;     g->vertex_owner[20] = 2;
    g->vertex_building[33] = BUILD_POSITION; g->vertex_owner[33] = 3;
    g->vertex_building[40] = BUILD_DESK;     g->vertex_owner[40] = 0;
}

int main(void) {
    /* 1) 2d6 : bornes [2,12], toutes les sommes apparaissent, 7 domine. */
    Rng r;
    rng_seed(&r, 1u);
    int seen[13] = { 0 };
    for (int i = 0; i < 40000; i++) {
        int s = roll_2d6(&r);
        assert(s >= 2 && s <= 12);
        seen[s]++;
    }
    for (int v = 2; v <= 12; v++) assert(seen[v] > 0);
    assert(seen[7] > seen[2] && seen[7] > seen[12]);   /* forme triangulaire */

    /* 2) Production recoupée par le calcul dual, pour chaque somme. */
    GameState g;
    game_init(&g, 4, 42u, 42u);
    place_fixture(&g);
    for (int roll = 2; roll <= 12; roll++) {
        zero_resources(&g);
        game_produce(&g, roll);
        long exp[MAX_PLAYERS][RES_COUNT];
        expected_production(&g, roll, exp);
        for (int p = 0; p < g.n_players; p++) {
            for (int k = 0; k < RES_COUNT; k++) {
                assert((long)g.players[p].resources[k] == exp[p][k]);
            }
        }
    }

    /* 3) Somme = 7 : aucun effet. */
    zero_resources(&g);
    game_produce(&g, 7);
    for (int p = 0; p < g.n_players; p++) {
        for (int k = 0; k < RES_COUNT; k++) assert(g.players[p].resources[k] == 0);
    }

    /* 4) Desk = 2 × Position : mêmes adjacences, production exactement doublée. */
    GameState h;
    game_init(&h, 2, 7u, 7u);
    const int V = 12;
    long total_pos = 0, total_desk = 0;
    h.vertex_owner[V] = 0;
    h.vertex_building[V] = BUILD_POSITION;
    for (int roll = 2; roll <= 12; roll++) {
        zero_resources(&h);
        game_produce(&h, roll);
        for (int k = 0; k < RES_COUNT; k++) total_pos += h.players[0].resources[k];
    }
    h.vertex_building[V] = BUILD_DESK;
    for (int roll = 2; roll <= 12; roll++) {
        zero_resources(&h);
        game_produce(&h, roll);
        for (int k = 0; k < RES_COUNT; k++) total_desk += h.players[0].resources[k];
    }
    assert(total_desk == 2 * total_pos);

    /* 5) Reproductibilité + simulation par lots (DoD) : 2000 tours, sans crash. */
    GameState a, b2;
    game_init(&a, 4, 555u, 555u);
    game_init(&b2, 4, 555u, 555u);
    place_fixture(&a);
    place_fixture(&b2);
    for (int turn = 0; turn < 2000; turn++) {
        int ra = game_turn(&a);
        int rb = game_turn(&b2);
        assert(ra == rb);
        assert(a.current >= 0 && a.current < a.n_players);
    }
    for (int p = 0; p < a.n_players; p++) {
        for (int k = 0; k < RES_COUNT; k++) {
            assert(a.players[p].resources[k] == b2.players[p].resources[k]);
            assert(a.players[p].resources[k] >= 0);   /* jamais négatif */
        }
    }

    /* Production non nulle globalement (le fixture touche des tuiles ressource). */
    long grand_total = 0;
    for (int p = 0; p < a.n_players; p++) {
        for (int k = 0; k < RES_COUNT; k++) grand_total += a.players[p].resources[k];
    }
    assert(grand_total > 0);

    printf("OK tour/production : 2d6, production (dual), 7 sans effet,\n");
    printf("  Desk=2xPosition, reproductibilite sur 2000 tours (%ld ressources).\n",
           grand_total);
    return 0;
}
