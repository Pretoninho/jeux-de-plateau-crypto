/* test_sim.c — simulation par lots (T7).
 *
 * Vérifie les invariantes d'agrégation et la reproductibilité, pas une valeur
 * exacte (le modèle diffère de territoire_sol_risk.c). Le taux « sans SOL »
 * est affiché à titre indicatif.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../src/types.h"
#include "../src/sim.h"

int main(void) {
    const int GAMES = 300, TURNS = 80, PLAYERS = 4;

    SimStats a;
    sim_run(&a, GAMES, TURNS, PLAYERS, 1u);

    /* Rekt ne produit jamais : production nulle dans TOUTES les parties. */
    assert(a.production_total[RES_NONE] == 0);
    assert(a.games_zero[RES_NONE] == GAMES);

    /* BTC (7 cases) : produit dans quasiment toutes les parties. */
    assert(a.games_zero[RES_BTC] < GAMES / 20);
    assert(a.production_total[RES_BTC] > 0);

    /* Chaque taux « sans production » est une proportion valide. */
    for (int k = RES_BTC; k < RES_COUNT; k++) {
        assert(a.games_zero[k] >= 0 && a.games_zero[k] <= GAMES);
    }

    /* SOL (1 case) : plus souvent absent que BTC (rareté attendue). */
    assert(a.games_zero[RES_SOL] >= a.games_zero[RES_BTC]);

    /* Score : chaque joueur a au moins ses Positions initiales. */
    long min_expected = (long)GAMES * PLAYERS * INITIAL_POSITIONS;
    assert(a.score_sum >= min_expected);
    assert(a.max_score >= INITIAL_POSITIONS);

    /* Reproductibilité : mêmes paramètres → mêmes statistiques. */
    SimStats b;
    sim_run(&b, GAMES, TURNS, PLAYERS, 1u);
    assert(a.score_sum == b.score_sum);
    assert(a.max_score == b.max_score);
    for (int k = 0; k < RES_COUNT; k++) {
        assert(a.production_total[k] == b.production_total[k]);
        assert(a.games_zero[k] == b.games_zero[k]);
    }

    /* Graine de base différente → statistiques (très probablement) différentes. */
    SimStats c;
    sim_run(&c, GAMES, TURNS, PLAYERS, 5000u);
    int differ = (c.score_sum != a.score_sum);
    for (int k = 0; k < RES_COUNT; k++) {
        if (c.production_total[k] != a.production_total[k]) differ = 1;
    }
    assert(differ);

    /* Le rapport écrit quelque chose de cohérent. */
    FILE *f = tmpfile();
    assert(f);
    sim_report(f, &a);
    fflush(f);
    assert(ftell(f) > 0);
    fclose(f);

    double sol_rate = 100.0 * a.games_zero[RES_SOL] / GAMES;
    printf("OK simulation : invariantes, reproductibilite, effet de graine.\n");
    printf("  parties sans production de SOL : %d/%d (%.1f%%)\n",
           a.games_zero[RES_SOL], GAMES, sol_rate);
    return 0;
}
