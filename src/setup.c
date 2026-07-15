/* setup.c — génération de plateau (cf. setup.h). */
#include "setup.h"

#include <assert.h>

/* Distribution des ressources : 18 tuiles ressource.
 * Révisée au playtest (2026-07-14) : SOL 1→2, BTC 7→6, pour lever le
 * déséquilibre lié à la rareté de SOL (voir MEMORY). SOL reste le plus rare. */
static const Resource RESOURCE_POOL[MAX_TILES - 1] = {
    RES_BTC, RES_BTC, RES_BTC, RES_BTC, RES_BTC, RES_BTC,           /* 6 */
    RES_ALT, RES_ALT, RES_ALT, RES_ALT,                            /* 4 */
    RES_ETH, RES_ETH, RES_ETH,                                     /* 3 */
    RES_STABLE, RES_STABLE, RES_STABLE,                            /* 3 */
    RES_SOL, RES_SOL                                              /* 2 */
};

/* Jetons numériques (cf. spec) : jamais de 7. */
static const int TOKEN_POOL[MAX_TILES - 1] = {
    2, 3, 3, 4, 4, 5, 5, 6, 6, 8, 8, 9, 9, 10, 10, 11, 11, 12
};

/* Mélange de Fisher-Yates sur un tableau d'entiers. */
static void shuffle_int(Rng *rng, int *a, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rng_range(rng, i + 1);
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}

void board_generate(Board *b, Rng *rng) {
    assert(b->n_tiles == MAX_TILES);

    /* 1) Ressources : 18 tuiles + 1 Rekt (RES_NONE), le tout mélangé. */
    int res[MAX_TILES];
    res[0] = (int)RES_NONE;                       /* la case Rekt */
    for (int i = 0; i < MAX_TILES - 1; i++) {
        res[i + 1] = (int)RESOURCE_POOL[i];
    }
    shuffle_int(rng, res, MAX_TILES);

    for (int i = 0; i < MAX_TILES; i++) {
        b->tiles[i].resource = (Resource)res[i];
    }

    /* 2) Jetons : mélangés, distribués aux seules tuiles ressource.
     *    La case Rekt reçoit 0 (aucune production). */
    int tok[MAX_TILES - 1];
    for (int i = 0; i < MAX_TILES - 1; i++) {
        tok[i] = TOKEN_POOL[i];
    }
    shuffle_int(rng, tok, MAX_TILES - 1);

    int k = 0;
    for (int i = 0; i < MAX_TILES; i++) {
        if (b->tiles[i].resource == RES_NONE) {
            b->tiles[i].number = 0;
        } else {
            b->tiles[i].number = tok[k++];
        }
    }
    assert(k == MAX_TILES - 1);   /* les 18 jetons ont été placés */
}
