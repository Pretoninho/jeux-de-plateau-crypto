/* board.h — topologie du plateau : tuiles (hexagones), intersections
 * (Positions/Desks) et arêtes (Lignes), avec toutes les tables d'adjacence.
 *
 * Point dur du projet (D3). Représentation 100 % entière, sans flottant :
 *   - une intersection est identifiée par les 3 hexagones qui s'y rencontrent ;
 *   - une arête est identifiée par les 2 hexagones qui la partagent.
 * Cette clé canonique rend la déduplication et l'adjacence exactes.
 *
 * Moteur pur (D2) : board_build() ne fait aucune I/O.
 */
#ifndef BOARD_H
#define BOARD_H

#include "hex.h"
#include "types.h"

/* Tuile hexagonale. Ressource et jeton sont assignés plus tard (T2). */
typedef struct {
    Cube     coord;
    Resource resource;      /* rempli en T2 ; RES_NONE par défaut */
    int      number;        /* jeton 2..12 (jamais 7) ; 0 = non assigné */
    int      vertices[6];   /* ids des 6 intersections (coins) */
    int      edges[6];      /* ids des 6 arêtes */
    int      neighbors[6];  /* ids des tuiles voisines, -1 si hors plateau */
    int      n_neighbors;   /* nombre de voisines sur le plateau (3..6) */
} Tile;

/* Intersection : coin où se rencontrent jusqu'à 3 hexagones. */
typedef struct {
    Cube tri[3];            /* clé canonique triée (3 hexes, on-board ou non) */
    int  tiles[3];  int n_tiles;   /* tuiles réellement sur le plateau (1..3) */
    int  edges[3];  int n_edges;   /* arêtes incidentes (2..3) */
    int  adj[3];    int n_adj;     /* intersections voisines (2..3) */
} Vertex;

/* Arête : segment partagé par 2 hexagones (support d'une Ligne). */
typedef struct {
    Cube pair[2];          /* clé canonique triée (2 hexes) */
    int  v[2];             /* intersections aux extrémités */
    int  tiles[2]; int n_tiles;    /* tuiles sur le plateau (1..2) */
} Edge;

/* Plateau complet, entièrement à taille fixe (D4). */
typedef struct {
    int    n_tiles, n_vertices, n_edges;
    Tile   tiles[MAX_TILES];
    Vertex vertices[MAX_VERTICES];
    Edge   edges[MAX_EDGES];
} Board;

/* Construit la topologie (hexagones + intersections + arêtes + adjacences).
 * Ne remplit ni ressources ni jetons (cf. T2). Idempotent, sans I/O. */
void board_build(Board *b);

#endif /* BOARD_H */
