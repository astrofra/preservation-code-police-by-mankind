/* Code Police — Java RNG and time, for the object graph. */
#ifndef JAVA_RANDOM_H
#define JAVA_RANDOM_H

#include <stdint.h>

typedef struct java_random { uint64_t state; } java_random;

static inline void jr_seed(java_random *r, uint64_t s) {
    r->state = (s ^ 0x5deece66dULL) & ((1ULL << 48) - 1);
}
static inline uint64_t jr_next(java_random *r, int bits) {
    r->state = (r->state * 0x5deece66dULL + 11) & ((1ULL << 48) - 1);
    return r->state >> (48 - bits);
}
static inline double jr_double(java_random *r) {
    uint64_t a = jr_next(r, 26), b = jr_next(r, 27);
    return (double)((a << 27) + b) / 9007199254740992.0;
}

#endif /* JAVA_RANDOM_H */
