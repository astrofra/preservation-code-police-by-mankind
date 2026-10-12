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

#include <stdio.h>
#include <stdlib.h>

int render_check_run(void); /* amiga/src/render_check.c */

static int check_asset(AmigaPlatform *platform) {
    AmigaImage *image = platform->load_image(platform->user, "Images/CARE.gif.png");
    if (!image) { printf("asset FAIL\n"); return 1; }
    printf("asset OK: %ldx%ld hash=%016llx\n", (long)image->width,
           (long)image->height, (unsigned long long)amiga_hash_image(image));
    platform->free_image(platform->user, image);
    return 0;
}

static int check_script(AmigaPlatform *platform) {
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
    for (i = 0; i < script->part_count; ++i) {
        printf("  part %d: len=%d start=%d sounds=%d effects=%d codes=",
               i, script->parts[i].time_length, script->parts[i].time_start,
               script->parts[i].sound_count, script->parts[i].effect_count);
        {
            int j;
            for (j = 0; j < script->parts[i].effect_count && j < 64; ++j)
                printf("%s%d", j ? "," : "", script->parts[i].effect_codes[j]);
        }
        printf("\n");
    }
    ks_free(script);
    return 0;
}

int main(int argc, char **argv) {
    AmigaFileHost host;
    AmigaPlatform platform;
    const char *root = argc > 1 ? argv[1] : "native-sdl2/assets";
    int failed = 0;

    amiga_file_host_init(&host, root);
    platform = amiga_file_host_platform(&host);
    failed |= check_asset(&platform);
    failed |= check_script(&platform);
    if (argc > 2 && argv[2][0] == 'r') {
        printf("--- renderer differential ---\n");
        failed |= render_check_run();
    }
    return failed;
}
