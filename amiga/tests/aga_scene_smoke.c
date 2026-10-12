/* Code Police — indexed scene smoke test (milestone N4), host only.
 *
 * Builds the real object graph from the script and assets (objects.c), then renders
 * whole-script frames through the indexed scene interpreter (aga_scene.c) and writes
 * them as 16-colour PPMs. This is the first time the effect tree drives the indexed
 * renderer, and it is the visual check that N4 is on track.
 *
 * Build:
 *   cc -std=c99 -I amiga/src -I native-sdl2/vendor/stb \
 *      amiga/tests/aga_scene_smoke.c amiga/src/amiga_platform.c \
 *      amiga/src/kscript.c amiga/src/renderer.c amiga/src/objects.c \
 *      amiga/src/agart.c amiga/src/agart_ctx.c amiga/src/agart_fix.c \
 *      amiga/src/agart_trig.c amiga/src/agart_gen.c amiga/src/agart_proj.c \
 *      amiga/src/aga_scene.c -lm -o amiga/build/aga_scene_smoke
 */
#include "aga_scene.h"
#include "amiga_platform.h"
#include "kscript.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int write_ppm(const char *path, const AgartScene *s, const AgartPart *pack) {
    FILE *f = fopen(path, "wb");
    int i, n = s->width * s->height;
    if (!f) return 1;
    fprintf(f, "P6\n%d %d\n255\n", s->width, s->height);
    for (i = 0; i < n; ++i) {
        const uint8_t *rgb = pack->palette_rgb[s->front[i] & 0x0F];
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
    return 0;
}

int main(int argc, char **argv) {
    const char *root = argc > 1 ? argv[1] : "native-sdl2/assets";
    const char *pack_path = argc > 2 ? argv[2] : "amiga/build/native-assets/assets.cpix";
    AmigaFileHost host;
    AmigaPlatform platform;
    KsScript *script;
    jworld *w;
    AgartPart *pack;
    AgartScene *s;
    const int ticks[] = { 6000, 12000, 20000 };
    int k;

    amiga_file_host_init(&host, root);
    platform = amiga_file_host_platform(&host);
    {
        size_t size = 0;
        uint8_t *bytes = platform.read_file(platform.user, "scene.txt", &size);
        if (!bytes) { printf("no scene.txt\n"); return 1; }
        script = ks_parse((const char *)bytes, (int)size);
        platform.free_file(platform.user, bytes);
    }
    if (!script) { printf("parse FAIL\n"); return 1; }
    pack = agart_load(pack_path);
    if (!pack) { printf("pack FAIL %s\n", pack_path); return 1; }
    w = jworld_create(script, &platform);
    if (!w) { printf("graph FAIL\n"); return 1; }
    s = aga_scene_create(pack, 320, 200);
    if (!s) { printf("scene FAIL\n"); return 1; }

    for (k = 0; k < 3; ++k) {
        char path[256];
        int draws = aga_scene_frame(s, w, ticks[k]);
        printf("tick %5d: %d primitives\n", ticks[k], draws);
        snprintf(path, sizeof path, "amiga/build/scene-%05d.ppm", ticks[k]);
        write_ppm(path, s, pack);
    }
    aga_scene_free(s);
    jworld_free(w);
    agart_free(pack);
    ks_free(script);
    return 0;
}
