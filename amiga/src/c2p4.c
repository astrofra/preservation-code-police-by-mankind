/* Code Police — chunky-to-planar, 4 bitplanes (C). See c2p4.h. */
#include "c2p4.h"

#if defined(__m68k__) && defined(AGART_KALMS)
extern void agart_c2p4_kalms(const uint8_t *src, size_t width,
                             size_t height, size_t source_skip,
                             uint8_t *planes[C2P4_PLANES],
                             size_t destination_skip);
#endif

static uint8_t pair_bits[C2P4_PLANES][256];
static int pair_ready;

static void c2p4_init_pairs(void) {
    unsigned p, value;
    if (pair_ready) return;
    for (p = 0; p < C2P4_PLANES; ++p) {
        for (value = 0; value < 256; ++value) {
            uint8_t left = (uint8_t)(value >> 4);
            uint8_t right = (uint8_t)(value & 0x0Fu);
            pair_bits[p][value] = (uint8_t)(((left >> p) & 1u) << 7 |
                                            ((right >> p) & 1u) << 6);
        }
    }
    pair_ready = 1;
}

int c2p4_byte4(const uint8_t *chunky, size_t width, size_t height,
               size_t chunky_stride, uint8_t *planes[C2P4_PLANES],
               size_t plane_stride) {
    size_t x, y, p;
    if (!chunky || !planes) return 1;
    for (p = 0; p < C2P4_PLANES; ++p) if (!planes[p]) return 1;
    if (width == 0 || height == 0 || (width & 7u) != 0u) return 2;
    if (chunky_stride < width || plane_stride < (width >> 3)) return 3;
#if defined(__m68k__) && defined(AGART_KALMS)
    if ((width & 31u) == 0u) {
        /* The indexed renderer maintains the low-nibble invariant, so the
         * 68020+ routine can consume four pixels per long without masking. */
        agart_c2p4_kalms(chunky, width, height, chunky_stride - width,
                         planes, plane_stride - (width >> 3));
        return 0;
    }
#endif
    c2p4_init_pairs();
    for (y = 0; y < height; ++y) {
        const uint8_t *src = chunky + y * chunky_stride;
        uint8_t *dst0 = planes[0] + y * plane_stride;
        uint8_t *dst1 = planes[1] + y * plane_stride;
        uint8_t *dst2 = planes[2] + y * plane_stride;
        uint8_t *dst3 = planes[3] + y * plane_stride;
        for (x = 0; x < width; x += 8) {
            size_t out = x >> 3;
            uint8_t a = (uint8_t)((src[x] << 4) | src[x + 1]);
            uint8_t b = (uint8_t)((src[x + 2] << 4) | src[x + 3]);
            uint8_t c = (uint8_t)((src[x + 4] << 4) | src[x + 5]);
            uint8_t d = (uint8_t)((src[x + 6] << 4) | src[x + 7]);
            dst0[out] = (uint8_t)(pair_bits[0][a] | (pair_bits[0][b] >> 2) |
                                  (pair_bits[0][c] >> 4) | (pair_bits[0][d] >> 6));
            dst1[out] = (uint8_t)(pair_bits[1][a] | (pair_bits[1][b] >> 2) |
                                  (pair_bits[1][c] >> 4) | (pair_bits[1][d] >> 6));
            dst2[out] = (uint8_t)(pair_bits[2][a] | (pair_bits[2][b] >> 2) |
                                  (pair_bits[2][c] >> 4) | (pair_bits[2][d] >> 6));
            dst3[out] = (uint8_t)(pair_bits[3][a] | (pair_bits[3][b] >> 2) |
                                  (pair_bits[3][c] >> 4) | (pair_bits[3][d] >> 6));
        }
    }
    return 0;
}
