/* Code Police — C2P self-check for the differential validator.
 *
 * Converts a deterministic 320x200 four-colour pattern to four bitplanes and
 * prints the FNV hash of each plane. amiga/validate_c2p.py computes the same
 * planes bit by bit in Python and compares: identical hashes mean the C2P is
 * correct against an independent reference, on the host and on m68k.
 */
#include "c2p4.h"

#include <stdio.h>
#include <stdlib.h>

static uint64_t fnv(const uint8_t *data, size_t n) {
    uint64_t h = 0xcbf29ce484222325ULL;
    size_t i;
    for (i = 0; i < n; ++i) { h ^= data[i]; h *= 0x100000001b3ULL; }
    return h;
}

int c2p_check_run(void) {
    const size_t w = 320, h = 200, ps = w / 8;
    uint8_t *chunky = (uint8_t *)malloc(w * h);
    uint8_t *planes[4];
    int p;
    size_t x, y;
    if (!chunky) return 1;
    for (p = 0; p < 4; ++p) planes[p] = (uint8_t *)calloc(ps * h, 1);
    for (y = 0; y < h; ++y)
        for (x = 0; x < w; ++x)
            chunky[y * w + x] = (uint8_t)((x * 3 + y * 5 + (x >> 4)) & 0x0Fu);
    if (c2p4_byte4(chunky, w, h, w, planes, ps) != 0) { printf("c2p FAIL\n"); return 1; }
    for (p = 0; p < 4; ++p)
        printf("c2p plane %d %016llx\n", p, (unsigned long long)fnv(planes[p], ps * h));
    for (p = 0; p < 4; ++p) free(planes[p]);
    free(chunky);
    return 0;
}
