/* Code Police — fixed-point core tests (milestone N3), host only.
 *
 * Pins the Q16.16 helpers against known values and against the platform's double
 * maths, so the generator ports can be trusted before they are compared to the
 * ARGB oracle. Build:
 *
 *   cc -std=c99 -I amiga/src amiga/tests/fix_check.c amiga/src/agart_fix.c \
 *      -lm -o amiga/build/fix_check && ./amiga/build/fix_check
 */
#include "agart_fix.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int failures;

static void near(const char *what, double got, double want, double tol) {
    double d = fabs(got - want);
    if (d > tol) {
        printf("FAIL %-22s got %.6f want %.6f (tol %.6f)\n", what, got, want, tol);
        ++failures;
    } else {
        printf("ok   %-22s %.6f\n", what, got);
    }
}

int main(void) {
    int i;
    /* mul: 1.5 * 2.0 = 3.0 */
    near("mul 1.5*2", fix16_mul(FIX16_ONE * 3 / 2, FIX16_ONE * 2) / 65536.0, 3.0, 0);
    /* div: 7 / 2 = 3.5 */
    near("div 7/2", fix16_div(FIX16_ONE * 7, FIX16_ONE * 2) / 65536.0, 3.5, 0);
    /* sqrt(2.0) */
    near("sqrt 2", fix16_sqrt(FIX16_ONE * 2) / 65536.0, sqrt(2.0), 2.0 / 65536.0);
    /* sqrt of the largest value Q16.16 can hold (~32768): sqrt = 181.02 */
    near("sqrt 32767", fix16_sqrt((fix16)0x7fff0000) / 65536.0, sqrt(32767.0), 0.01);
    /* len3(3,4,0) = 5; and a big case len3(3000,4000,0) = 5000 */
    near("len3 3,4,0", fix16_len3(FIX16_ONE * 3, FIX16_ONE * 4, 0) / 65536.0, 5.0, 2e-4);
    near("len3 3000,4000,0",
         fix16_len3((fix16)(3000.0 * 65536.0), (fix16)(4000.0 * 65536.0), 0) / 65536.0,
         5000.0, 0.05);
    /* Randomised cross-check against double maths. The tolerance is relative:
     * quantising the operands to 1/65536 already moves a quotient by ~|q|/2^16
     * times the operand, so an absolute bound would be wrong for large results. */
    srand(1);
    for (i = 0; i < 20000; ++i) {
        double a = (rand() % 200000) / 1000.0 - 100.0;
        double b = (rand() % 200000) / 1000.0 - 100.0;
        fix16 fa = (fix16)(a * 65536.0), fb = (fix16)(b * 65536.0);
        double m = fix16_mul(fa, fb) / 65536.0;
        if (fabs(m - a * b) > 1e-3 * (1.0 + fabs(a * b))) {
            printf("FAIL mul %f*%f -> %f\n", a, b, m);
            ++failures;
            break;
        }
        if (fabs(b) > 0.5) {
            double q = fix16_div(fa, fb) / 65536.0;
            if (fabs(q - a / b) > 1e-3 * (1.0 + fabs(a / b))) {
                printf("FAIL div %f/%f -> %f (want %f)\n", a, b, q, a / b);
                ++failures;
                break;
            }
        }
    }
    /* Randomised sqrt over the representable range: relative error must be tiny. */
    for (i = 0; i < 20000; ++i) {
        double a = (rand() % 30000000) / 1000.0;      /* 0 .. 30000, fits Q16.16 */
        fix16 fa = (fix16)(a * 65536.0);
        double r = fix16_sqrt(fa) / 65536.0;
        if (fabs(r - sqrt(a)) > 2e-3 * (1.0 + sqrt(a))) {
            printf("FAIL sqrt %f -> %f (want %f)\n", a, r, sqrt(a));
            ++failures;
            break;
        }
    }
    printf(failures ? "--- %d FAILURES\n" : "--- all fixed-point checks passed\n", failures);
    return failures != 0;
}
