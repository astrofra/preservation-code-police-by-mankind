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

/* Bilinear sample of a block's four corner UVs at pixel (x, y), in Q16.16. The
 * generator grid holds the UV at pixel (16 i, 16 j), so the four corners of a
 * node's cell are its own UV and its three neighbours'. */
static int32_t node_uv(int32_t a, int32_t b, int32_t c, int32_t d,
                       int w, int h, int x, int y) {
    int64_t num;
    if (w <= 0 || h <= 0) return a;
    num = (int64_t)a * (w - x) * (h - y) + (int64_t)b * x * (h - y) +
          (int64_t)c * (w - x) * y + (int64_t)d * x * y;
    return (int32_t)(num / ((int64_t)w * h));
}

/* Per-pixel rule. COPY and ALPHA skip the transparency key; ADD and ADD_BLOCK do
 * not (the additive paths never test it). */
typedef enum { OP_COPY, OP_ALPHA, OP_ADD, OP_ADD_BLOCK } AgartOp;

/* Draws one sub-rectangle [x0,y0)-[x1,y1), clipped to the context clip, sampling
 * the texture through the four corner UVs. `colour` is the shading index for
 * OP_ADD_BLOCK. */
static void blit_block(AgartContext *c, const AgartTexture *t,
                       int x0, int y0, int x1, int y1,
                       const AgartUV *uv, AgartOp op,
                       const uint8_t blend[16][16], const AgartTables *tables,
                       uint8_t colour) {
    int w, h, du, dv, rowdu, rowdv, px, py;
    if (x0 < c->clip.x1) x0 = c->clip.x1;
    if (y0 < c->clip.y1) y0 = c->clip.y1;
    if (x1 > c->clip.x2) x1 = c->clip.x2;
    if (y1 > c->clip.y2) y1 = c->clip.y2;
    w = x1 - x0;
    h = y1 - y0;
    if (w <= 0 || h <= 0 || !t->width || !t->height) return;
    /* UV steps across the block, exactly as drawrecUVS divides the corner
     * difference by the block width and height. */
    du = (uv[1].u - uv[0].u) / w;
    dv = (uv[1].v - uv[0].v) / w;
    rowdu = (uv[2].u - uv[0].u) / h;
    rowdv = (uv[2].v - uv[0].v) / h;
    for (py = 0; py < h; ++py) {
        uint8_t *dst = c->pixels + (size_t)(y0 + py) * c->width;
        int32_t cu = uv[0].u + rowdu * py;
        int32_t cv = uv[0].v + rowdv * py;
        for (px = 0; px < w; ++px) {
            int32_t tu = cu >> 16, tv = cv >> 16;
            uint8_t src = 0;
            int x = x0 + px;
            if (op == OP_ADD_BLOCK) {
                /* drawrecUVRGB addresses its procedural textures with the
                 * original 8-bit wrap: (v & 0xff) * 256 + (u & 0xff).
                 * These textures are kept at 256x256 by build_assets.py. */
                tu &= 0xff;
                tv &= 0xff;
            }
            if (tu >= 0 && tu < t->width && tv >= 0 && tv < t->height)
                src = agart_texel(t, (unsigned)tu, (unsigned)tv);
            if (op == OP_COPY) {
                if (src != AGART_KEY_INDEX) dst[x] = src;
            } else if (op == OP_ALPHA) {
                if (src != AGART_KEY_INDEX) dst[x] = blend[src][dst[x]];
            } else if (op == OP_ADD) {
                dst[x] = tables->add[src][dst[x]];
            } else {
                dst[x] = tables->add[src][colour];
            }
            cu += du;
            cv += dv;
        }
    }
}

/* Draws the context rect in 16x16 blocks, taking each block's corner UVs from the
 * bilinear map of the rect's four corners (the engine's Pretable model). */
static void blit_map(AgartContext *c, const AgartTexture *t, const AgartUV *uv,
                     AgartOp op, const uint8_t blend[16][16],
                     const AgartTables *tables, uint8_t colour) {
    int w = c->rect.x2 - c->rect.x1;
    int h = c->rect.y2 - c->rect.y1;
    int bx, by, cols, rows;
    if (w <= 0 || h <= 0) return;
    rows = (h + 15) >> 4;
    cols = (w + 15) >> 4;
    for (by = 0; by < rows; ++by) {
        int bh = (by * 16 + 16 <= h) ? 16 : (h - by * 16);
        for (bx = 0; bx < cols; ++bx) {
            int bw = (bx * 16 + 16 <= w) ? 16 : (w - bx * 16);
            AgartUV block[4];
            int gx = bx * 16, gy = by * 16;
            block[0].u = node_uv(uv[0].u, uv[1].u, uv[2].u, uv[3].u, w, h, gx, gy);
            block[0].v = node_uv(uv[0].v, uv[1].v, uv[2].v, uv[3].v, w, h, gx, gy);
            block[1].u = node_uv(uv[0].u, uv[1].u, uv[2].u, uv[3].u, w, h, gx + bw, gy);
            block[1].v = node_uv(uv[0].v, uv[1].v, uv[2].v, uv[3].v, w, h, gx + bw, gy);
            block[2].u = node_uv(uv[0].u, uv[1].u, uv[2].u, uv[3].u, w, h, gx, gy + bh);
            block[2].v = node_uv(uv[0].v, uv[1].v, uv[2].v, uv[3].v, w, h, gx, gy + bh);
            blit_block(c, t, c->rect.x1 + gx, c->rect.y1 + gy,
                       c->rect.x1 + gx + bw, c->rect.y1 + gy + bh,
                       block, op, blend, tables, colour);
        }
    }
}

void agart_blit_opaque(AgartContext *c, const AgartTexture *t, const AgartUV *uv) {
    blit_map(c, t, uv, OP_COPY, NULL, NULL, 0);
}

void agart_blit_alpha(AgartContext *c, const AgartTexture *t, const AgartUV *uv,
                      const uint8_t blend[16][16]) {
    blit_map(c, t, uv, OP_ALPHA, blend, NULL, 0);
}

void agart_blit_add(AgartContext *c, const AgartTexture *t, const AgartUV *uv,
                    const AgartTables *tables) {
    blit_map(c, t, uv, OP_ADD, NULL, tables, 0);
}

void agart_blit_add_block(AgartContext *c, const AgartTexture *t, const AgartUV *uv,
                          const AgartTables *tables, uint8_t block_index) {
    blit_map(c, t, uv, OP_ADD_BLOCK, NULL, tables, block_index);
}

/* The generator grid path: the engine's draw() steps the context in 16x16 blocks
 * and reads four nodes of the Pretable per block. Here the four nodes are the four
 * corner UVs of each grid cell, and the shading colour is the grid cell's own. */
static void blit_grid(AgartContext *c, const AgartTexture *t,
                      const AgartBlock *grid, int mx, int my, AgartOp op,
                      const uint8_t blend[16][16], const AgartTables *tables) {
    int bx, by;
    for (by = 0; by < my; ++by) {
        for (bx = 0; bx < mx; ++bx) {
            int x0 = c->rect.x1 + bx * 16;
            int y0 = c->rect.y1 + by * 16;
            int x1 = x0 + 16, y1 = y0 + 16;
            int xm = bx + 1 < mx ? bx + 1 : bx;
            int ym = by + 1 < my ? by + 1 : by;
            const AgartBlock *tl = &grid[by * mx + bx];
            const AgartBlock *tr = &grid[by * mx + xm];
            const AgartBlock *bl = &grid[ym * mx + bx];
            AgartUV uv[4];
            uv[0].u = tl->u; uv[0].v = tl->v;
            uv[1].u = tr->u; uv[1].v = tr->v;
            uv[2].u = bl->u; uv[2].v = bl->v;
            uv[3].u = tr->u; uv[3].v = bl->v;
            if (x1 > c->rect.x2) x1 = c->rect.x2;
            if (y1 > c->rect.y2) y1 = c->rect.y2;
            blit_block(c, t, x0, y0, x1, y1, uv, op, blend, tables, tl->colour);
        }
    }
}

void agart_grid_colour(AgartContext *c, const AgartTexture *t,
                       const AgartBlock *grid, int mx, int my,
                       const AgartTables *tables) {
    blit_grid(c, t, grid, mx, my, OP_ADD_BLOCK, NULL, tables);
}

void agart_grid_alpha(AgartContext *c, const AgartTexture *t,
                      const AgartBlock *grid, int mx, int my,
                      const uint8_t blend[16][16]) {
    blit_grid(c, t, grid, mx, my, OP_ALPHA, blend, NULL);
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
