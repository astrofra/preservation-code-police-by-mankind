/* Code Police — AGA preview (C).
 *
 * Renders a whole-script frame through the verified engine, quantises it to the
 * part's 16-colour AGA palette, applies that palette back to RGB and writes a
 * PPM. This is what the AGA screen will show: the real engine output, in 16
 * colours, with the palette the Copper would load. It makes the display path
 * inspectable without an Amiga.
 */
#include "objects.h"
#include "aga_quant.h"
#include "aga_palette.h"

#include <stdio.h>
#include <stdlib.h>

int aga_preview_run(jworld *w, int tick, const char *path) {
    static uint8_t lut[AGA_QUANT_LUT_SIZE];
    uint8_t *indices;
    FILE *f;
    int part_index;
    int i, count = w->width * w->height;
    int32_t t;

    /* Render sequentially up to the requested tick, as the oracle does. */
    for (t = 0; t <= tick; t = jadd(t, 2)) jworld_frame(w, t);

    part_index = 0;
    for (i = 0; i < w->part_count - 1; ++i) {
        if (jworld_part(w, tick) == w->parts[i]) part_index = i;
    }
    aga_quant_build_lut(AGA_PALETTES[part_index], lut);

    indices = (uint8_t *)malloc((size_t)count);
    if (!indices) return 1;
    aga_quantize((const uint32_t *)w->root.Chunk2, indices, (size_t)count, lut);

    f = fopen(path, "wb");
    if (!f) { free(indices); return 1; }
    fprintf(f, "P6\n520 300\n255\n");
    for (i = 0; i < count; ++i) {
        const uint8_t *c = AGA_PALETTES[part_index][indices[i]];
        fputc(c[0], f); fputc(c[1], f); fputc(c[2], f);
    }
    fclose(f);
    free(indices);
    printf("preview tick %d part %d -> %s\n", tick, part_index, path);
    return 0;
}
