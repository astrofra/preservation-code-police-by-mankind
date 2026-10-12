/* Code Police — ARGB to AGA palette index (C).
 *
 * The engine renders 32-bit 0xAARRGGBB; the AGA screen is 16 palette indices.
 * The study's §4.1 makes the renderer indexed, but the verified engine stays
 * ARGB (it is the oracle), so the port quantises its output once per frame with
 * a nearest-colour lookup: a 32768-entry table over the 5-bit RGB cube, built
 * per palette and reused. This keeps the oracle bit-exact while producing the
 * four-plane chunky the Amiga displays.
 */
#ifndef AGA_QUANT_H
#define AGA_QUANT_H

#include <stddef.h>
#include <stdint.h>

#define AGA_QUANT_LUT_SIZE 32768

/* Builds the 5-bit-cube -> palette index table for a 16-colour palette. */
void aga_quant_build_lut(const uint8_t palette[16][3], uint8_t *lut);

/* Quantises ARGB pixels to palette indices. lut is from aga_quant_build_lut. */
void aga_quantize(const uint32_t *argb, uint8_t *indices, size_t count,
                  const uint8_t *lut);

#endif /* AGA_QUANT_H */
