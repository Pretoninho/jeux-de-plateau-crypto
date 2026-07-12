/* setup.h — génération de plateau (T2) : ressources + jetons numériques.
 *
 * Distinct de la topologie (board_build) : ici on remplit les champs
 * `resource` et `number` d'un plateau déjà construit, de façon aléatoire
 * mais reproductible (RNG seedé, D7). Moteur pur (D2), sans I/O.
 */
#ifndef SETUP_H
#define SETUP_H

#include "board.h"
#include "rng.h"

/* Assigne aux 19 tuiles :
 *   - la distribution de ressources (BTC 7, Alts 4, ETH 3, Stable 3, SOL 1)
 *     + 1 case Rekt (RES_NONE) placée aléatoirement ;
 *   - les 18 jetons numériques (2..12 sauf 7) sur les tuiles ressource,
 *     la case Rekt recevant le jeton 0 (aucune production).
 * Suppose un plateau déjà construit par board_build(). */
void board_generate(Board *b, Rng *rng);

#endif /* SETUP_H */
