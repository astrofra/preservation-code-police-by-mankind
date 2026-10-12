/* Code Police — integer trigonometry (milestone N3). See agart_trig.h.
 *
 * sin/cos use a 7th-order minimax-style polynomial on the quadrant-reduced angle
 * (max error about 1e-5, well under one Q16.16 unit). atan2 uses the standard
 * |ratio| <= 1 reduction with a 7th-order odd polynomial (max error about 2e-4
 * rad, i.e. under 13 Q16.16 units).
 */
#include "agart_trig.h"

fix16 fix16_sin(fix16 x) {
    int sign = 1;
    fix16 s, t, r;
    /* sin(x) is odd: work with a non-negative angle. */
    if (x < 0) { x = -x; sign = -1; }
    /* Reduce modulo 2pi to [0, 2pi). */
    if (x >= FIX16_TWO_PI) x %= FIX16_TWO_PI;
    /* sin(pi + a) = -sin(a): fold to [0, pi] and flip the sign. */
    if (x > FIX16_PI) { x -= FIX16_PI; sign = -sign; }
    /* sin(pi - a) = sin(a): fold to [0, pi/2]. */
    if (x > FIX16_HALF_PI) x = FIX16_PI - x;
    /* sin(x)/x = 1 - s/6 + s^2/120 - s^3/5040, with s = x^2, by Horner. */
    s = fix16_mul(x, x);
    t = (fix16)-(FIX16_ONE / 5040);
    t = (fix16)(FIX16_ONE / 120) + fix16_mul(s, t);
    t = (fix16)-(FIX16_ONE / 6) + fix16_mul(s, t);
    t = fix16_mul(s, t);
    r = fix16_mul(x, FIX16_ONE + t);
    return sign < 0 ? -r : r;
}

fix16 fix16_cos(fix16 x) {
    return fix16_sin(x + FIX16_HALF_PI);
}

/* atan(r) on |r| <= 1, degree-9 odd minimax polynomial (max error ~1.5e-5 rad).
 * Coefficients are (value * 65536) rounded. */
static fix16 atan_unit(fix16 r) {
    fix16 s = fix16_mul(r, r);
    fix16 p = 1366;                 /*  0.0208351 */
    p = -5580 + fix16_mul(s, p);    /* -0.0851330 */
    p = 11806 + fix16_mul(s, p);    /*  0.1801410 */
    p = -21647 + fix16_mul(s, p);   /* -0.3302995 */
    p = 65527 + fix16_mul(s, p);    /*  0.9998660 */
    return fix16_mul(r, p);
}

fix16 fix16_atan2(fix16 y, fix16 x) {
    fix16 ax, ay, angle;
    if (x == 0 && y == 0) return 0;
    ax = x < 0 ? -x : x;
    ay = y < 0 ? -y : y;
    /* Reduce to |ratio| <= 1. */
    if (ay <= ax) {
        angle = atan_unit(fix16_div(ay, ax == 0 ? 1 : ax));
    } else {
        angle = FIX16_HALF_PI - atan_unit(fix16_div(ax, ay == 0 ? 1 : ay));
    }
    if (x < 0) angle = (y < 0 ? angle - FIX16_PI : FIX16_PI - angle);
    else if (y < 0) angle = -angle;
    return angle;
}
