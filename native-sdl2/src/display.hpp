#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <stdexcept>

namespace Display {
struct Size { int width, height; };
struct Rect { int x, y, width, height; };
inline int dimension(const std::string& text) {
    if(text.empty() || text.find_first_not_of("0123456789")!=std::string::npos)
        throw std::runtime_error("--resolution requires WIDTHxHEIGHT");
    unsigned long n=std::stoul(text);
    if(n<1 || n>16384) throw std::runtime_error("Display dimensions must be between 1 and 16384");
    return static_cast<int>(n);
}
inline Size resolution(const std::string& text) {
    auto x=text.find_first_of("xX");
    if(x==std::string::npos) throw std::runtime_error("--resolution requires WIDTHxHEIGHT");
    return {dimension(text.substr(0,x)),dimension(text.substr(x+1))};
}
inline Rect viewport(int width,int height) {
    if(width<=0 || height<=0) return {0,0,0,0};
    int w=width,h=height;
    if(int64_t(width)*300>int64_t(height)*520) w=std::max(1,int((int64_t(height)*520+150)/300));
    else h=std::max(1,int((int64_t(width)*300+260)/520));
    return {(width-w)/2,(height-h)/2,w,h};
}
inline Size windowSize(int width,int height) { auto r=viewport(width,height); return {r.width,r.height}; }
}
