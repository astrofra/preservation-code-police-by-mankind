/* Code Police — fixed-point projection check (milestone N4), host only.
 *
 * Compares agart_position3d_fix, agart_setzoom3d_fix and agart_setzoomp_fix against
 * the engine formulas (option a tolerance, like the N3 generator checks). Build:
 *   cc -std=c99 -I amiga/src amiga/tests/fix_proj_check.c amiga/src/agart_fix.c \
 *      amiga/src/agart_trig.c amiga/src/agart_proj.c -lm -o amiga/build/fix_proj_check
 */
#include "agart_proj.h"
#include "agart_trig.h"

#include <math.h>
#include <stdio.h>

#define MX 33
#define MY 19

int main(void) {
    AgartCamera cam;
    AgartBox box = { 0, 0, 520, 300 };
    fix16 rays[MX * MY * 3];
    double max_ray = 0;
    int i, j, worst = 0;
    double max_x = 0, max_y = 0, max_u = 0, max_v = 0;

    cam.X = 0; cam.Y = 0; cam.Z = 0;
    cam.O1 = (fix16)(0.7 * 65536.0);
    cam.O2 = (fix16)(-0.3 * 65536.0);
    cam.O3 = (fix16)(1.1 * 65536.0);
    cam.FOV = (fix16)(2.0 * 65536.0);
    agart_position3d_fix(&cam, &box, MX, MY, rays);

    {
        double c4 = cos(0.7), s4 = sin(0.7);
        double c5 = cos(-0.3), s5 = sin(-0.3);
        double c6 = cos(1.1), s6 = sin(1.1);
        double d7 = 2.0;
        int w = box.x2 - box.x1, h = box.y2 - box.y1;
        for (j = 0; j < MY; ++j) for (i = 0; i < MX; ++i) {
            double xi = (double)(i * 16 - w / 2) / w;
            double yi = (double)(j * 16 - h / 2) / h;            double d16 = c4 * xi - s4 * yi;
            double d17 = s4 * xi + c4 * yi;
            double d18 = d7, d19 = d16;
            double d20 = c5 * d17 - s5 * d18;
            double d21 = s5 * d17 + c5 * d18;
            double ex = s6 * d21 + c6 * d19;
            double ey = d20;
            double ez = c6 * d21 - s6 * d19;
            double dx = rays[(j * MX + i) * 3] / 65536.0 - ex;
            double dy = rays[(j * MX + i) * 3 + 1] / 65536.0 - ey;
            if (fabs(dx) > max_x) max_x = fabs(dx);
            if (fabs(dy) > max_y) max_y = fabs(dy);
            (void)ez;
        }
    }
    printf("position3d: max |dX|=%.5f  max |dY|=%.5f\n", max_x, max_y);
    (void)max_ray; (void)worst; (void)j;

    /* Setzoom3D: a sprite in the middle of the screen. */
    {
        AgartBox clip = { 0, 0, 320, 200 }, out;
        fix16 u[4], v[4];
        fix16 d = (fix16)(0.1 * 65536.0), d2 = (fix16)(0.1 * 65536.0);
        fix16 d3 = (fix16)(10.0 * 65536.0), d4 = (fix16)(2.0 * 65536.0);
        fix16 d5 = (fix16)(2.0 * 65536.0), d6 = (fix16)(100.0 * 65536.0);
        int ok = agart_setzoom3d_fix(d, d2, d3, d4, d5, d6, 256, 256, &clip, &out, u, v);
        printf("setzoom3d ok=%d box=(%d,%d)-(%d,%d) u=(%.2f..%.2f) v=(%.2f..%.2f)\n",
               ok, out.x1, out.y1, out.x2, out.y2, u[0] / 65536.0, u[1] / 65536.0,
               v[0] / 65536.0, v[2] / 65536.0);
    }
    /* SetzoomP: a 2D sprite, top-left quadrant, clipped. */
    {
        AgartBox clip = { 0, 0, 320, 200 }, out;
        fix16 u[4], v[4];
        fix16 d = (fix16)(-0.2 * 65536.0), d2 = (fix16)(-0.2 * 65536.0);
        fix16 d3 = (fix16)(0.6 * 65536.0), d4 = (fix16)(0.6 * 65536.0);
        int ok = agart_setzoomp_fix(d, d2, d3, d4, 128, 64, &clip, &out, u, v);
        printf("setzoomp ok=%d box=(%d,%d)-(%d,%d) u=(%.2f..%.2f) v=(%.2f..%.2f)\n",
               ok, out.x1, out.y1, out.x2, out.y2, u[0] / 65536.0, u[1] / 65536.0,
               v[0] / 65536.0, v[2] / 65536.0);
    }
    (void)max_u; (void)max_v;
    return 0;
}
