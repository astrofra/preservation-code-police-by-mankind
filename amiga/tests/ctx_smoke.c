/* Code Police — indexed-context smoke test (milestone N2), host only.
 *
 * Loads one CPIX part, builds its pair tables, renders a few textures through the
 * four indexed operators into a 320x200 index buffer, quantises nothing (the
 * buffer already holds indices), and writes the result as a 16-colour PPM through
 * the part palette. Build and run on the host:
 *
 *   cc -std=c99 -I amiga/src amiga/tests/ctx_smoke.c amiga/src/agart.c \
 *      amiga/src/agart_ctx.c -o amiga/build/ctx_smoke && \
 *   ./amiga/build/ctx_smoke amiga/build/native-assets 3 amiga/build/ctx-smoke.ppm
 *
 * It is a visual and structural check, not a fidelity check: it proves the indexed
 * path draws and that the operators are wired, before the object graph drives it.
 */
#include "agart.h"
#include "agart_ctx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int write_ppm(const char *path, const AgartContext *c, const AgartPart *part) {
    FILE *f = fopen(path, "wb");
    int i, n = c->width * c->height;
    if (!f) return 1;
    fprintf(f, "P6\n%d %d\n255\n", c->width, c->height);
    for (i = 0; i < n; ++i) {
        const uint8_t *rgb = part->palette_rgb[c->pixels[i] & 0x0F];
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : "amiga/build/native-assets/assets.cpix";
    const char *out = argc > 2 ? argv[2] : "amiga/build/ctx-smoke.ppm";
    AgartPart *part;
    AgartTables tables;
    uint8_t blend[16][16];
    uint8_t *buffer = (uint8_t *)calloc(320 * 200, 1);
    AgartContext ctx;
    int i, drawn = 0;
    part = agart_load(path);
    if (!part) { printf("load FAIL %s\n", path); return 1; }
    printf("pack: %d textures\n", part->count);
    agart_tables_build(&tables, part);
    agart_blend_build(blend, part, 160);
    ctx.pixels = buffer;
    ctx.previous = NULL;
    ctx.width = 320;
    ctx.height = 200;
    ctx.clip.x1 = ctx.clip.y1 = 0;
    ctx.clip.x2 = 320;
    ctx.clip.y2 = 200;
    ctx.rect = ctx.clip;
    agart_clear(&ctx, AGART_BLACK_INDEX);
    /* Draw each texture of the part into a band, using its role. */
    for (i = 0; i < part->count; ++i) {
        const AgartTexture *t = &part->textures[i];
        AgartUV uv[4];
        int x = (i % 4) * 80;
        int y = (i / 4) * 50;
        uv[0].u = 0; uv[0].v = 0;
        uv[1].u = t->width << 16; uv[1].v = 0;
        uv[2].u = 0; uv[2].v = t->height << 16;
        uv[3].u = t->width << 16; uv[3].v = t->height << 16;
        ctx.rect.x1 = x; ctx.rect.y1 = y;
        ctx.rect.x2 = x + 80 < 320 ? x + 80 : 320;
        ctx.rect.y2 = y + 50 < 200 ? y + 50 : 200;
        if (!ctx.rect.x2 || !ctx.rect.y2) continue;
        if (t->role == AGART_ROLE_ADDITIVE) agart_blit_add(&ctx, t, uv, &tables);
        else agart_blit_opaque(&ctx, t, uv);
        ++drawn;
    }
    printf("drew %d textures into %dx%d index buffer\n", drawn, ctx.width, ctx.height);
    if (write_ppm(out, &ctx, part)) { printf("write FAIL %s\n", out); return 1; }
    printf("wrote %s\n", out);
    free(buffer);
    agart_free(part);
    return 0;
}
