/* sim.h — simulation par lots (T7), façon territoire_sol_risk.c.
 *
 * Joue un grand nombre de parties automatiques sur des graines successives et
 * agrège des statistiques : production totale/moyenne par actif, nombre de
 * parties sans aucune production d'un actif (le fameux ~6,8 % pour SOL, cf.
 * spec §6), distribution des scores. Utile pour valider empiriquement le
 * moteur sans I/O interactive. sim_report() est le seul point d'affichage.
 */
#ifndef SIM_H
#define SIM_H

#include <stdio.h>
#include "types.h"

typedef struct {
    int  games;
    int  turns_per_game;
    int  n_players;
    long production_total[RES_COUNT];   /* production cumulée sur tout le lot */
    int  games_zero[RES_COUNT];         /* parties à production nulle d'un actif */
    long score_sum;                     /* somme de tous les scores finaux */
    int  max_score;                     /* meilleur score final observé */
} SimStats;

/* Joue `games` parties (`turns` tours chacune, `n_players` joueurs), graines
 * base_seed, base_seed+1, … Agrège dans `st`. Sans I/O. */
void sim_run(SimStats *st, int games, int turns, int n_players,
             unsigned int base_seed);

/* Écrit un rapport lisible des statistiques agrégées. */
void sim_report(FILE *out, const SimStats *st);

#endif /* SIM_H */
