/* Code Police — chunky-to-planar, 4 bitplanes (C). See c2p4.h. */
#include "c2p4.h"

int c2p4_byte4(const uint8_t *chunky, size_t width, size_t height,
               size_t chunky_stride, uint8_t *planes[C2P4_PLANES],
               size_t plane_stride) {
    size_t x, y, p;
    if (!chunky || !planes) return 1;
    for (p = 0; p < C2P4_PLANES; ++p) if (!planes[p]) return 1;
    if (width == 0 || height == 0 || (width & 7u) != 0u) return 2;
    if (chunky_stride < width || plane_stride < (width >> 3)) return 3;
    for (y = 0; y < height; ++y) {
        const uint8_t *src = chunky + y * chunky_stride;
        uint8_t *dst0 = planes[0] + y * plane_stride;
        uint8_t *dst1 = planes[1] + y * plane_stride;
        uint8_t *dst2 = planes[2] + y * plane_stride;
        uint8_t *dst3 = planes[3] + y * plane_stride;
        for (x = 0; x < width; x += 8) {
            uint8_t b0 = 0, b1 = 0, b2 = 0, b3 = 0;
            unsigned k;
            for (k = 0; k < 8; ++k) {
                uint8_t v = (uint8_t)(src[x + k] & 0x0Fu);
                uint8_t bit = (uint8_t)(0x80u >> k);
                if (v & 1u) b0 |= bit;
                if (v & 2u) b1 |= bit;
                if (v & 4u) b2 |= bit;
                if (v & 8u) b3 |= bit;
            }
            dst0[x >> 3] = b0;
            dst1[x >> 3] = b1;
            dst2[x >> 3] = b2;
            dst3[x >> 3] = b3;
        }
    }
    return 0;
}
