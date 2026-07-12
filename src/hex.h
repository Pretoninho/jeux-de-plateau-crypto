/* hex.h — arithmétique sur coordonnées cube pour grille hexagonale.
 *
 * Représentation « cube » (x + y + z == 0), standard et bien documentée
 * (cf. redblobgames). Sert de fondation à la topologie du plateau (D3).
 * Logique pure : aucun état global, aucune I/O.
 */
#ifndef HEX_H
#define HEX_H

/* Coordonnée cube d'un hexagone. Invariant : x + y + z == 0. */
typedef struct {
    int x, y, z;
} Cube;

/* Les 6 directions unitaires, dans l'ordre cyclique autour de l'hexagone.
 * Deux directions consécutives (d, d+1) encadrent un coin (intersection). */
extern const Cube HEX_DIRS[6];

Cube cube_add(Cube a, Cube b);
int  cube_eq(Cube a, Cube b);
int  cube_dist(Cube a, Cube b);
int  cube_cmp(Cube a, Cube b);          /* ordre total, pour clés canoniques */
Cube cube_neighbor(Cube c, int dir);    /* dir dans [0,6) */

#endif /* HEX_H */
