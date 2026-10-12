// Differential oracle for the C renderer port (amiga/src/renderer.c).
//
// Drives the verified C++ renderer with a fixed, synthetic scene and prints a
// hash of the resulting chunky buffer. The C port, given the same scene and
// parameters, must print the same hash. This is independent of the script, the
// object graph and the platform, so renderer bugs are caught in isolation.
#include "engine.hpp"
#include "assets.hpp"
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

static uint64_t fnv(const Array<jint>& pixels) {
    uint64_t h = 0xcbf29ce484222325ULL;
    for (jint p : *pixels.storage) { h ^= (uint32_t)(p.bits | 0xff000000u); h *= 0x100000001b3ULL; }
    return h;
}

static uint64_t fnv_pixels(kRenderContext* rc) {
    uint64_t h = 0xcbf29ce484222325ULL;
    for (jint y = 0; y < rc->MaxPreY; ++y) for (jint x = 0; x < rc->MaxPreX; ++x) {
        auto px = rc->Pretable[x][y];
        for (int f : {int(px->u), int(px->v), int(px->r), int(px->g), int(px->b)}) {
            h ^= (uint32_t)f; h *= 0x100000001b3ULL;
        }
    }
    return h;
}

// A synthetic texture: a deterministic gradient with one transparent index.
static Array<jint> make_texture(int w, int h) {
    Array<jint> t(jint(w * h));
    for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
        int v = (x * 7 + y * 13) & 0xff;
        uint32_t px = (uint32_t)(0xFF000000u | (v << 16) | ((v * 2 & 0xff) << 8) | ((255 - v) & 0xff));
        t[jint(y * w + x)] = jint(px);
    }
    t[jint(0)] = jint(0xFFFFFFFFu); // transparent sentinel: int value -1
    return t;
}

struct Scene {
    kraycasting* owner;
    kRenderContext* root;
    raycasting* rcs;
};

static Scene scene_of(Platform& platform, int w, int h) {
    static kraycasting demo(platform, 888);
    demo.init();
    Scene s;
    s.owner = &demo;
    s.root = demo.RenderContext.get();
    s.rcs = demo.rcs.get();
    (void)w; (void)h;
    return s;
}

int main() {
    FilePlatform platform("native-sdl2/assets");
    Scene s = scene_of(platform, 520, 300);
    Ref<kRenderContext> root = s.owner->RenderContext;
    Ref<raycasting> rcs = s.owner->rcs;
    Array<jint> tex_pixels = make_texture(32, 32);
    auto tex = std::make_shared<kTexture>(s.owner, std::make_shared<Image>());
    tex->Modulo = 32; tex->Height = 32; tex->pixels = tex_pixels;

    // 1. Position3D, then hash the pixel table (XF/YF/ZF).
    rcs->Position3D(-40.0, 20.0, 300.0, 0.3, -0.2, 0.1, 300.0, root);
    std::cout << "position3d " << std::hex << fnv_pixels(root.get()) << "\n";

    // 2. fill the buffer deterministically, then MotionBlur.
    for (size_t i = 0; i < root->CurrentChunk.storage->size(); ++i)
        (*root->CurrentChunk.storage)[i] = jint((uint32_t)(0xFF000000u | ((i * 2654435761u) & 0xFFFFFFu)));
    for (size_t i = 0; i < root->Chunk2.storage->size(); ++i)
        (*root->Chunk2.storage)[i] = jint((uint32_t)(0xFF000000u | ((i * 40503u) & 0xFFFFFFu)));
    rcs->MotionBlur(root);
    std::cout << "motionblur " << std::hex << fnv(root->CurrentChunk) << "\n";

    // 3. findtunnel.
    auto hf = std::make_shared<helpfade>(s.owner);
    hf->Frv1 = 10; hf->Frv2 = 200; hf->Frd1 = 100; hf->Frd2 = 5000;
    hf->Fgv1 = 5; hf->Fgv2 = 100; hf->Fgd1 = 100; hf->Fgd2 = 5000;
    hf->Fbv1 = 20; hf->Fbv2 = 160; hf->Fbd1 = 100; hf->Fbd2 = 5000;
    rcs->findtunnel(120.0, 0.5, root, hf);
    std::cout << "findtunnel " << std::hex << fnv_pixels(root.get()) << "\n";

    // 4. findfloor.
    auto hf2 = std::make_shared<helpfade>(s.owner);
    hf2->Frv1 = 30; hf2->Frv2 = 220; hf2->Frd1 = 50; hf2->Frd2 = 4000;
    hf2->Fgv1 = 15; hf2->Fgv2 = 120; hf2->Fgd1 = 50; hf2->Fgd2 = 4000;
    hf2->Fbv1 = 25; hf2->Fbv2 = 180; hf2->Fbd1 = 50; hf2->Fbd2 = 4000;
    rcs->findfloor(200.0, 100.0, 8.0, 12.0, root, hf2);
    std::cout << "findfloor " << std::hex << fnv_pixels(root.get()) << "\n";

    // 5. findtwirl (writes kPixel shading, so hash the pixel table too).
    rcs->findtwirl(0.4, 0.7, 0.9, 1.1, 1.3, 1.5, root);
    std::cout << "findtwirl " << std::hex << fnv_pixels(root.get()) << "\n";

    // 6. SetzoomP then flare3D with a texture that has a transparent index.
    rcs->SetzoomP(0.1, 0.2, 0.7, 0.8, root, tex, 200.0);
    std::cout << "setzoomp " << std::hex << fnv(root->CurrentChunk) << "\n";
    rcs->flare3D(0.0, 0.0, 300.0, 80.0, 80.0, 300.0, root, tex);
    std::cout << "flare3d " << std::hex << fnv(root->CurrentChunk) << "\n";
    std::cout.flush();
    std::_Exit(0); // avoid FilePlatform's mutex during static destruction
}
