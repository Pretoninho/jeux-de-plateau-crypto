/* test_setup.c — vérifie la génération de plateau (T2) : distribution des
 * ressources, jetons numériques, reproductibilité à graine égale.
 */
#include <assert.h>
#include <stdio.h>

#include "../src/board.h"
#include "../src/rng.h"
#include "../src/setup.h"
#include "../src/game.h"

/* Contrôle les invariantes de distribution sur un plateau généré. */
static void check_distribution(const Board *b) {
    int res[RES_COUNT] = { 0 };
    int tok[13] = { 0 };          /* index = valeur du jeton 0..12 */
    int rekt = 0;

    for (int i = 0; i < b->n_tiles; i++) {
        const Tile *t = &b->tiles[i];
        assert(t->resource >= 0 && t->resource < RES_COUNT);
        res[t->resource]++;

        if (t->resource == RES_NONE) {
            assert(t->number == 0);   /* Rekt : aucune production */
            rekt++;
        } else {
            assert(t->number >= 2 && t->number <= 12 && t->number != 7);
            tok[t->number]++;
        }
    }

    /* Distribution des actifs (révisée au playtest : BTC 6 / SOL 2). */
    assert(res[RES_BTC]    == 6);
    assert(res[RES_ALT]    == 4);
    assert(res[RES_ETH]    == 3);
    assert(res[RES_STABLE] == 3);
    assert(res[RES_SOL]    == 2);
    assert(res[RES_NONE]   == 1);
    assert(rekt == 1);

    /* Multiset des jetons : 2,3,3,4,4,5,5,6,6,8,8,9,9,10,10,11,11,12. */
    static const int expect[13] = { 0, 0, 1, 2, 2, 2, 2, 0, 2, 2, 2, 2, 1 };
    for (int v = 2; v <= 12; v++) {
        assert(tok[v] == expect[v]);
    }
}

int main(void) {
    /* 1) API directe board_generate() avec un RNG explicite. */
    Rng r;
    rng_seed(&r, 42u);
    Board bd;
    board_build(&bd);
    board_generate(&bd, &r);
    check_distribution(&bd);

    /* 2) Robustesse : la distribution tient pour de nombreuses graines. */
    for (unsigned int s = 0; s < 300u; s++) {
        GameState g;
        game_init(&g, 3, s, s);
        check_distribution(&g.board);
    }

    /* 3) Reproductibilité : même graine → plateau identique. */
    GameState a, b;
    game_init(&a, 4, 999u, 999u);
    game_init(&b, 4, 999u, 999u);
    for (int i = 0; i < MAX_TILES; i++) {
        assert(a.board.tiles[i].resource == b.board.tiles[i].resource);
        assert(a.board.tiles[i].number   == b.board.tiles[i].number);
    }

    /* 4) La graine a un effet : deux graines distinctes diffèrent. */
    GameState c;
    game_init(&c, 4, 1000u, 1000u);
    int differ = 0;
    for (int i = 0; i < MAX_TILES; i++) {
        if (a.board.tiles[i].resource != c.board.tiles[i].resource ||
            a.board.tiles[i].number   != c.board.tiles[i].number) {
            differ = 1;
        }
    }
    assert(differ);

    /* 5) rng_range reste dans les bornes et couvre les valeurs. */
    Rng rr;
    rng_seed(&rr, 7u);
    int seen[6] = { 0 };
    for (int i = 0; i < 2000; i++) {
        int v = rng_range(&rr, 6);
        assert(v >= 0 && v < 6);
        seen[v] = 1;
    }
    for (int v = 0; v < 6; v++) {
        assert(seen[v]);
    }

    printf("OK generation : distribution, jetons, reproductibilite, RNG.\n");
    printf("  verifie sur 300 graines + reproductibilite a graine egale.\n");
    return 0;
}
