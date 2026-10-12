/* Code Police — fixed-point scene generators (milestone N3).
 *
 * The three generators (findfloor, findtunnel, findtwirl) fill the per-16x16-block
 * pretable with texture coordinates and a shading colour. In the engine the
 * coordinates are already integers (Q16.16) and the colour is an additive RGB
 * triple consumed by drawrecUVRGB. This module reproduces both with integer maths
 * only (documentation/amiga-native-runtime.md §5 and §4.1):
 *
 *   - coordinates stay Q16.16 (agart_fix.h);
 *   - the shading colour is quantised once per block to a palette index instead of
 *     stored as r/g/b, so the additive operator becomes the pair table add[][].
 *
 * The generators are decoupled from the oracle's rcontext: they read the
 * camera-projected ray of a block and write a packed {u, v, colour} record.
 */
#ifndef AGART_GEN_H
#define AGART_GEN_H

#include "agart_fix.h"

/* atan2(1,0) in Q16.16 is pi/2 * 65536; findtunnel scales angles by this to turn
 * radians into a 65536-wide texture turn. */
#define AGART_TUNNEL_SCALE ((fix16)5340354)

/* One generator output for a block: Q16.16 texture coordinates and the palette
 * index that feeds the additive pair table. */
typedef struct AgartBlock {
    fix16 u, v;
    uint8_t colour;
    uint8_t pad;
} AgartBlock;

/* The fade helper in fixed point. rd1/rv1.. follow the engine's helpfade: a depth
 * threshold and a colour, three times over, one per channel. */
typedef struct AgartFade {
    fix16 Frd1, Frv1, Frd2, Frv2;
    fix16 Fgd1, Fgv1, Fgd2, Fgv2;
    fix16 Fbd1, Fbv1, Fbd2, Fbv2;
} AgartFade;

/* A reverse palette: maps a 5-bit-per-channel RGB cell (r>>3,g>>3,b>>3) to the
 * nearest of the 16 palette entries. Built once per palette. */
typedef struct AgartColourLut {
    uint8_t cell[32768];
} AgartColourLut;

void agart_colour_lut_build(AgartColourLut *lut, const uint8_t palette_rgb[16][3]);

/* Maps an 8-bit RGB triple to the nearest palette index, via the LUT. */
uint8_t agart_colour_index(const AgartColourLut *lut, int r, int g, int b);

/* raycasting.colorclip, indexed: writes the additive palette index for a depth. */
uint8_t agart_colorclip_index(const AgartFade *hf, fix16 depth, const AgartColourLut *lut);

/* raycasting.findfloor over a grid of mx by my blocks. rays[i] holds the block's
 * XF, YF, ZF (camera-projected ray). out[i] receives the packed result. */
void agart_floor_fix(const fix16 *rays, int mx, int my,
                     fix16 posX, fix16 posY, fix16 posZ,
                     fix16 d, fix16 d2, fix16 d3, fix16 d4,
                     const AgartFade *hf, const AgartColourLut *lut, AgartBlock *out);

/* raycasting.findtunnel. rays[i] is XF, YF, ZF; d is the tunnel radius, d2 the
 * texture scale, pos Z the camera Z. */
void agart_tunnel_fix(const fix16 *rays, int mx, int my,
                      fix16 posZ, fix16 d, fix16 d2,
                      const AgartFade *hf, const AgartColourLut *lut, AgartBlock *out);

/* raycasting.findtwirl. The four parameters are the two sine-wave radii/phase
 * pairs (d, d2, d3, d4) and the two amplitude/frequency pairs come through
 * d5 (phase 2) and d6 (radius 2); see the engine's argument order in
 * kEffectX::rendah (FindInto params 2..7). posX/posY offset the result. */
void agart_twirl_fix(const fix16 *rays, int mx, int my,
                     fix16 posX, fix16 posY,
                     fix16 d, fix16 d2, fix16 d3, fix16 d4, fix16 d5, fix16 d6,
                     const AgartColourLut *lut, AgartBlock *out);

#endif /* AGART_GEN_H */
