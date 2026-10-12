/* Code Police — fixed-point scene generators (milestone N3). See agart_gen.h. */
#include "agart_gen.h"
#include "agart_trig.h"

#include <string.h>

void agart_colour_lut_build(AgartColourLut *lut, const uint8_t palette_rgb[16][3]) {
    int cell;
    for (cell = 0; cell < 32768; ++cell) {
        int r = (((cell >> 10) & 31) * 255) / 31;
        int g = (((cell >> 5) & 31) * 255) / 31;
        int b = ((cell & 31) * 255) / 31;
        int i, best = 0, bestd = 1 << 30;
        for (i = 0; i < 16; ++i) {
            int dr = r - palette_rgb[i][0];
            int dg = g - palette_rgb[i][1];
            int db = b - palette_rgb[i][2];
            int d = dr * dr + dg * dg + db * db;
            if (d < bestd) { bestd = d; best = i; }
        }
        lut->cell[cell] = (uint8_t)best;
    }
}

uint8_t agart_colour_index(const AgartColourLut *lut, int r, int g, int b) {
    int cell;
    if (r < 0) r = 0; else if (r > 255) r = 255;
    if (g < 0) g = 0; else if (g > 255) g = 255;
    if (b < 0) b = 0; else if (b > 255) b = 255;
    cell = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
    return lut->cell[cell];
}

/* Clamps a wide intermediate channel to the engine's 0..255 range. */
static int clamp255(int64_t v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (int)v;
}

/* colorclip: pick a scalar amount for a depth, then a colour per channel. The
 * engine computes three independent ramps (r, g, b); here the three amounts are
 * combined into a single RGB triple and quantised once. */
uint8_t agart_colorclip_index(const AgartFade *hf, fix16 depth, const AgartColourLut *lut) {
    fix16 r = hf->Frv1, g = hf->Fgv1, b = hf->Fbv1;
    if (depth > hf->Frd2) {
        r = hf->Frv2;
    } else if (depth > hf->Frd1 && hf->Frd2 != hf->Frd1) {
        r = hf->Frv1 + fix16_mul(fix16_div(hf->Frv2 - hf->Frv1, hf->Frd2 - hf->Frd1), depth - hf->Frd1);
    }
    if (depth > hf->Fgd2) {
        g = hf->Fgv2;
    } else if (depth > hf->Fgd1 && hf->Fgd2 != hf->Fgd1) {
        g = hf->Fgv1 + fix16_mul(fix16_div(hf->Fgv2 - hf->Fgv1, hf->Fgd2 - hf->Fgd1), depth - hf->Fgd1);
    }
    if (depth > hf->Fbd2) {
        b = hf->Fbv2;
    } else if (depth > hf->Fbd1 && hf->Fbd2 != hf->Fbd1) {
        b = hf->Fbv1 + fix16_mul(fix16_div(hf->Fbv2 - hf->Fbv1, hf->Fbd2 - hf->Fbd1), depth - hf->Fbd1);
    }
    /* The engine stores jint(d) (truncated to integer) into kPixel->r/g/b before
     * drawrecUVRGB adds them, so truncate here too. */
    return agart_colour_index(lut, (int)(r >> 16), (int)(g >> 16), (int)(b >> 16));
}

void agart_floor_fix(const fix16 *rays, int mx, int my,
                     fix16 posX, fix16 posY, fix16 posZ,
                     fix16 d, fix16 d2, fix16 d3, fix16 d4,
                     const AgartFade *hf, const AgartColourLut *lut, AgartBlock *out) {
    int n, n2;
    /* Every input is Q16.16. The engine's u = jint((d8 - PosZ) * d3 * 65536) has
     * exactly the value fix16_mul(d8 - PosZ, fix16(d3)), so the fold is free. The
     * engine also does d += PosY and d2 -= PosY before the loop; do that here. */
    fix16 near_d = d + posY;
    fix16 far_d = d2 - posY;
    for (n = 0; n < my; ++n) {
        for (n2 = 0; n2 < mx; ++n2) {
            const fix16 *ray = rays + (n * mx + n2) * 3;
            fix16 xf = ray[0], yf = ray[1], zf = ray[2];
            fix16 den = yf;
            fix16 px, pz, depth;
            AgartBlock *b = out + n * mx + n2;
            if (den == 0) den = 1;               /* 1e-8 is below one Q16.16 unit */
            if (yf < 0) {
                px = fix16_mul(zf, fix16_div(near_d, den));
                pz = fix16_mul(xf, fix16_div(near_d, den));
                b->u = fix16_mul(px - posZ, d3);
                b->v = fix16_mul(pz - posX, d4);
                depth = fix16_len3(px, pz, near_d);
            } else {
                px = fix16_mul(zf, fix16_div(far_d, den));
                pz = fix16_mul(xf, fix16_div(far_d, den));
                b->u = fix16_mul(-px - posZ, d3);
                b->v = fix16_mul(-pz - posX, d4);
                depth = fix16_len3(px, pz, far_d);
            }
            b->colour = agart_colorclip_index(hf, depth, lut);
            b->pad = 0;
        }
    }
}

void agart_tunnel_fix(const fix16 *rays, int mx, int my,
                      fix16 posZ, fix16 d, fix16 d2,
                      const AgartFade *hf, const AgartColourLut *lut, AgartBlock *out) {
    int n, n2;
    /* Engine: u = jint((d12 + PosZ) * d2 * 65536), v = jint(angle * 5340353.7) +
     * 0x800000, depth = sqrt(d12^2 + d^2) because cos^2+sin^2 = 1. */
    for (n = 0; n < my; ++n) {
        for (n2 = 0; n2 < mx; ++n2) {
            const fix16 *ray = rays + (n * mx + n2) * 3;
            fix16 xf = ray[0], yf = ray[1], zf = ray[2];
            fix16 ay = yf < 0 ? -yf : yf;
            fix16 d8 = fix16_len3(xf, ay, 0);
            fix16 angle, d12, depth;
            AgartBlock *b = out + n * mx + n2;
            if (d8 == 0) d8 = 1;                 /* 1e-7 is below one Q16.16 unit */
            angle = fix16_atan2(xf, ay);
            d12 = fix16_mul(fix16_div(zf, d8), d);
            b->u = fix16_mul(d12 + posZ, d2);
            b->v = (fix16)((int64_t)angle * AGART_TUNNEL_SCALE >> FIX16_SHIFT);
            b->v = (fix16)((uint32_t)b->v + 0x800000u);
            depth = fix16_len3(d12, d, 0);
            b->colour = agart_colorclip_index(hf, depth, lut);
            b->pad = 0;
        }
    }
}

/* pi / 128 in Q16.16: the engine's "d14 * pi / 128". */
#define TWIRL_TWO_PI_OVER_256 ((fix16)1609)

void agart_twirl_fix(const fix16 *rays, int mx, int my,
                     fix16 posX, fix16 posY,
                     fix16 d, fix16 d2, fix16 d3, fix16 d4, fix16 d5, fix16 d6,
                     const AgartColourLut *lut, AgartBlock *out) {
    int n, n2;
    /* Engine argument order (kEffectX code 1 calls findtwirl with params 2..7):
     *   angle = atan2(XF*256, YF*256)
     *   r     = sqrt((XF*256)^2 + (YF*256)^2)
     *   angle += d3 * sin((d  + r * pi/128) * d2)
     *   r     += d6 * sin((d4 + r * pi/128) * d5)
     *   u = sin(angle) * r * 65536 + PosX * 65536   (and likewise v with cos, PosY)
     * then a second pass with YF*256-8 gives the colour difference n4, scaled by
     * 26/24/23 into r/g/b. posX and posY arrive already in Q16.16. */
    for (n = 0; n < my; ++n) {
        for (n2 = 0; n2 < mx; ++n2) {
            const fix16 *ray = rays + (n * mx + n2) * 3;
            fix16 d11 = (fix16)((int64_t)ray[0] << 8);   /* XF * 256 */
            fix16 d12 = (fix16)((int64_t)ray[1] << 8);   /* YF * 256 */
            fix16 angle = fix16_atan2(d11, d12);
            fix16 r = fix16_len3(d11, d12, 0);
            fix16 rpi, d15, angle2, r2, n4;
            AgartBlock *b = out + n * mx + n2;
            rpi = fix16_mul(r, TWIRL_TWO_PI_OVER_256);
            angle += fix16_mul(d3, fix16_sin(fix16_mul(d + rpi, d2)));
            r += fix16_mul(d6, fix16_sin(fix16_mul(d4 + rpi, d5)));
            b->u = fix16_mul(fix16_sin(angle), r) + posX;
            b->v = fix16_mul(fix16_cos(angle), r) + posY;
            /* Second pass: the colour is the change between the two positions. */
            d15 = d12 - ((fix16)8 << 16);
            angle2 = fix16_atan2(d11, d15);
            r2 = fix16_len3(d11, d15, 0);
            rpi = fix16_mul(r2, TWIRL_TWO_PI_OVER_256);
            angle2 += fix16_mul(d3, fix16_sin(fix16_mul(d + rpi, d2)));
            r2 += fix16_mul(d6, fix16_sin(fix16_mul(d4 + rpi, d5)));
            n4 = b->u - (fix16_mul(fix16_sin(angle2), r2) + posX);
            /* The engine does r = jint(n4 * 26), g = n4 * 24, b = n4 * 23 with the
             * raw Q16.16 n4, and drawrecUVRGB then clamps each channel to 0..255
             * before the additive pass. Reproduce that clamp here and quantise the
             * clamped triple to one palette index. */
            b->colour = agart_colour_index(lut,
                                           (int)clamp255((int64_t)n4 * 26),
                                           (int)clamp255((int64_t)n4 * 24),
                                           (int)clamp255((int64_t)n4 * 23));
            b->pad = 0;
        }
    }
}
