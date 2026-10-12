/* Code Police — renderer differential harness (C side).
 *
 * Runs the same synthetic scene as native-sdl2/tools/render_oracle.cpp through
 * the C renderer and prints the same hashes. compare_render.py runs both and
 * diffs them: identical output means the C port is bit-exact with the verified
 * C++ engine for Position3D, MotionBlur, findtunnel, findfloor, findtwirl,
 * SetzoomP and flare3D.
 */
#include "renderer.h"
#include "amiga_platform.h"

#include <stdio.h>
#include <stdlib.h>

static uint64_t fnv_buffer(const int32_t *pixels, int count) {
    uint64_t h = 0xcbf29ce484222325ULL;
    int i;
    for (i = 0; i < count; ++i) {
        h ^= (uint64_t)((uint32_t)pixels[i] | 0xff000000u);
        h *= 0x100000001b3ULL;
    }
    return h;
}

static uint64_t fnv_pixels(const rcontext *rc) {
    uint64_t h = 0xcbf29ce484222325ULL;
    jint y, x;
    for (y = 0; y < rc->MaxPreY; ++y) {
        for (x = 0; x < rc->MaxPreX; ++x) {
            const kpixel *px = rc->Pretable[x + jmul(y, rc->PretableX)];
            jint f[5];
            int k;
            f[0] = px->u; f[1] = px->v; f[2] = px->r; f[3] = px->g; f[4] = px->b;
            for (k = 0; k < 5; ++k) { h ^= (uint64_t)(uint32_t)f[k]; h *= 0x100000001b3ULL; }
        }
    }
    return h;
}

static void fill_texture(texture *tx) {
    int w = tx->Modulo, h = tx->Height, x, y;
    tx->pixels = (int32_t *)malloc((size_t)w * (size_t)h * sizeof(int32_t));
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            int v = (x * 7 + y * 13) & 0xff;
            tx->pixels[y * w + x] = (int32_t)(0xFF000000u | ((uint32_t)v << 16) |
                (((uint32_t)(v * 2) & 0xff) << 8) | (uint32_t)(255 - v));
        }
    }
    tx->pixels[0] = (int32_t)0xFFFFFFFFu; /* int -1: transparent */
}

int render_check_run(void) {
    rcontext *root = (rcontext *)malloc(sizeof(rcontext));
    rcaster rcs;
    texture tex;
    helpfade hf, hf2;
    int i, count;
    if (!root) return 1;

    rcr_init_root(root, 520, 300);
    rcs_init(&rcs, root);

    tex.Modulo = 32; tex.Height = 32; tex.pixels = NULL;
    fill_texture(&tex);

    rcs_position3d(&rcs, -40.0, 20.0, 300.0, 0.3, -0.2, 0.1, 300.0, root);
    printf("position3d %llx\n", (unsigned long long)fnv_pixels(root));

    count = 520 * 300;
    for (i = 0; i < count; ++i)
        root->CurrentChunk[i] = (int32_t)(0xFF000000u | (((uint32_t)i * 2654435761u) & 0xFFFFFFu));
    for (i = 0; i < count; ++i)
        root->Chunk2[i] = (int32_t)(0xFF000000u | (((uint32_t)i * 40503u) & 0xFFFFFFu));
    rcs_motion_blur(&rcs, root);
    printf("motionblur %llx\n", (unsigned long long)fnv_buffer(root->CurrentChunk, count));

    hf.Frv1 = 10; hf.Frv2 = 200; hf.Frd1 = 100; hf.Frd2 = 5000;
    hf.Fgv1 = 5;  hf.Fgv2 = 100; hf.Fgd1 = 100; hf.Fgd2 = 5000;
    hf.Fbv1 = 20; hf.Fbv2 = 160; hf.Fbd1 = 100; hf.Fbd2 = 5000;
    rcs_findtunnel(&rcs, 120.0, 0.5, root, &hf);
    printf("findtunnel %llx\n", (unsigned long long)fnv_pixels(root));

    hf2.Frv1 = 30; hf2.Frv2 = 220; hf2.Frd1 = 50; hf2.Frd2 = 4000;
    hf2.Fgv1 = 15; hf2.Fgv2 = 120; hf2.Fgd1 = 50; hf2.Fgd2 = 4000;
    hf2.Fbv1 = 25; hf2.Fbv2 = 180; hf2.Fbd1 = 50; hf2.Fbd2 = 4000;
    rcs_findfloor(&rcs, 200.0, 100.0, 8.0, 12.0, root, &hf2);
    printf("findfloor %llx\n", (unsigned long long)fnv_pixels(root));

    rcs_findtwirl(&rcs, 0.4, 0.7, 0.9, 1.1, 1.3, 1.5, root);
    printf("findtwirl %llx\n", (unsigned long long)fnv_pixels(root));

    rcs_setzoomp(&rcs, 0.1, 0.2, 0.7, 0.8, root, &tex, 200.0);
    printf("setzoomp %llx\n", (unsigned long long)fnv_buffer(root->CurrentChunk, count));

    rcs_flare3d(&rcs, 0.0, 0.0, 300.0, 80.0, 80.0, 300.0, root, &tex);
    printf("flare3d %llx\n", (unsigned long long)fnv_buffer(root->CurrentChunk, count));

    free(tex.pixels);
    free(rcs.OneRec);
    rcr_free(root);
    free(root);
    return 0;
}
