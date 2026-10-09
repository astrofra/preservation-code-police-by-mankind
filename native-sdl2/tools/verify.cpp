#include "engine.hpp"
#include "assets.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <iomanip>

struct Reader {
    std::ifstream in;
    explicit Reader(const char* p):in(p,std::ios::binary) { if(!in) throw std::runtime_error("Cannot read reference"); }
    uint64_t integer(int bytes) { uint64_t n=0; for(int i=0;i<bytes;++i) { int b=in.get(); if(b<0) throw std::runtime_error("Truncated reference"); n=(n<<8)|unsigned(b); } return n; }
    double number() { uint64_t n=integer(8); double d; std::memcpy(&d,&n,8); return d; }
    String string() { size_t n=integer(2); String s(n,'\0'); in.read(&s[0],n); return s; }
};
static uint64_t hash(const Array<jint>& pixels) {
    uint64_t h=0xcbf29ce484222325ULL;
    for(jint p:*pixels.storage) { h^=p.bits|0xff000000u; h*=0x100000001b3ULL; } return h;
}
static std::vector<double> state(kraycasting& demo) {
    std::vector<double> v;
    for(auto& item:*demo.CamerHash.entries) {
        auto c=std::static_pointer_cast<kcamera>(item.second);
        for(double d:{c->FOV,c->O1,c->O2,c->O3,c->X,c->Y,c->Z}) v.push_back(d);
    }
    for(auto& item:*demo.partcHash.entries) {
        auto p=std::static_pointer_cast<kparticle>(item.second);
        for(auto a:{p->Ax,p->Ay,p->Az,p->Vx,p->Vy,p->Vz}) v.insert(v.end(),a.storage->begin(),a.storage->end());
        for(double d:{p->aveX,p->aveY,p->aveZ}) v.push_back(d);
        v.insert(v.end(),p->force.storage->begin(),p->force.storage->end()); v.push_back(int32_t(p->nbparticle));
        for(auto a:{p->posx,p->posy,p->posz}) v.insert(v.end(),a.storage->begin(),a.storage->end());
    }
    return v;
}
static void savePPM(const std::string& path,const Array<jint>& pixels) {
    std::ofstream file(path,std::ios::binary); file<<"P6\n520 300\n255\n";
    for(jint p:*pixels.storage) for(int shift:{16,8,0}) file.put(char(p.bits>>shift));
}
int main(int argc,char** argv) try {
    if(argc!=4) throw std::runtime_error("Usage: verify-native ASSETS REFERENCE PCM_DIRECTORY");
    // Meaningful boundary cases, independent of the renderer implementation.
    if(int32_t(jint(0x7fffffff)+jint(1))!=-2147483647-1 || int32_t(jint(-1)>>jint(1))!=-1 ||
       (jint(-1)<<jint(1)).bits!=0xfffffffeu || int32_t(jint(std::nan("")))!=0 ||
       int32_t(jint(INFINITY))!=2147483647 || int32_t(jint(-INFINITY))!=-2147483647-1 ||
       int32_t(jint(-2147483647-1)/jint(-1))!=-2147483647-1) throw std::runtime_error("Java arithmetic boundary");
    FilePlatform platform(argv[1]); platform.recordEvents=true;
    kraycasting demo(platform,888); demo.init();
    for(auto& sound:platform.sounds) {
        auto expected=readBytes(std::string(argv[3])+"/"+static_cast<const std::string&>(sound->name)+".s16be");
        if(expected.size()!=sound->pcm.size()*2) throw std::runtime_error("PCM length");
        for(size_t i=0;i<sound->pcm.size();++i) if(uint16_t(sound->pcm[i])!=((unsigned(expected[i*2])<<8)|expected[i*2+1])) throw std::runtime_error("PCM sample");
    }
    Reader ref(argv[2]); if(ref.integer(4)!=0x43505231) throw std::runtime_error("Reference magic");
    size_t frames=ref.integer(4),different=0,events=0,significant=0; double worst=0;
    for(size_t f=0;f<frames;++f) {
        int tick=int(ref.integer(4)); uint64_t expected=ref.integer(8);
        demo.random.seed(100000+tick); auto pixels=demo.frameAt(jint(tick));
        if(hash(pixels)!=expected) { ++different; if(different<12) std::cout<<"Pixel hash differs at "<<tick<<'\n'; }
        auto values=state(demo); size_t count=ref.integer(4); if(count!=values.size()) throw std::runtime_error("State count");
        for(size_t i=0;i<count;++i) {
            double d=ref.number(); double error=std::fabs(d-values[i])/std::max(1.0,std::fabs(d));
            if(error>worst) worst=error;
            if(!std::isfinite(values[i]) || error>1e-10) { if(significant<5) std::cout<<"State mismatch tick "<<tick<<" index "<<i<<" native "<<values[i]<<" java "<<d<<'\n'; ++significant; }
        }
        size_t commands=ref.integer(4);
        if(commands!=platform.events.size()) throw std::runtime_error("Audio event count at "+std::to_string(tick));
        for(size_t i=0;i<commands;++i) { String name=ref.string(),op=ref.string(); if(platform.events[i]!=std::make_pair(name,op)) throw std::runtime_error("Audio command at "+std::to_string(tick)); }
        events+=commands; platform.events.clear();
        if(ref.integer(1)) {
            size_t delta=0; for(size_t i=0;i<pixels.storage->size();++i) if(((*pixels.storage)[i].bits|0xff000000u)!=ref.integer(4)) ++delta;
            savePPM(std::string(argv[2])+"-native-"+std::to_string(tick)+".ppm",pixels);
            std::cout<<"Capture "<<tick<<": "<<delta<<" differing pixels\n";
        }
    }
    std::cout<<frames<<" frames; "<<different<<" differing hashes; "<<significant<<" significant state differences; max relative state error "<<std::setprecision(17)<<worst<<"; "<<events<<" exact sound commands; 10 exact PCM decodes.\n";
    return different || significant ? 1 : 0;
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 2; }
