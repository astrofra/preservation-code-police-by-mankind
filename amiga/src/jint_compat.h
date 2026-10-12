/* Code Police — jint compatibility for the C renderer.
 *
 * These inline helpers reproduce java_semantics.hpp's jint exactly: 32-bit
 * wraparound arithmetic, masked shifts, Java's arithmetic right shift, and the
 * saturating double-to-int narrowing (NaN -> 0). The C port must be bit-exact
 * with the verified C++ engine, so every operation goes through one of these.
 */
#ifndef JINT_COMPAT_H
#define JINT_COMPAT_H

#include <math.h>
#include <stdint.h>

typedef int32_t jint;

static inline jint jint_of(double v) {
    if (isnan(v)) return 0;
    if (v >= 2147483647.0) return (jint)0x7fffffff;
    if (v <= -2147483648.0) return (jint)0x80000000;
    return (jint)v;
}
static inline jint jadd(jint a, jint b) { return (jint)((uint32_t)a + (uint32_t)b); }
static inline jint jsub(jint a, jint b) { return (jint)((uint32_t)a - (uint32_t)b); }
static inline jint jmul(jint a, jint b) { return (jint)((uint32_t)a * (uint32_t)b); }
static inline jint jshl(jint a, jint b) { return (jint)((uint32_t)a << (uint32_t)(b & 31)); }
static inline jint jshr(jint a, jint b) {
    uint32_t n = (uint32_t)b & 31u;
    if (!n) return a;
    return (jint)(((uint32_t)a >> n) | (((uint32_t)a & 0x80000000u) ? (0xffffffffu << (32u - n)) : 0u));
}
static inline jint jdiv(jint a, jint b) { return (jint)((int64_t)a / (int64_t)b); }
static inline jint jmod(jint a, jint b) { return (jint)((int64_t)a % (int64_t)b); }
static inline jint jabs_or(jint a, jint b) { return (jint)((uint32_t)a | (uint32_t)b); }
static inline jint jand(jint a, jint b) { return (jint)((uint32_t)a & (uint32_t)b); }

/* Java's && / || over jint truthiness: nonzero is true. */
#define JTRUE(a) ((a) != 0)

#endif /* JINT_COMPAT_H */
