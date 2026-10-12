/* Code Police — Amiga port entry point.
 *
 * Milestone 1: load and decode an asset through the platform boundary and
 * print its dimensions and hash. This proves the C host, the file path and the
 * PNG decode on the real target runtime before any engine code is ported.
 * Milestone 2: parse the KarateScript and print the counts the verified engine
 * reports, so the tokeniser can be compared against evidence/diagnostic.json.
 */
#include "amiga_platform.h"
#include "kscript.h"
#include "objects.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int render_check_run(void); /* amiga/src/render_check.c */
int c2p_check_run(void);    /* amiga/src/c2p_check.c */
int aga_preview_run(jworld *w, int tick, const char *path); /* aga_preview.c */

static int check_asset(AmigaPlatform *platform) {
    AmigaImage *image = platform->load_image(platform->user, "Images/CARE.gif.png");
    if (!image) { printf("asset FAIL\n"); return 1; }
    printf("asset OK: %ldx%ld hash=%016llx\n", (long)image->width,
           (long)image->height, (unsigned long long)amiga_hash_image(image));
    platform->free_image(platform->user, image);
    return 0;
}

static int check_script(AmigaPlatform *platform, KsScript **out) {
    size_t size = 0;
    uint8_t *bytes = platform->read_file(platform->user, "scene.txt", &size);
    KsScript *script;
    int i;
    if (!bytes) { printf("script FAIL: cannot read scene.txt\n"); return 1; }
    script = ks_parse((const char *)bytes, (int)size);
    platform->free_file(platform->user, bytes);
    if (!script) { printf("script FAIL: parse\n"); return 1; }
    printf("script: images=%d sounds=%d tables=%d ticles=%d cams=%d rects=%d "
           "parts=%d loop=%d\n",
           script->image_count, script->sound_count, script->table_count,
           script->ticle_count, script->camera_count, script->rect_count,
           script->part_count, script->end_pref);
    for (i = 0; i < script->part_count; ++i)
        printf("  part %d: len=%d start=%d sounds=%d effects=%d\n",
               i, script->parts[i].time_length, script->parts[i].time_start,
               script->parts[i].sound_count, script->parts[i].effect_count);
    *out = script;
    return 0;
}

static int check_graph(const KsScript *script, const AmigaPlatform *platform, int render_frames) {
    jworld *w = jworld_create(script, platform);
    int i;
    if (!w) { printf("graph FAIL\n"); return 1; }
    printf("graph: textures=%d tables=%d cams=%d particles=%d rects=%d parts=%d "
           "end_date=%d\n", w->texture_count, w->table_count, w->camera_count,
           w->particle_count, w->rect_count, w->part_count - 1, w->end_date);
    if (render_frames) {
        /* Render the whole 222 s cycle sequentially, as verify.cpp does (every
         * 2 ticks), so frame T is drawn after T/2 earlier frames and the
         * persistent object state matches the oracle. Print every frame's hash
         * for comparison with native-sdl2/build/reference.bin. */
        int wanted[14];
        jint t;
        for (i = 0; i < 12; ++i) wanted[i] = i * 2000;
        wanted[12] = 11050; wanted[13] = 11070;
        for (t = 0; t <= 22200; t = jadd(t, 2)) {
            jworld_frame(w, t);
            printf("frame %ld %016llx\n", (long)t,
                   (unsigned long long)jworld_hash(w));
            for (i = 0; i < 14; ++i) {
                if (wanted[i] == (int)t) {
                    jpart *part = jworld_part(w, t);
                    int k;
                    printf("tick %ld part", (long)t);
                    for (k = 0; k < w->part_count - 1; ++k)
                        if (w->parts[k] == part) printf(" %d", k);
                    for (k = 0; k < w->particle_count; ++k) {
                        jparticle *p = w->particles[k];
                        printf(" par %s ave %.10g %.10g %.10g p0 %.10g %.10g %.10g",
                               w->particle_names[k], p->aveX, p->aveY, p->aveZ,
                               p->posx[0], p->posy[0], p->posz[0]);
                    }
                    printf("\n");
                }
            }
        }
    }
    jworld_free(w);
    return 0;
}

int main(int argc, char **argv) {
    AmigaFileHost host;
    AmigaPlatform platform;
    KsScript *script = NULL;
    const char *root = argc > 1 ? argv[1] : "native-sdl2/assets";
    const char *mode = argc > 2 ? argv[2] : "";
    int failed = 0;

#ifdef AGA_SCREEN
    { FILE *m = fopen("CP:ran.txt", "w");
      if (m) { fprintf(m, "main entered, argc=%d argv1=%s\n", argc, root); fclose(m); } }
#endif
    amiga_file_host_init(&host, root);
    platform = amiga_file_host_platform(&host);
    setvbuf(stdout, NULL, _IONBF, 0);
    failed |= check_asset(&platform);
    failed |= check_script(&platform, &script);
    /* The screen mode builds its own world below, so skip the throwaway one. */
    if (!failed && script && strcmp(mode, "screen"))
        failed |= check_graph(script, &platform, !strcmp(mode, "frames"));
    if (!strcmp(mode, "render")) {
        printf("--- renderer differential ---\n");
        failed |= render_check_run();
    }
    if (!strcmp(mode, "c2p")) {
        failed |= c2p_check_run();
    }
    if (!strcmp(mode, "preview") && script) {
        jworld *w = jworld_create(script, &platform);
        if (w) {
            failed |= aga_preview_run(w, 6000, "amiga/build/preview-6000.ppm");
            failed |= aga_preview_run(w, 20000, "amiga/build/preview-20000.ppm");
            jworld_free(w);
        } else {
            failed |= 1;
        }
    }
    if (!strcmp(mode, "screen") && script) {
        jworld *w;
#ifdef AGA_SCREEN
        if (!amiga_screen_open_loading()) {
            failed |= 1;
            w = NULL;
        } else {
            amiga_screen_set_message("Building scene graph...");
            w = jworld_create(script, &platform);
        }
#else
        w = jworld_create(script, &platform);
#endif
#ifdef AGA_SCREEN
        if (w) amiga_screen_set_message("Scene ready. Rendering...");
#endif
        if (w) { failed |= amiga_screen_run(w); jworld_free(w); }
        else { amiga_screen_close(); failed |= 1; }
    }
    ks_free(script);
    return failed;
}
