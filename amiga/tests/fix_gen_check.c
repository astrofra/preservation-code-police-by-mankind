/* Code Police — fixed-point generator check for tunnel and twirl (milestone N3),
 * host only. Same method as fix_floor_check.c (option a): the engine's floating
 * formulas against the fixed port, on the same ray grid, reporting coordinate
 * divergence in Q16.16 units and colour-index mismatches.
 *
 * Build:
 *   cc -std=c99 -I amiga/src amiga/tests/fix_gen_check.c amiga/src/agart_fix.c \
 *      amiga/src/agart_trig.c amiga/src/agart_gen.c -lm -o amiga/build/fix_gen_check
 */
#include "agart_gen.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MX 10
#define MY 6

static double dabs(double v) { return v < 0 ? -v : v; }

static void build_palette(uint8_t palette[16][3]) {
    int i, j;
    for (i = 0; i < 16; ++i)
        for (j = 0; j < 3; ++j)
            palette[i][j] = (uint8_t)(i * 17);
}

static void report(const char *name, int total, int max_du, int max_dv,
                   double mean_du, double mean_dv, int mismatch) {
    printf("%-8s %d blocks  max |du|=%.4f texel  max |dv|=%.4f texel  "
           "mean |du|=%.5f  colour mismatch %d/%d\n",
           name, total, max_du / 65536.0, max_dv / 65536.0,
           mean_du / total / 65536.0, mismatch, total);
}

int main(void) {
    fix16 rays[MX * MY * 3];
    double fr[MX * MY * 3];
    AgartBlock out[MX * MY];
    uint8_t palette[16][3];
    AgartColourLut lut;
    int i, j;
    AgartFade hf;

    srand(11);
    for (i = 0; i < MX * MY; ++i) {
        double xf = (rand() % 3000 - 1500) / 1000.0;
        double yf = (rand() % 3000 - 1500) / 1000.0;
        double zf = (rand() % 3000 - 1500) / 1000.0;
        fr[i * 3] = xf; fr[i * 3 + 1] = yf; fr[i * 3 + 2] = zf;
        rays[i * 3] = (fix16)(xf * 65536.0);
        rays[i * 3 + 1] = (fix16)(yf * 65536.0);
        rays[i * 3 + 2] = (fix16)(zf * 65536.0);
    }
    build_palette(palette);
    agart_colour_lut_build(&lut, palette);
    hf.Frd1 = (fix16)(10.0 * 65536.0); hf.Frv1 = 0;
    hf.Frd2 = (fix16)(100.0 * 65536.0); hf.Frv2 = (fix16)(200.0 * 65536.0);
    hf.Fgd1 = hf.Frd1; hf.Fgv1 = 0;
    hf.Fgd2 = hf.Frd2; hf.Fgv2 = (fix16)(180.0 * 65536.0);
    hf.Fbd1 = hf.Frd1; hf.Fbv1 = 0;
    hf.Fbd2 = hf.Frd2; hf.Fbv2 = (fix16)(160.0 * 65536.0);

    /* ---- findtunnel ---- */
    {
        double d = 5.0, d2 = 3.0;
        int max_du = 0, max_dv = 0, mismatch = 0, total = MX * MY;
        double mean_du = 0, mean_dv = 0;
        agart_tunnel_fix(rays, MX, MY, 0, (fix16)(d * 65536.0), (fix16)(d2 * 65536.0),
                         &hf, &lut, out);
        for (i = 0; i < MY; ++i) for (j = 0; j < MX; ++j) {
            const double *r = &fr[(i * MX + j) * 3];
            double xf = r[0], ay = dabs(r[1]), zf = r[2];
            double d8 = sqrt(xf * xf + ay * ay);
            double angle, d12, u, v, depth, rr, gg, bb;
            int du, dv, ir, ig, ib, cell;
            if (d8 == 0) d8 = 1e-7;
            angle = atan2(xf, ay);
            d12 = (zf / d8) * d;
            u = d12 * d2;
            v = trunc(angle * 5340353.715440872) + trunc(0x800000);
            depth = sqrt(d12 * d12 + d * d);
            du = out[i * MX + j].u - (int)lround(u * 65536.0);
            dv = out[i * MX + j].v - (int)lround(v);
            if (dabs(du) > dabs(max_du)) max_du = du;
            if (dabs(dv) > dabs(max_dv)) max_dv = dv;
            mean_du += dabs(du); mean_dv += dabs(dv);
            rr = 0; gg = 0; bb = 0;
            if (depth > 100.0) { rr = 200; gg = 180; bb = 160; }
            else if (depth > 10.0) { double t = depth - 10.0; rr = t * 200 / 90; gg = t * 180 / 90; bb = t * 160 / 90; }
            ir = (int)rr; ig = (int)gg; ib = (int)bb;
            cell = (((ir < 0 ? 0 : ir > 255 ? 255 : ir) >> 3) << 10) |
                   (((ig < 0 ? 0 : ig > 255 ? 255 : ig) >> 3) << 5) |
                   ((ib < 0 ? 0 : ib > 255 ? 255 : ib) >> 3);
            if (lut.cell[cell] != out[i * MX + j].colour) ++mismatch;
        }
        report("tunnel", total, max_du, max_dv, mean_du, mean_dv, mismatch);
    }

    /* ---- findtwirl ---- */
    {
        double d = 0.0, d2 = 0.05, d3 = 0.5, d4 = 0.0, d5 = 0.03, d6 = 100.0;
        int max_du = 0, max_dv = 0, mismatch = 0, total = MX * MY;
        double mean_du = 0, mean_dv = 0;
        agart_twirl_fix(rays, MX, MY, 0, 0,
                         (fix16)(d * 65536.0), (fix16)(d2 * 65536.0),
                         (fix16)(d3 * 65536.0), (fix16)(d4 * 65536.0),
                         (fix16)(d5 * 65536.0), (fix16)(d6 * 65536.0),
                         &lut, out);
        for (i = 0; i < MY; ++i) for (j = 0; j < MX; ++j) {
            const double *r = &fr[(i * MX + j) * 3];
            double d11 = r[0] * 256.0, d12 = r[1] * 256.0;
            double angle = atan2(d11, d12), rad = sqrt(d11 * d11 + d12 * d12);
            double d15 = d12 - 8.0, angle2, r2, u, v, n4;
            double rr, gg, bb;
            int du, dv, ir, ig, ib, cell;
            angle += d3 * sin((d + (rad * 3.141592653589793) / 128.0) * d2);
            rad += d6 * sin((d4 + (rad * 3.141592653589793) / 128.0) * d5);
            u = sin(angle) * rad * 65536.0;
            v = cos(angle) * rad * 65536.0;
            angle2 = atan2(d11, d15);
            r2 = sqrt(d11 * d11 + d15 * d15);
            angle2 += d3 * sin((d + (r2 * 3.141592653589793) / 128.0) * d2);
            r2 += d6 * sin((d4 + (r2 * 3.141592653589793) / 128.0) * d5);
            n4 = u - (sin(angle2) * r2 * 65536.0);
            du = out[i * MX + j].u - (int)lround(u);
            dv = out[i * MX + j].v - (int)lround(v);
            if (dabs(du) > dabs(max_du)) max_du = du;
            if (dabs(dv) > dabs(max_dv)) max_dv = dv;
            mean_du += dabs(du); mean_dv += dabs(dv);
            rr = n4 * 26; gg = n4 * 24; bb = n4 * 23;
            ir = (int)rr; ig = (int)gg; ib = (int)bb;
            cell = (((ir < 0 ? 0 : ir > 255 ? 255 : ir) >> 3) << 10) |
                   (((ig < 0 ? 0 : ig > 255 ? 255 : ig) >> 3) << 5) |
                   ((ib < 0 ? 0 : ib > 255 ? 255 : ib) >> 3);
            if (lut.cell[cell] != out[i * MX + j].colour) ++mismatch;
        }
        report("twirl", total, max_du, max_dv, mean_du, mean_dv, mismatch);
    }
    return 0;
}
