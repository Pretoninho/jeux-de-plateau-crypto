/* game.h — état de jeu (squelette Phase 1).
 *
 * Regroupe le plateau, les joueurs et l'occupation des intersections/arêtes.
 * Tout est à taille fixe (D4). La boucle de tour, la production et la
 * construction viendront dans T3/T4 ; ici on pose seulement les structures
 * et l'initialisation. Moteur pur (D2) : aucune I/O.
 */
#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "rng.h"

typedef struct {
    int resources[RES_COUNT];   /* stock par actif */
    int victory_points;
} Player;

typedef struct {
    Board  board;
    Player players[MAX_PLAYERS];
    int    n_players;
    int    current;             /* index du joueur actif */
    unsigned int seed;          /* graine RNG (D5) */
    Rng    rng;                 /* état du générateur (génération, puis dés) */

    /* Occupation du plateau. */
    Building vertex_building[MAX_VERTICES];  /* NONE / POSITION / DESK */
    int      vertex_owner[MAX_VERTICES];     /* id joueur, -1 si libre */
    int      edge_owner[MAX_EDGES];          /* Ligne : id joueur, -1 si libre */
} GameState;

/* Initialise une partie : plateau construit, joueurs et plateau vierges.
 * n_players attendu dans [2, MAX_PLAYERS] (D9). */
void game_init(GameState *g, int n_players, unsigned int seed);

#endif /* GAME_H */
