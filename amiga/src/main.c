/* Code Police — Amiga port entry point.
 *
 * Milestone 1: load and decode an asset through the platform boundary and
 * print its dimensions and hash. This proves the C host, the file path and the
 * PNG decode on the real target runtime before any engine code is ported.
 */
#include "amiga_platform.h"

#include <stdio.h>

int main(int argc, char **argv) {
    AmigaFileHost host;
    AmigaPlatform platform;
    AmigaImage *image;
    const char *root = argc > 1 ? argv[1] : "native-sdl2/assets";
    const char *name = argc > 2 ? argv[2] : "Images/CARE.gif.png";
    char relative[256];

    amiga_file_host_init(&host, root);
    platform = amiga_file_host_platform(&host);
    snprintf(relative, sizeof relative, "%s", name);
    image = platform.load_image(platform.user, relative);
    if (!image) {
        printf("FAIL: cannot load %s under %s\n", name, root);
        return 1;
    }
    printf("OK: %s %ldx%ld hash=%016llx\n", name, (long)image->width, (long)image->height,
           (unsigned long long)amiga_hash_image(image));
    platform.free_image(platform.user, image);
    return 0;
}
