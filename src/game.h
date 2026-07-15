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
    unsigned int seed;          /* graine du PLATEAU (reproductible) */
    unsigned int dice_seed;     /* graine des DÉS (flux indépendant, non scripté) */
    Rng    rng;                 /* flux RNG des DÉS uniquement (la génération du
                                 * plateau utilise un flux local, jetable) */

    /* Occupation du plateau. */
    Building vertex_building[MAX_VERTICES];  /* NONE / POSITION / DESK */
    int      vertex_owner[MAX_VERTICES];     /* id joueur, -1 si libre */
    int      edge_owner[MAX_EDGES];          /* Ligne : id joueur, -1 si libre */
} GameState;

/* Initialise une partie : plateau construit, joueurs et plateau vierges.
 * n_players attendu dans [2, MAX_PLAYERS] (D9).
 *
 * DEUX graines indépendantes (la génération du plateau ne script PAS la partie) :
 *   - board_seed : détermine le plateau (reproductible) ;
 *   - dice_seed  : détermine les dés, sur un flux séparé.
 * En vraie partie, l'appelant passe une dice_seed issue d'entropie (partie NON
 * scriptée) ; en test/simulation, une valeur fixe (reproductible). Le moteur
 * reste pur : il ne lit aucune source d'entropie lui-même. */
void game_init(GameState *g, int n_players,
               unsigned int board_seed, unsigned int dice_seed);

/* Mise en place : pose `per_player` Positions gratuites par joueur (respecte la
 * règle de distance, ignore le coût), pour amorcer la production. À appeler
 * après game_init(). Le placement est ici automatique (choix interactif : plus
 * tard). Sans cet amorçage, aucune production ne peut démarrer. */
void game_place_initial(GameState *g, int per_player);

#endif /* GAME_H */
