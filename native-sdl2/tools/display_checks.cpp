#include "display.hpp"
#include <iostream>

int main() try {
    const int cases[][6]={{1280,720,1248,720,16,0},{800,600,800,462,0,69},
        {1920,1080,1872,1080,24,0},{1024,768,1024,591,0,88},
        {600,1000,600,346,0,327},{3440,1440,2496,1440,472,0},{1040,600,1040,600,0,0}};
    for(const auto& c:cases) {
        auto w=Display::windowSize(c[0],c[1]); auto r=Display::viewport(c[0],c[1]);
        if(w.width!=c[2] || w.height!=c[3] || r.width!=c[2] || r.height!=c[3] || r.x!=c[4] || r.y!=c[5])
            throw std::runtime_error("Geometry mismatch");
    }
    for(const auto& value:{"0x720","-1x720","1280","1280x","12x3x4","999999999999x720","1280x720bad"}) {
        bool rejected=false; try { Display::resolution(value); } catch(const std::exception&) { rejected=true; }
        if(!rejected) throw std::runtime_error("Accepted invalid resolution");
    }
    for(int w=1;w<4096;w+=13) for(int h=1;h<4096;h+=17) {
        auto fit=Display::windowSize(w,h); auto again=Display::windowSize(fit.width,fit.height);
        if(fit.width>w || fit.height>h || fit.width!=again.width || fit.height!=again.height)
            throw std::runtime_error("Fit bounds or idempotence");
    }
    std::cout<<"PASS: seven reference geometries, malformed CLI dimensions, fit bounds and resize idempotence.\n";
    return 0;
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
