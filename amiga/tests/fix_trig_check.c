/* Code Police — integer trigonometry tests (milestone N3), host only.
 *
 * Measures fix16_sin / fix16_cos / fix16_atan2 against libm. Build:
 *   cc -std=c99 -I amiga/src amiga/tests/fix_trig_check.c amiga/src/agart_fix.c \
 *      amiga/src/agart_trig.c -lm -o amiga/build/fix_trig_check && \
 *   ./amiga/build/fix_trig_check
 */
#include "agart_trig.h"

#include <math.h>
#include <stdio.h>

int main(void) {
    int i;
    double max_sin = 0, max_cos = 0, max_atan = 0;
    double worst_sin_at = 0, worst_atan_at = 0;
    /* Sweep a full turn, plus a bit, and a band of atan2 ratios. */
    for (i = -700000; i <= 1400000; i += 37) {
        double a = i / 65536.0;
        double s = fix16_sin((fix16)i) / 65536.0;
        double c = fix16_cos((fix16)i) / 65536.0;
        double ds = fabs(s - sin(a)), dc = fabs(c - cos(a));
        if (ds > max_sin) { max_sin = ds; worst_sin_at = a; }
        if (dc > max_cos) max_cos = dc;
    }
    for (i = -4000000; i <= 4000000; i += 53) {
        int j;
        for (j = -4000000; j <= 4000000; j += 7919) {
            double y = i / 65536.0, x = j / 65536.0;
            double got = fix16_atan2((fix16)i, (fix16)j) / 65536.0;
            double want = atan2(y, x);
            double d = fabs(got - want);
            if (d > max_atan) { max_atan = d; worst_atan_at = want; }
        }
    }
    printf("sin  max abs err %.2e (at %.4f rad)\n", max_sin, worst_sin_at);
    printf("cos  max abs err %.2e\n", max_cos);
    printf("atan max abs err %.2e (at %.4f rad)\n", max_atan, worst_atan_at);
    /* The sin/cos Taylor series stops one term early because 1/362880 rounds to
     * zero in Q16.16, so ~4e-4 is the floor; atan uses a degree-9 minimax fit at
     * ~1e-4. Both are far below a texel. */
    printf("%s\n", (max_sin < 1e-3 && max_cos < 1e-3 && max_atan < 1e-3) ? "PASS" : "FAIL");
    return !(max_sin < 1e-3 && max_cos < 1e-3 && max_atan < 1e-3);
}
