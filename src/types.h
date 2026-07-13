/* types.h — constantes et énumérations partagées du moteur.
 *
 * État borné (D4) : toutes les tailles max sont fixées ici, pas d'allocation
 * dynamique. Rayon 2 → 19 tuiles, 54 intersections, 72 arêtes (plateau hexagonal).
 */
#ifndef TYPES_H
#define TYPES_H

#define BOARD_RADIUS 2
#define MAX_TILES    19
#define MAX_VERTICES 54
#define MAX_EDGES    72
#define MAX_PLAYERS  4

/* Positions gratuites posées à la mise en place (amorçage de la production,
 * 2 par joueur par défaut). Sans elles, aucune production ne démarre. */
#define INITIAL_POSITIONS 2

/* Actifs crypto (cf. spec §Plateau). RES_NONE = case Rekt / non assigné. */
typedef enum {
    RES_NONE = 0,   /* Rekt : ne produit jamais rien */
    RES_BTC,
    RES_ETH,
    RES_ALT,
    RES_STABLE,
    RES_SOL,
    RES_COUNT       /* sentinelle : nombre d'actifs (Rekt inclus) */
} Resource;

/* Constructions sur une intersection (glossaire crypto).
 * La Ligne (route) est portée par une arête, pas par une intersection. */
typedef enum {
    BUILD_NONE = 0,
    BUILD_POSITION,   /* colonie : 1 point, produit 1 unité */
    BUILD_DESK        /* desk (ville) : 2 points, produit 2 unités (D8) */
} Building;

#endif /* TYPES_H */
