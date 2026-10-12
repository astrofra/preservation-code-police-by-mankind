/* Code Police — Q16.16 fixed-point core (milestone N3). See agart_fix.h. */
#include "agart_fix.h"

fix16 fix16_mul(fix16 a, fix16 b) {
    return (fix16)(((int64_t)a * b) >> FIX16_SHIFT);
}

fix16 fix16_div(fix16 a, fix16 b) {
    if (b == 0) return a < 0 ? (fix16)0x80000000 : (fix16)0x7fffffff;
    return (fix16)((((int64_t)a) << FIX16_SHIFT) / b);
}

/* Integer square root of a 64-bit value by Newton's method, seeded from the bit
 * length. Returns floor(sqrt(n)). Exact: the loop converges on the integer root. */
static uint32_t isqrt64(uint64_t n) {
    uint64_t x, y;
    int b;
    if (n == 0) return 0;
    /* Seed: the highest power of two not exceeding sqrt(n). */
    x = 0;
    for (b = 62; b >= 0; b -= 2) {
        uint64_t t = (uint64_t)1 << b;
        if (n >= t * t || b == 0) { x = (uint64_t)1 << (b / 2); break; }
    }
    for (;;) {
        y = (x + n / x) >> 1;
        if (y >= x) break;
        x = y;
    }
    return (uint32_t)x;
}

fix16 fix16_sqrt(fix16 a) {
    if (a <= 0) return 0;
    /* sqrt(a / 2^16) * 2^16 = sqrt((a << 16)). */
    return (fix16)isqrt64((uint64_t)(uint32_t)a << FIX16_SHIFT);
}

fix16 fix16_len3(fix16 x, fix16 y, fix16 z) {
    /* The raw squares are Q32, so their integer square root is already Q16: no
     * extra shift. Every generator feeds |v| < 2^20 (unit-plane rays and radii),
     * so the Q32 sum is far below 2^63 and never overflows. Shifting each square
     * down to Q16 first would lose the fraction of small values and cost texels;
     * see the regression test for a tunnel block with a tiny ray. */
    int64_t sum = (int64_t)x * x + (int64_t)y * y + (int64_t)z * z;
    if (sum <= 0) return 0;
    return (fix16)isqrt64((uint64_t)sum);
}

fix16 fix16_clamp(fix16 v, fix16 lo, fix16 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
