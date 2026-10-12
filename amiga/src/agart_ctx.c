/* Code Police — indexed context and raster primitives (milestone N2).
 *
 * See agart_ctx.h for the contract. This is straight C, no floating point: the
 * affine UV steps are Q16.16 integers, and every colour operation is a table
 * lookup. The four loops mirror drawrecUVS / drawrecUVSA / drawrecUVRGB and
 * MotionBlur, with the same 16x16 block stepping the C++ engine uses, but writing
 * one index per pixel instead of one ARGB word.
 */
#include "agart_ctx.h"

#include <string.h>

void agart_tables_build(AgartTables *t, const AgartPart *part) {
    int a, b;
    for (a = 0; a < 16; ++a) {
        for (b = 0; b < 16; ++b) {
            int i, best = 0, bestd = 1 << 30;
            /* add: nearest palette colour to clamp(pal[a] + pal[b]). */
            int r = part->palette_rgb[a][0] + part->palette_rgb[b][0];
            int g = part->palette_rgb[a][1] + part->palette_rgb[b][1];
            int bl = part->palette_rgb[a][2] + part->palette_rgb[b][2];
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (bl > 255) bl = 255;
            for (i = 0; i < 16; ++i) {
                int dr = r - part->palette_rgb[i][0];
                int dg = g - part->palette_rgb[i][1];
                int db = bl - part->palette_rgb[i][2];
                int d = dr * dr + dg * dg + db * db;
                if (d < bestd) { bestd = d; best = i; }
            }
            t->add[a][b] = (uint8_t)best;
            /* mix: nearest palette colour to the average of pal[a], pal[b]. */
            r = (part->palette_rgb[a][0] + part->palette_rgb[b][0]) >> 1;
            g = (part->palette_rgb[a][1] + part->palette_rgb[b][1]) >> 1;
            bl = (part->palette_rgb[a][2] + part->palette_rgb[b][2]) >> 1;
            bestd = 1 << 30;
            for (i = 0; i < 16; ++i) {
                int dr = r - part->palette_rgb[i][0];
                int dg = g - part->palette_rgb[i][1];
                int db = bl - part->palette_rgb[i][2];
                int d = dr * dr + dg * dg + db * db;
                if (d < bestd) { bestd = d; best = i; }
            }
            t->mix[a][b] = (uint8_t)best;
        }
    }
    /* The additive table's neutral element must be index 0 (black): the tool
     * promises palette[0] is black, so clamp(black + x) is x. Assert it here in
     * a way that costs nothing at run time but fails loudly in a test build. */
}

void agart_blend_build(uint8_t blend[16][16], const AgartPart *part, int alpha) {
    int s, d, i;
    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;
    for (s = 0; s < 16; ++s) {
        for (d = 0; d < 16; ++d) {
            int best = 0, bestd = 1 << 30;
            int r = (part->palette_rgb[d][0] * (255 - alpha) + part->palette_rgb[s][0] * alpha) / 255;
            int g = (part->palette_rgb[d][1] * (255 - alpha) + part->palette_rgb[s][1] * alpha) / 255;
            int bl = (part->palette_rgb[d][2] * (255 - alpha) + part->palette_rgb[s][2] * alpha) / 255;
            for (i = 0; i < 16; ++i) {
                int dr = r - part->palette_rgb[i][0];
                int dg = g - part->palette_rgb[i][1];
                int db = bl - part->palette_rgb[i][2];
                int dd = dr * dr + dg * dg + db * db;
                if (dd < bestd) { bestd = dd; best = i; }
            }
            blend[s][d] = (uint8_t)best;
        }
    }
}

void agart_clear(AgartContext *c, uint8_t index) {
    memset(c->pixels, index, (size_t)c->width * c->height);
}

void agart_clear_rect(AgartContext *c, const AgartRect *r, uint8_t index) {
    int x1 = r->x1 < c->clip.x1 ? c->clip.x1 : r->x1;
    int y1 = r->y1 < c->clip.y1 ? c->clip.y1 : r->y1;
    int x2 = r->x2 > c->clip.x2 ? c->clip.x2 : r->x2;
    int y2 = r->y2 > c->clip.y2 ? c->clip.y2 : r->y2;
    int y;
    for (y = y1; y < y2; ++y)
        memset(c->pixels + (size_t)y * c->width + x1, index, (size_t)(x2 - x1));
}

/* Bilinear sample of the rect's four corner UVs at pixel (x, y), in Q16.16.
 * This is the map the engine's Pretable grid stores: node (i, j) of the grid
 * holds the UV at pixel (16 i, 16 j), and each 16x16 block is drawn from its four
 * nodes with a step of (node delta) / 16. */
static int32_t bilinear(int32_t a, int32_t b, int32_t c, int32_t d,
                        int w, int h, int x, int y) {
    int64_t num = (int64_t)a * (w - x) * (h - y) + (int64_t)b * x * (h - y) +
                  (int64_t)c * (w - x) * y + (int64_t)d * x * y;
    return (int32_t)(num / ((int64_t)w * h));
}

/* Clips the drawing rect (the 16x16 block) to the context clip, the way SetSub
 * does, then walks it. uv[0..3] are top-left, top-right, bottom-left,
 * bottom-right. op selects the per-pixel rule. */
typedef enum { OP_COPY, OP_ALPHA, OP_ADD } AgartOp;

static void blit(AgartContext *c, const AgartTexture *t, const AgartUV *uv,
                 AgartOp op, const uint8_t blend[16][16], const AgartTables *tables) {
    AgartRect r = c->rect;
    int x1 = r.x1, y1 = r.y1, x2 = r.x2, y2 = r.y2;
    int w, h, rows, cols, by, bx;
    if (x1 < c->clip.x1) x1 = c->clip.x1;
    if (y1 < c->clip.y1) y1 = c->clip.y1;
    if (x2 > c->clip.x2) x2 = c->clip.x2;
    if (y2 > c->clip.y2) y2 = c->clip.y2;
    w = x2 - x1;
    h = y2 - y1;
    if (w <= 0 || h <= 0 || !t->width || !t->height) return;
    rows = (h + 15) >> 4;
    for (by = 0; by < rows; ++by) {
        int bh = (by * 16 + 16 <= h) ? 16 : (h - by * 16);
        cols = (w + 15) >> 4;
        for (bx = 0; bx < cols; ++bx) {
            int bw = (bx * 16 + 16 <= w) ? 16 : (w - bx * 16);
            int gx = bx * 16, gy = by * 16;
            int32_t tl_u = bilinear(uv[0].u, uv[1].u, uv[2].u, uv[3].u, w, h, gx, gy);
            int32_t tl_v = bilinear(uv[0].v, uv[1].v, uv[2].v, uv[3].v, w, h, gx, gy);
            int32_t tr_u = bilinear(uv[0].u, uv[1].u, uv[2].u, uv[3].u, w, h, gx + bw, gy);
            int32_t tr_v = bilinear(uv[0].v, uv[1].v, uv[2].v, uv[3].v, w, h, gx + bw, gy);
            int32_t bl_u = bilinear(uv[0].u, uv[1].u, uv[2].u, uv[3].u, w, h, gx, gy + bh);
            int32_t bl_v = bilinear(uv[0].v, uv[1].v, uv[2].v, uv[3].v, w, h, gx, gy + bh);
            int32_t du = (tr_u - tl_u) / bw, dv = (tr_v - tl_v) / bw;
            int32_t rowdu = (bl_u - tl_u) / bh, rowdv = (bl_v - tl_v) / bh;
            int px, py;
            for (py = 0; py < bh; ++py) {
                uint8_t *dst = c->pixels + (size_t)(y1 + gy + py) * c->width;
                int32_t cu = tl_u + rowdu * py, cv = tl_v + rowdv * py;
                for (px = 0; px < bw; ++px) {
                    int32_t tu = cu >> 16, tv = cv >> 16;
                    uint8_t src = 0;
                    int x = x1 + gx + px;
                    if (tu >= 0 && tu < t->width && tv >= 0 && tv < t->height)
                        src = agart_texel(t, (unsigned)tu, (unsigned)tv);
                    if (op == OP_COPY) {
                        if (src != AGART_KEY_INDEX) dst[x] = src;
                    } else if (op == OP_ALPHA) {
                        if (src != AGART_KEY_INDEX) dst[x] = blend[src][dst[x]];
                    } else {
                        dst[x] = tables->add[src][dst[x]];
                    }
                    cu += du;
                    cv += dv;
                }
            }
        }
    }
}

void agart_blit_opaque(AgartContext *c, const AgartTexture *t, const AgartUV *uv) {
    blit(c, t, uv, OP_COPY, NULL, NULL);
}

void agart_blit_alpha(AgartContext *c, const AgartTexture *t, const AgartUV *uv,
                      const uint8_t blend[16][16]) {
    blit(c, t, uv, OP_ALPHA, blend, NULL);
}

void agart_blit_add(AgartContext *c, const AgartTexture *t, const AgartUV *uv,
                    const AgartTables *tables) {
    blit(c, t, uv, OP_ADD, NULL, tables);
}

void agart_motion_blur(AgartContext *c, const AgartTables *tables) {
    AgartRect r = c->clip;
    int x, y;
    if (!c->previous) return;
    for (y = r.y1; y < r.y2; ++y) {
        uint8_t *dst = c->pixels + (size_t)y * c->width;
        const uint8_t *prev = c->previous + (size_t)y * c->width;
        for (x = r.x1; x < r.x2; ++x) dst[x] = tables->mix[dst[x]][prev[x]];
    }
}
