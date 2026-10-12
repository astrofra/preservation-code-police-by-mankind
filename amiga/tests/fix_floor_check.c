/* Code Police — fixed-point generator check (milestone N3), host only.
 *
 * The chosen validation (option a): run the engine's floating-point findfloor on a
 * synthetic block grid, run the fixed-point port on the same rays and parameters,
 * and measure the difference in the Q16.16 texture coordinates and the additive
 * colour index. The result is a tolerance report, because fixed versus double
 * rounding will not be bit-identical, and the point is to bound the divergence.
 *
 * Build:
 *   cc -std=c99 -I amiga/src amiga/tests/fix_floor_check.c amiga/src/agart_fix.c \
 *      amiga/src/agart_trig.c amiga/src/agart_gen.c -lm -o amiga/build/fix_floor_check && \
 *   ./amiga/build/fix_floor_check
 */
#include "agart_gen.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MX 12
#define MY 8

static double dabs(double v) { return v < 0 ? -v : v; }

int main(void) {
    /* A deterministic ray grid and camera, in the ranges findfloor sees. */
    fix16 rays[MX * MY * 3];
    double fr[MX * MY * 3];
    int i, j;
    AgartFade hf;
    AgartColourLut lut;
    AgartBlock out[MX * MY];
    uint8_t palette[16][3];
    int max_du = 0, max_dv = 0, colour_mismatch = 0, total = MX * MY;
    double mean_du = 0, mean_dv = 0;

    srand(7);
    for (i = 0; i < MX * MY; ++i) {
        double xf = (rand() % 2000 - 1000) / 1000.0;
        double yf = (rand() % 2000 - 1000) / 1000.0;
        double zf = (rand() % 2000 - 1000) / 1000.0;
        fr[i * 3 + 0] = xf; fr[i * 3 + 1] = yf; fr[i * 3 + 2] = zf;
        rays[i * 3 + 0] = (fix16)(xf * 65536.0);
        rays[i * 3 + 1] = (fix16)(yf * 65536.0);
        rays[i * 3 + 2] = (fix16)(zf * 65536.0);
    }
    /* A ramp palette so the colour index is meaningful. */
    for (i = 0; i < 16; ++i)
        for (j = 0; j < 3; ++j)
            palette[i][j] = (uint8_t)(i * 17);
    agart_colour_lut_build(&lut, palette);

    /* helpfade: a simple ramp, in the engine's scaled units (v already *65536). */
    hf.Frd1 = (fix16)(10.0 * 65536.0); hf.Frv1 = 0;
    hf.Frd2 = (fix16)(100.0 * 65536.0); hf.Frv2 = (fix16)(200.0 * 65536.0);
    hf.Fgd1 = hf.Frd1; hf.Fgv1 = 0;
    hf.Fgd2 = hf.Frd2; hf.Fgv2 = (fix16)(180.0 * 65536.0);
    hf.Fbd1 = hf.Frd1; hf.Fbv1 = 0;
    hf.Fbd2 = hf.Frd2; hf.Fbv2 = (fix16)(160.0 * 65536.0);

    agart_floor_fix(rays, MX, MY, 0, 0, 0,
                    (fix16)(5.0 * 65536.0), (fix16)(50.0 * 65536.0),
                    (fix16)(2.0 * 65536.0), (fix16)(2.0 * 65536.0),
                    &hf, &lut, out);

    /* Floating-point reference in plain units with the SAME conventions the fixed
     * port uses: d, d2, d3, d4 are the plain engine arguments (5, 50, 2, 2), the
     * ray components are plain slopes, and the engine's "x *= 65536" step is the
     * fix16 shift, reproduced here only when the value is stored back (u, v, the
     * fade thresholds). depth stays plain, exactly as fix16_len3 returns it. */
    for (i = 0; i < MY; ++i) {
        for (j = 0; j < MX; ++j) {
            const double *ray = &fr[(i * MX + j) * 3];
            double xf = ray[0], yf = ray[1], zf = ray[2];
            double d = 5.0, d2 = 50.0, d3 = 2.0, d4 = 2.0;
            double dd9, dd10, depth, u, v;
            double fr1 = 10.0, fv1 = 0.0, fr2 = 100.0, fv2 = 200.0;
            double fg1 = 10.0, gv1 = 0.0, fg2 = 100.0, gv2 = 180.0;
            double fb1 = 10.0, bv1 = 0.0, fb2 = 100.0, bv2 = 160.0;
            double rr, gg, bb;
            int ir, ig, ib, cell, ref_index;
            int du, dv;

            if (yf == 0.0) yf = 1e-8;
            if (yf < 0.0) {
                dd9 = (zf * d) / yf; dd10 = (xf * d) / yf;
                u = (dd9 - 0.0) * d3; v = (dd10 - 0.0) * d4;
                depth = sqrt(dd9 * dd9 + dd10 * dd10 + d * d);
            } else {
                dd9 = (zf * d2) / yf; dd10 = (xf * d2) / yf;
                u = (-dd9 - 0.0) * d3; v = (-dd10 - 0.0) * d4;
                depth = sqrt(dd9 * dd9 + dd10 * dd10 + d2 * d2);
            }
            du = out[i * MX + j].u - (int)lround(u * 65536.0);
            dv = out[i * MX + j].v - (int)lround(v * 65536.0);
            if (dabs(du) > dabs(max_du)) max_du = du;
            if (dabs(dv) > dabs(max_dv)) max_dv = dv;
            mean_du += dabs(du); mean_dv += dabs(dv);

            rr = fv1;
            if (depth > fr2) rr = fv2;
            else if (depth > fr1) rr = fv1 + (fv2 - fv1) * (depth - fr1) / (fr2 - fr1);
            gg = gv1;
            if (depth > fg2) gg = gv2;
            else if (depth > fg1) gg = gv1 + (gv2 - gv1) * (depth - fg1) / (fg2 - fg1);
            bb = bv1;
            if (depth > fb2) bb = bv2;
            else if (depth > fb1) bb = bv1 + (bv2 - bv1) * (depth - fb1) / (fb2 - fb1);
            ir = (int)rr; ig = (int)gg; ib = (int)bb;
            if (ir < 0) ir = 0; else if (ir > 255) ir = 255;
            if (ig < 0) ig = 0; else if (ig > 255) ig = 255;
            if (ib < 0) ib = 0; else if (ib > 255) ib = 255;
            cell = ((ir >> 3) << 10) | ((ig >> 3) << 5) | (ib >> 3);
            ref_index = lut.cell[cell];
            if (ref_index != out[i * MX + j].colour) {
                if (colour_mismatch < 5)
                    printf("  colour diff at %d,%d: fix %d ref %d (rgb %d,%d,%d)\n",
                           j, i, out[i * MX + j].colour, ref_index, ir, ig, ib);
                ++colour_mismatch;
            }
        }
    }
    printf("findfloor: %d blocks, max |du|=%d (%.4f texels), max |dv|=%d (%.4f texels)\n",
           total, max_du, max_du / 65536.0, max_dv, max_dv / 65536.0);
    printf("           mean |du|=%.2f (%.5f texels), mean |dv|=%.2f (%.5f texels)\n",
           mean_du / total, mean_du / total / 65536.0,
           mean_dv / total, mean_dv / total / 65536.0);
    printf("colour index mismatches: %d / %d\n", colour_mismatch, total);
    return 0;
}
