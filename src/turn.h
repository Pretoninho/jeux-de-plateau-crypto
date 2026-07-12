/* turn.h — boucle de tour et production (T3).
 *
 * Un tour = lancer 2d6 → production → (construction : T4) → joueur suivant.
 * Production : pour une somme ≠ 7, chaque tuile portant ce numéro verse son
 * actif aux Positions (1) / Desks (2) adjacents. Somme = 7 → tour sans effet
 * (Margin Call hors scope Phase 1). Moteur pur (D2), sans I/O.
 */
#ifndef TURN_H
#define TURN_H

#include "game.h"

/* Lance 2d6 sur le flux RNG fourni. Retour dans [2, 12]. RNG isolé (D5). */
int roll_2d6(Rng *r);

/* Applique la production d'un lancer donné (no-op si roll == 7). */
void game_produce(GameState *g, int roll);

/* Joue un tour complet : lancer + production + passage au joueur suivant.
 * Retourne la somme des dés. La construction (T4) n'est pas encore branchée. */
int game_turn(GameState *g);

#endif /* TURN_H */
