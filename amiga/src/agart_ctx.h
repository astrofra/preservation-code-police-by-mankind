/* Code Police — indexed 4-bit render context and raster primitives (milestone N2).
 *
 * documentation/amiga-native-runtime.md §4 to §6. This is the native path: the
 * framebuffer holds one palette index per pixel (0..15), never a 32-bit ARGB
 * value. Every reachable per-pixel colour operator reduces to a small lookup
 * because in the indexed model both operands are palette indices:
 *
 *   opaque copy   dst = tex                      (skips index 15, the key)
 *   alpha blend   dst = blend[alpha][src][dst]  (one 256-byte table per call)
 *   additive      dst = add255[src][dst]        (clamped at 0x0F, not 0xFF)
 *   motion blur   dst = mix[dst_a][dst_b]
 *
 * The additive and blur tables are palette-dependent, so they are rebuilt when a
 * part (and so a palette) changes, never inside the pixel loop. Index 0 is black,
 * the neutral element of the additive table: clamp(black + x) = x, so the
 * transparency key of a texture, which indexes 0 in the additive role, adds
 * nothing, exactly as the original's additive path with 0x000000 would.
 *
 * This file is deliberately standalone C with no floating point: it is the
 * primitive layer that a vasm kernel can later replace behind the same API (§7).
 */
#ifndef AGART_CTX_H
#define AGART_CTX_H

#include <stddef.h>
#include <stdint.h>
#include "agart.h"

/* A rectangle in screen coordinates, half-open [x1, x2) x [y1, y2). */
typedef struct AgartRect {
    int x1, y1, x2, y2;
} AgartRect;

/* A texture coordinate, Q16.16 like the engine's kPixel u/v. */
typedef struct AgartUV {
    int32_t u, v;
} AgartUV;

/* The indexed framebuffer and the clipping context used to draw into it. */
typedef struct AgartContext {
    uint8_t *pixels;           /* current buffer, width * height indices */
    uint8_t *previous;         /* previous frame's buffer, for motion blur */
    int width, height;
    AgartRect clip;            /* current clip rectangle */
    AgartRect rect;            /* the drawing rectangle (sub-rect of clip) */
} AgartContext;

/* The pair tables for one 16-colour palette. add[] is clamp(a+b) to 0x0F; mix[]
 * is the motion-blur average; the alpha blends are built on demand per call. */
typedef struct AgartTables {
    uint8_t add[16][16];
    uint8_t mix[16][16];
} AgartTables;

/* Builds the palette-dependent tables. Call once per part, not per frame. */
void agart_tables_build(AgartTables *t, const AgartPart *part);

/* Builds the 256-entry alpha table for a blend of `alpha`/255 (0..255), i.e.
 * dst = (dst*(255-alpha) + src*alpha) / 255, quantised to the palette. One call
 * per drawrecUVSA; the result indexes [src][dst]. */
void agart_blend_build(uint8_t blend[16][16], const AgartPart *part, int alpha);

/* Framebuffer helpers. */
void agart_clear(AgartContext *c, uint8_t index);
void agart_clear_rect(AgartContext *c, const AgartRect *r, uint8_t index);

/* The four reachable per-pixel operators (§4.1). Each maps the source texture
 * over the context's rect through the affine UV map fixed by the four corners
 * (top-left, top-right, bottom-left, bottom-right), as drawrecUVS does. The
 * additive path consults the tables; opaque skips index 15 (the key). */
void agart_blit_opaque(AgartContext *c, const AgartTexture *t, const AgartUV *uv);
void agart_blit_alpha(AgartContext *c, const AgartTexture *t, const AgartUV *uv,
                      const uint8_t blend[16][16]);
void agart_blit_add(AgartContext *c, const AgartTexture *t, const AgartUV *uv,
                    const AgartTables *tables);
/* Averages the context's rect with the previous frame's buffer, as the engine's
 * MotionBlur does (Chunk and Chunk2 at the same offset, not a spatial shift). */
void agart_motion_blur(AgartContext *c, const AgartTables *tables);

#endif /* AGART_CTX_H */
