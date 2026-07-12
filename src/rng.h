/* rng.h — générateur pseudo-aléatoire isolé et seedé (D5).
 *
 * Un seul point à migrer si besoin (xorshift32 → PCG32…). État explicite
 * (pas de rand() global) → runs reproductibles à graine égale. Sans I/O.
 */
#ifndef RNG_H
#define RNG_H

typedef struct {
    unsigned int state;
} Rng;

/* Initialise le générateur à partir d'une graine (0 accepté). */
void rng_seed(Rng *r, unsigned int seed);

/* Prochain mot pseudo-aléatoire 32 bits (xorshift32). */
unsigned int rng_next(Rng *r);

/* Entier uniforme dans [0, n), n > 0, sans biais modulo (rejet). */
int rng_range(Rng *r, int n);

#endif /* RNG_H */
