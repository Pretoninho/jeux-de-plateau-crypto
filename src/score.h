/* score.h — valorisation / points de victoire (T5).
 *
 * Position = 1 point, Desk = 2 points (cf. spec). Le score est ici *recalculé*
 * depuis l'état du plateau, indépendamment du compteur `victory_points`
 * maintenu à la construction — les deux doivent coïncider (recoupement).
 *
 * En Phase 1, on NE bloque PAS le moteur sur l'atteinte de 10 (pas de trading) :
 * ces fonctions servent à logger/afficher le score, la condition de victoire
 * réelle est différée en Phase 2. Moteur pur (D2), sans I/O.
 */
#ifndef SCORE_H
#define SCORE_H

#include "game.h"

/* Score d'un joueur, recalculé depuis le plateau. */
int game_score(const GameState *g, int player);

/* Joueur en tête (score max). Égalité → plus petit indice. */
int game_leader(const GameState *g);

#endif /* SCORE_H */
