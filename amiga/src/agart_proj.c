/* Code Police — fixed-point projection and sprite UV (milestone N4).
 *
 * See agart_proj.h. Transcribed from engine.cpp Position3D / Setzoom3D / SetzoomP,
 * keeping the engine's argument order and its exact clipping arithmetic. Every
 * quantity is Q16.16; the engine's jint(x * 65536) is PX(x) here, and its jint(x) is
 * a truncation to whole pixels (>> 16).
 *
 * Corner order is the primitive's: 0 = top-left, 1 = top-right, 2 = bottom-left,
 * 3 = bottom-right, matching the four arguments of drawrecUVS.
 */
#include "agart_proj.h"
#include "agart_trig.h"

/* One Q16.16 value from an integer pixel count. */
#define PX(n) ((fix16)((int64_t)(n) << 16))
#define HALF ((fix16)(FIX16_ONE / 2))

void agart_position3d_fix(const AgartCamera *cam, const AgartBox *box,
                          int mx, int my, fix16 *rays) {
    fix16 c4 = fix16_cos(cam->O1), s4 = fix16_sin(cam->O1);
    fix16 c5 = fix16_cos(cam->O2), s5 = fix16_sin(cam->O2);
    fix16 c6 = fix16_cos(cam->O3), s6 = fix16_sin(cam->O3);
    fix16 d7 = cam->FOV;                 /* the projection plane depth */
    int w = box->x2 - box->x1;
    int h = box->y2 - box->y1;
    int half_w = w / 2, half_h = h / 2;
    int i, j;
    for (j = 0; j < my; ++j) {
        for (i = 0; i < mx; ++i) {
            /* XI, YI: the block's top-left relative to the box centre, scaled by
             * 1/width and 1/height (the engine's d and d2 in rcr_init_root). The
             * engine's loop starts at 0 and steps by the block size, so there is no
             * half-block offset here. */
            fix16 xi = fix16_div(PX(i * 16 - half_w), PX(w ? w : 1));
            fix16 yi = fix16_div(PX(j * 16 - half_h), PX(h ? h : 1));
            fix16 d16 = fix16_mul(c4, xi) - fix16_mul(s4, yi);
            fix16 d17 = fix16_mul(s4, xi) + fix16_mul(c4, yi);
            fix16 d18 = d7;
            fix16 d19 = d16;
            fix16 d20 = fix16_mul(c5, d17) - fix16_mul(s5, d18);
            fix16 d21 = fix16_mul(s5, d17) + fix16_mul(c5, d18);
            fix16 *r = rays + (j * mx + i) * 3;
            r[0] = fix16_mul(s6, d21) + fix16_mul(c6, d19);
            r[1] = d20;
            r[2] = fix16_mul(c6, d21) - fix16_mul(s6, d19);
        }
    }
}

int agart_setzoom3d_fix(fix16 d, fix16 d2, fix16 d3, fix16 d4, fix16 d5, fix16 d6,
                        int tex_modulo, int tex_height, const AgartBox *clip,
                        AgartBox *out_box, fix16 out_u[4], fix16 out_v[4]) {
    fix16 d11 = PX(clip->x2 - clip->x1);
    fix16 d12 = PX(clip->y2 - clip->y1);
    fix16 d13, d14, d15, c, c2, d10, d9, d16, d17, d18, d19, d20, d21;
    if (d3 <= (fix16)655) return 0;                 /* d3 <= 0.01 */
    if (d11 <= 0 || d12 <= 0) return 0;
    d13 = fix16_div(d6, d3);
    d14 = fix16_div(PX(tex_modulo), d11);
    d15 = fix16_div(PX(tex_height), d12);
    c = fix16_mul(d, d13);
    c2 = fix16_mul(d2, d13);
    d14 = fix16_mul(fix16_mul(fix16_mul(d14, HALF), d4), d13);
    d15 = fix16_mul(fix16_mul(fix16_mul(d15, HALF), d5), d13);
    d10 = fix16_mul(c - d14, d11) + PX(clip->x1) + (fix16)(d11 >> 1);
    d9 = fix16_mul(c2 - d15, d12) + PX(clip->y1) + (fix16)(d12 >> 1);
    d16 = fix16_mul(c + d14, d11) + PX(clip->x1) + (fix16)(d11 >> 1);
    d17 = fix16_mul(c2 + d15, d12) + PX(clip->y1) + (fix16)(d12 >> 1);
    d18 = 0; d19 = 0;
    d20 = PX(tex_modulo - 1);
    d21 = PX(tex_height - 1);
    if (d10 > d16) { fix16 t = d16; d16 = d10; d10 = t; t = d20; d20 = d18; d18 = t; }
    if (d9 > d17) { fix16 t = d17; d17 = d9; d9 = t; t = d21; d21 = d19; d19 = t; }
    if (d16 > PX(clip->x2)) {
        d20 = fix16_mul(d20 - d18, fix16_div(PX(clip->x2) - d10, d16 - d10));
        d16 = PX(clip->x2);
    }
    if (d10 < PX(clip->x1)) {
        d18 = fix16_mul(d20 - d18, fix16_div(PX(clip->x1) - d10, d16 - d10));
        d10 = PX(clip->x1);
    }
    if (d17 > PX(clip->y2)) {
        d21 = fix16_mul(d21 - d19, fix16_div(PX(clip->y2) - d9, d17 - d9));
        d17 = PX(clip->y2);
    }
    if (d9 < PX(clip->y1)) {
        d19 = fix16_mul(d21 - d19, fix16_div(PX(clip->y1) - d9, d17 - d9));
        d9 = PX(clip->y1);
    }
    out_box->x1 = (int)(d10 >> 16);
    out_box->y1 = (int)(d9 >> 16);
    out_box->x2 = (int)(d16 >> 16);
    out_box->y2 = (int)(d17 >> 16);
    if (out_box->x2 <= out_box->x1 || out_box->y2 <= out_box->y1) return 0;
    /* u: left corners carry d18, right corners d20. v: top corners d19, bottom
     * corners d21. */
    out_u[0] = d18; out_u[1] = d20; out_u[2] = d18; out_u[3] = d20;
    out_v[0] = d19; out_v[1] = d19; out_v[2] = d21; out_v[3] = d21;
    return 1;
}

int agart_setzoomp_fix(fix16 d, fix16 d2, fix16 d3, fix16 d4,
                       int tex_modulo, int tex_height, const AgartBox *clip,
                       AgartBox *out_box, fix16 out_u[4], fix16 out_v[4]) {
    fix16 d7 = PX(clip->x2 - clip->x1);
    fix16 d8 = PX(clip->y2 - clip->y1);
    fix16 d9 = 0, d10 = 0;                 /* u_lo, v_lo */
    fix16 d11 = PX(tex_modulo - 1);        /* u_hi */
    fix16 d12 = PX(tex_height - 1);        /* v_hi */
    if (d7 <= 0 || d8 <= 0) return 0;
    d = fix16_mul(d, d7) + PX(clip->x1);
    d2 = fix16_mul(d2, d8) + PX(clip->y1);
    d3 = fix16_mul(d3, d7) + PX(clip->x1);
    d4 = fix16_mul(d4, d8) + PX(clip->y1);
    if (d > d3) { fix16 t = d3; d3 = d; d = t; t = d11; d11 = d9; d9 = t; }
    if (d2 > d4) { fix16 t = d4; d4 = d2; d2 = t; t = d12; d12 = d10; d10 = t; }
    if (d3 > PX(clip->x2)) { d11 = fix16_mul(d11 - d9, fix16_div(PX(clip->x2) - d, d3 - d)); d3 = PX(clip->x2); }
    if (d < PX(clip->x1)) { d9 = fix16_mul(d11 - d9, fix16_div(PX(clip->x1) - d, d3 - d)); d = PX(clip->x1); }
    if (d4 > PX(clip->y2)) { d12 = fix16_mul(d12 - d10, fix16_div(PX(clip->y2) - d2, d4 - d2)); d4 = PX(clip->y2); }
    if (d2 < PX(clip->y1)) { d10 = fix16_mul(d12 - d10, fix16_div(PX(clip->y1) - d2, d4 - d2)); d2 = PX(clip->y1); }
    out_box->x1 = (int)(d >> 16);
    out_box->y1 = (int)(d2 >> 16);
    out_box->x2 = (int)(d3 >> 16);
    out_box->y2 = (int)(d4 >> 16);
    if (out_box->x2 <= out_box->x1 || out_box->y2 <= out_box->y1) return 0;
    out_u[0] = d9; out_u[1] = d11; out_u[2] = d9; out_u[3] = d11;
    out_v[0] = d10; out_v[1] = d10; out_v[2] = d12; out_v[3] = d12;
    return 1;
}
