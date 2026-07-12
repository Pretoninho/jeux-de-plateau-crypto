/* rng.c — xorshift32 à état explicite (cf. rng.h). */
#include "rng.h"

#include <assert.h>

void rng_seed(Rng *r, unsigned int seed) {
    /* L'état de xorshift doit être non nul. */
    r->state = (seed != 0u) ? seed : 0x1u;
    /* Petit brassage pour les graines faibles (0, 1, 2…). */
    for (int i = 0; i < 8; i++) {
        (void)rng_next(r);
    }
}

unsigned int rng_next(Rng *r) {
    unsigned int x = r->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->state = x;
    return x;
}

int rng_range(Rng *r, int n) {
    assert(n > 0);
    unsigned int un = (unsigned int)n;
    /* Seuil de rejet : (2^32) mod n, écarte le « reste » qui biaiserait. */
    unsigned int threshold = (0u - un) % un;
    unsigned int x;
    do {
        x = rng_next(r);
    } while (x < threshold);
    return (int)(x % un);
}
