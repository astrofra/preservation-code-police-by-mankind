#include "assets.hpp"
#include <fstream>
#include <chrono>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include "stb_image.h"

std::vector<uint8_t> readBytes(const std::string& path) {
    std::ifstream stream(path,std::ios::binary);
    if(!stream) throw std::runtime_error("Cannot open "+path);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(stream),{});
}
int64_t FilePlatform::currentTimeMillis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()+clockOffset;
}
String FilePlatform::script() {
    auto bytes=readBytes(directory+"/scene.txt"); return String(bytes.begin(),bytes.end());
}
Ref<Image> FilePlatform::image(const String& relative) {
    std::string path=directory+"/"+static_cast<const std::string&>(relative)+".png";
    int w=0,h=0,c=0;
    stbi_uc* rgba=stbi_load(path.c_str(),&w,&h,&c,4);
    if(!rgba) throw std::runtime_error("Cannot decode "+path+": "+stbi_failure_reason());
    auto image=std::make_shared<Image>(); image->width=w; image->height=h;
    image->pixels=Array<jint>(jint(w*h));
    for(int i=0;i<w*h;++i) image->pixels[jint(i)]=jint((uint32_t(rgba[i*4+3])<<24)|
        (uint32_t(rgba[i*4])<<16)|(uint32_t(rgba[i*4+1])<<8)|uint32_t(rgba[i*4+2]));
    stbi_image_free(rgba); return image;
}
std::vector<int16_t> decodeAU(const std::vector<uint8_t>& bytes) {
    auto be=[&](size_t offset) { if(offset+4>bytes.size()) throw std::runtime_error("Truncated AU header");
        return (uint32_t(bytes[offset])<<24)|(uint32_t(bytes[offset+1])<<16)|(uint32_t(bytes[offset+2])<<8)|bytes[offset+3]; };
    if(be(0)!=0x2e736e64 || be(12)!=1 || be(16)!=8000 || be(20)!=1)
        throw std::runtime_error("Expected original 8 kHz mono mu-law AU");
    size_t offset=be(4), size=be(8);
    if(offset<24 || offset>bytes.size()) throw std::runtime_error("Invalid AU offset");
    if(size==0xffffffffu) size=bytes.size()-offset;
    if(size>bytes.size()-offset) throw std::runtime_error("Truncated AU samples");
    std::vector<int16_t> samples(size);
    for(size_t i=0;i<size;++i) {
        unsigned u=(~bytes[offset+i])&255u;
        int value=int(((u&15u)<<3)+132u)<<((u>>4)&7u);
        samples[i]=static_cast<int16_t>((u&128u) ? 132-value : value-132);
    }
    return samples;
}
Ref<DemoAudio> FilePlatform::audio(const String& relative) {
    String path=relative;
    auto sound=std::make_shared<Sound>(audioMutex);
    sound->name=relative.substr(relative.find_last_of('/')+1);
    sound->pcm=decodeAU(readBytes(directory+"/"+static_cast<const std::string&>(path)));
    sound->event=[this](const String& name,const String& op) { if(recordEvents) events.emplace_back(name,op); };
    sounds.push_back(sound); return sound;
}
void Sound::command(const String& op) {
    std::lock_guard<std::mutex> lock(mutex);
    event(name,op);
    playing=op!="stop"; looping=op=="loop"; position=0;
}
void FilePlatform::mix(int16_t* destination,size_t count) {
    std::lock_guard<std::mutex> lock(audioMutex);
    for(size_t i=0;i<count;++i) {
        int mixed=0;
        for(auto& sound:sounds) if(sound->playing && !sound->pcm.empty()) {
            mixed+=sound->pcm[sound->position++];
            if(sound->position==sound->pcm.size()) { sound->position=0; sound->playing=sound->looping; }
        }
        destination[i]=static_cast<int16_t>(std::max(-32768,std::min(32767,mixed)));
    }
}
