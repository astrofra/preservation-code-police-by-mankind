// Headless frame exporter for the Amiga AGA palette study (documentation/amiga-aga-feasibility.md, step 1).
// It reuses the verified engine exactly as verify.cpp does: the same seeded JavaRandom
// sequence (seed 888 at init, 100000 + tick before each frame) and the same frameAt()
// entry point, so the exported PPMs are the reference output of the C++ engine, not a
// re-implementation. No SDL, no window, no Amiga.
#include "engine.hpp"
#include "assets.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

static void savePPM(const std::string& path,const Array<jint>& pixels) {
    std::ofstream file(path,std::ios::binary); file<<"P6\n520 300\n255\n";
    for(jint p:*pixels.storage) for(int shift:{16,8,0}) file.put(char(p.bits>>shift));
}

int main(int argc,char** argv) try {
    if(argc<3 || argc>4) throw std::runtime_error("Usage: dump-frames ASSETS OUTPUT_DIR [STRIDE_TICKS]");
    const int stride = argc==4 ? std::atoi(argv[3]) : 40;
    if(stride<=0) throw std::runtime_error("Stride must be positive");
    FilePlatform platform(argv[1]);
    kraycasting demo(platform,888); demo.init();
    // Resolve each script part object back to its index so frames can be labelled.
    std::map<const kPartX*,int> partIndex;
    auto& parts=*demo.THEkScript->kPartXscript.storage;
    for(size_t i=0;i<parts.size();++i) partIndex[parts[i].get()]=int(i);
    const std::string directory=argv[2];
    int count=0;
    // The script cycle is 222.0 s = 22,200 ticks of the original 10 ms unit.
    for(int tick=0;tick<22200;tick+=stride) {
        demo.random.seed(100000+tick);
        auto pixels=demo.frameAt(jint(tick));
        const int part=partIndex.at(demo.THEkScript->part(jint(tick)).get());
        char name[64]; std::snprintf(name,sizeof name,"tick-%05d-part-%02d.ppm",tick,part);
        savePPM(directory+"/"+name,pixels);
        ++count;
    }
    std::cout<<"Exported "<<count<<" frames at stride "<<stride<<" ticks to "<<directory<<"\n";
    return 0;
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 2; }
