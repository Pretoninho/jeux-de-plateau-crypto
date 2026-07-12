/* hex.c — implémentation de l'arithmétique cube (cf. hex.h). */
#include "hex.h"

/* Ordre cyclique canonique des 6 voisins (60° d'écart entre consécutifs). */
const Cube HEX_DIRS[6] = {
    { +1, -1,  0 }, { +1,  0, -1 }, {  0, +1, -1 },
    { -1, +1,  0 }, { -1,  0, +1 }, {  0, -1, +1 }
};

static int iabs(int v) {
    return v < 0 ? -v : v;
}

Cube cube_add(Cube a, Cube b) {
    Cube r = { a.x + b.x, a.y + b.y, a.z + b.z };
    return r;
}

int cube_eq(Cube a, Cube b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

int cube_dist(Cube a, Cube b) {
    return (iabs(a.x - b.x) + iabs(a.y - b.y) + iabs(a.z - b.z)) / 2;
}

/* Ordre lexicographique (x, y, z) : -1 si a<b, 0 si égal, +1 si a>b. */
int cube_cmp(Cube a, Cube b) {
    if (a.x != b.x) return a.x < b.x ? -1 : 1;
    if (a.y != b.y) return a.y < b.y ? -1 : 1;
    if (a.z != b.z) return a.z < b.z ? -1 : 1;
    return 0;
}

Cube cube_neighbor(Cube c, int dir) {
    return cube_add(c, HEX_DIRS[dir]);
}
