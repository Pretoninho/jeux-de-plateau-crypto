/* sim.c — simulation par lots (cf. sim.h). */
#include "sim.h"

#include <string.h>

#include "game.h"
#include "turn.h"
#include "build.h"
#include "score.h"

static const char *res_name(Resource r) {
    switch (r) {
        case RES_BTC:    return "BTC";
        case RES_ETH:    return "ETH";
        case RES_ALT:    return "Alt";
        case RES_STABLE: return "Stable";
        case RES_SOL:    return "SOL";
        default:         return "Rekt";
    }
}

/* Somme d'un actif sur tous les joueurs. */
static long total_res(const GameState *g, Resource r) {
    long s = 0;
    for (int p = 0; p < g->n_players; p++) s += g->players[p].resources[r];
    return s;
}

/* Bot minimal (même politique que la démo) : Position → Desk → Ligne. */
static void auto_build(GameState *g, int p) {
    for (int v = 0; v < g->board.n_vertices; v++) {
        if (can_build_position(g, p, v) == BUILD_OK) { build_position(g, p, v); return; }
    }
    for (int v = 0; v < g->board.n_vertices; v++) {
        if (can_build_desk(g, p, v) == BUILD_OK) { build_desk(g, p, v); return; }
    }
    for (int e = 0; e < g->board.n_edges; e++) {
        if (can_build_line(g, p, e) == BUILD_OK) { build_line(g, p, e); return; }
    }
}

/* Joue une partie ; `prod` reçoit la production cumulée par actif. */
static void play_one(GameState *g, int turns, long prod[RES_COUNT]) {
    for (int k = 0; k < RES_COUNT; k++) prod[k] = 0;

    for (int t = 0; t < turns; t++) {
        int p = g->current;

        long before[RES_COUNT];
        for (int k = 0; k < RES_COUNT; k++) before[k] = total_res(g, (Resource)k);

        int d = roll_2d6(&g->rng);
        game_produce(g, d);

        /* Production du tour = delta mesuré AVANT toute dépense de construction. */
        for (int k = 0; k < RES_COUNT; k++) {
            prod[k] += total_res(g, (Resource)k) - before[k];
        }

        auto_build(g, p);
        g->current = (p + 1) % g->n_players;
    }
}

void sim_run(SimStats *st, int games, int turns, int n_players,
             unsigned int base_seed) {
    memset(st, 0, sizeof(*st));
    st->games = games;
    st->turns_per_game = turns;
    st->n_players = n_players;

    for (int i = 0; i < games; i++) {
        GameState g;
        /* Simulation reproductible : board et dés déterministes (même graine
         * dérivée), sur deux flux indépendants. */
        unsigned int s = base_seed + (unsigned int)i;
        game_init(&g, n_players, s, s);
        game_place_initial(&g, INITIAL_POSITIONS);

        long prod[RES_COUNT];
        play_one(&g, turns, prod);

        for (int k = 0; k < RES_COUNT; k++) {
            st->production_total[k] += prod[k];
            if (prod[k] == 0) st->games_zero[k]++;
        }
        for (int p = 0; p < n_players; p++) {
            int s = game_score(&g, p);
            st->score_sum += s;
            if (s > st->max_score) st->max_score = s;
        }
    }
}

void sim_report(FILE *out, const SimStats *st) {
    fprintf(out, "=== Simulation : %d parties x %d tours, %d joueurs ===\n",
            st->games, st->turns_per_game, st->n_players);
    fprintf(out, " actif   | prod. totale | moy./partie | parties sans prod.\n");
    for (int k = RES_BTC; k < RES_COUNT; k++) {
        double mean = st->games ? (double)st->production_total[k] / st->games : 0.0;
        double zero = st->games ? 100.0 * st->games_zero[k] / st->games : 0.0;
        fprintf(out, " %-7s | %12ld | %11.1f | %5d (%.1f%%)\n",
                res_name((Resource)k), st->production_total[k], mean,
                st->games_zero[k], zero);
    }
    long players_total = (long)st->games * st->n_players;
    double mean_score = players_total ? (double)st->score_sum / players_total : 0.0;
    fprintf(out, " score moyen / joueur : %.2f | meilleur score : %d\n",
            mean_score, st->max_score);
}
