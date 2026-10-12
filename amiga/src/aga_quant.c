/* Code Police — ARGB to AGA palette index (C). See aga_quant.h. */
#include "aga_quant.h"

void aga_quant_build_lut(const uint8_t palette[16][3], uint8_t *lut) {
    unsigned r, g, b;
    for (r = 0; r < 32; ++r) {
        int rv = (int)(r * 255u / 31u);
        for (g = 0; g < 32; ++g) {
            int gv = (int)(g * 255u / 31u);
            for (b = 0; b < 32; ++b) {
                int bv = (int)(b * 255u / 31u);
                int best = 0, best_d = 1 << 30, i;
                for (i = 0; i < 16; ++i) {
                    int dr = rv - (int)palette[i][0];
                    int dg = gv - (int)palette[i][1];
                    int db = bv - (int)palette[i][2];
                    int d = dr * dr + dg * dg + db * db;
                    if (d < best_d) { best_d = d; best = i; }
                }
                lut[(r << 10) | (g << 5) | b] = (uint8_t)best;
            }
        }
    }
}

void aga_quantize(const uint32_t *argb, uint8_t *indices, size_t count,
                  const uint8_t *lut) {
    size_t i;
    for (i = 0; i < count; ++i) {
        uint32_t p = argb[i];
        unsigned cell = (((p >> 19) & 31u) << 10) | (((p >> 11) & 31u) << 5) |
                        ((p >> 3) & 31u);
        indices[i] = lut[cell];
    }
}
