// Frame-state dumper for the C port's differential work.
//
// Renders the script at fixed ticks and prints the camera, particle-system and
// rectangle positions/counters after each frame. The C port prints the same
// lines; diffing them localizes which effect diverges (camera maths, particle
// simulation, or setrect), because the state is smaller than a pixel hash.
#include "engine.hpp"
#include "assets.hpp"
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>

int main() {
    FilePlatform platform("native-sdl2/assets");
    kraycasting demo(platform, 888);
    demo.init();
    /* Render sequentially (as verify.cpp does) and dump the state at the ticks
     * the C port diverges on, so the two can be compared field by field. */
    const int wanted[] = {11040, 11050, 11060, 11070, 11080};
    for (int tick = 0; tick <= 22200; tick += 2) {
        demo.random.seed(100000 + tick);
        demo.frameAt(jint(tick));
        bool dump = false;
        for (int w : wanted) if (w == tick) dump = true;
        if (!dump) continue;
        auto part = demo.THEkScript->part(jint(tick));
        std::cout << "tick " << tick << " part";
        for (size_t p = 0; p + 1 < demo.THEkScript->kPartXscript.storage->size(); ++p) {
            if (demo.THEkScript->kPartXscript[jint(p)].get() == part.get()) std::cout << " " << p;
        }
        for (auto& item : *demo.partcHash.entries) {
            auto p = std::static_pointer_cast<kparticle>(item.second);
            std::cout << " par " << item.first << " ave " << std::setprecision(10)
                      << p->aveX << " " << p->aveY << " " << p->aveZ
                      << " p0 " << (*p->posx.storage)[0] << " " << (*p->posy.storage)[0]
                      << " " << (*p->posz.storage)[0]
                      << " v0 " << (*p->Vx.storage)[0] << " " << (*p->Vy.storage)[0]
                      << " " << (*p->Vz.storage)[0];
        }
        std::cout << "\n";
    }
    std::cout.flush();
    std::_Exit(0);
}
