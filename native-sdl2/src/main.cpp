#include "engine.hpp"
#include "assets.hpp"
#include <SDL.h>
#include <iostream>
#include <random>
#include <cstdlib>

namespace {
void require(bool ok,const char* operation) {
    if(!ok) throw std::runtime_error(std::string(operation)+": "+SDL_GetError());
}
void audioCallback(void* userdata,Uint8* bytes,int length) {
    static_cast<FilePlatform*>(userdata)->mix(reinterpret_cast<int16_t*>(bytes),size_t(length)/2);
}
struct Desktop {
    SDL_Window* window=nullptr;
    SDL_Renderer* renderer=nullptr;
    SDL_Texture* texture=nullptr;
    SDL_AudioDeviceID device=0;
    ~Desktop() {
        if(device) SDL_CloseAudioDevice(device);
        if(texture) SDL_DestroyTexture(texture);
        if(renderer) SDL_DestroyRenderer(renderer);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
};
}
int main(int argc,char** argv) try {
    std::string assets,capture;
    bool mute=false,smoke=false;
    double duration=0,offset=0;
    int scale=2;
    for(int i=1;i<argc;++i) {
        std::string arg=argv[i];
        auto value=[&]() -> std::string { if(++i==argc) throw std::runtime_error("Missing value for "+arg); return argv[i]; };
        if(arg=="--assets") assets=value();
        else if(arg=="--scale") scale=std::stoi(value());
        else if(arg=="--mute") mute=true;
        else if(arg=="--duration") duration=std::stod(value());
        else if(arg=="--offset") offset=std::stod(value());
        else if(arg=="--capture") capture=value();
        else if(arg=="--smoke-test") { smoke=true; duration=4; }
        else if(arg=="--help") {
            std::cout<<"Code Police / Mankind — C++11/SDL2\n"
                <<"Usage: code-police [--scale 1..4] [--mute] [--assets DIRECTORY]\n"
                <<"Escape or close window: quit. F11: fullscreen.\n"
                <<"Diagnostics: --duration SECONDS --offset SECONDS --capture FILE.bmp --smoke-test\n";
            return 0;
        } else throw std::runtime_error("Unknown option: "+arg);
    }
    if(scale<1 || scale>4 || !std::isfinite(duration) || !std::isfinite(offset) || duration<0 || offset<0 || offset>86400)
        throw std::runtime_error("Invalid scale, duration or offset");
    require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|(mute ? 0 : SDL_INIT_AUDIO))==0,"SDL_Init");
    if(assets.empty()) {
        char* base=SDL_GetBasePath(); require(base!=nullptr,"SDL_GetBasePath");
        assets=std::string(base)+"assets"; SDL_free(base);
    }
    FilePlatform platform(assets);
    // Desktop is destroyed first: callback closes before its platform storage.
    Desktop desktop;
    desktop.window=SDL_CreateWindow("Code Police — Mankind",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        520*scale,300*scale,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
    require(desktop.window!=nullptr,"SDL_CreateWindow");
    SDL_SetWindowMinimumSize(desktop.window,520,300);
    desktop.renderer=SDL_CreateRenderer(desktop.window,-1,SDL_RENDERER_ACCELERATED);
    if(!desktop.renderer) desktop.renderer=SDL_CreateRenderer(desktop.window,-1,SDL_RENDERER_SOFTWARE);
    require(desktop.renderer!=nullptr,"SDL_CreateRenderer");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
    require(SDL_RenderSetLogicalSize(desktop.renderer,520,300)==0,"SDL_RenderSetLogicalSize");
    desktop.texture=SDL_CreateTexture(desktop.renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,520,300);
    require(desktop.texture!=nullptr,"SDL_CreateTexture");
    require(SDL_SetTextureBlendMode(desktop.texture,SDL_BLENDMODE_NONE)==0,"SDL_SetTextureBlendMode");
    std::random_device entropy;
    kraycasting demo(platform,(uint64_t(entropy())<<32)^entropy());
    demo.init(); demo.kTime->StartTime-=int64_t(offset*1000);
    if(!mute) {
        SDL_AudioSpec desired{}; desired.freq=8000; desired.format=AUDIO_S16SYS; desired.channels=1;
        desired.samples=256; desired.callback=audioCallback; desired.userdata=&platform;
        desktop.device=SDL_OpenAudioDevice(nullptr,0,&desired,nullptr,0);
        require(desktop.device!=0,"SDL_OpenAudioDevice (use --mute if audio is unavailable)");
        SDL_PauseAudioDevice(desktop.device,0);
    }
    std::cout<<std::unitbuf<<"520x300 framebuffer; "<<platform.sounds.size()<<" external sounds; "
        <<(mute ? "muted" : "8 kHz mono audio active")<<"; cycle "<<int32_t(demo.THEkScript->EndDate)/100.0<<" seconds. Escape to quit.\n";
    std::vector<uint32_t> pixels(520*300);
    Uint64 started=SDL_GetPerformanceCounter();
    bool running=true,minimized=false,restored=false,fullscreen=false;
    size_t frames=0,loops=0;
    bool durationReached=false;
    std::string quitReason="script";
    while(running && !demo.boolend) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type==SDL_QUIT) { running=false; quitReason="window close"; }
            if(event.type==SDL_KEYDOWN) {
                if(event.key.keysym.sym==SDLK_ESCAPE) { running=false; quitReason=durationReached ? "requested duration" : "Escape"; }
                if(event.key.keysym.sym==SDLK_F11) {
                    fullscreen=!fullscreen;
                    require(SDL_SetWindowFullscreen(desktop.window,fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)==0,"fullscreen");
                }
            }
        }
        if(!running) break;
        int64_t timerStart=demo.kTime->StartTime;
        auto frame=demo.frame();
        if(timerStart!=demo.kTime->StartTime) { ++loops; std::cout<<"Timeline loop "<<loops<<'\n'; }
        for(size_t i=0;i<pixels.size();++i) pixels[i]=(*frame.storage)[i].bits|0xff000000u;
        require(SDL_UpdateTexture(desktop.texture,nullptr,pixels.data(),520*int(sizeof(uint32_t)))==0,"SDL_UpdateTexture");
        SDL_SetRenderDrawColor(desktop.renderer,0,0,0,255);
        require(SDL_RenderClear(desktop.renderer)==0,"SDL_RenderClear");
        require(SDL_RenderCopy(desktop.renderer,desktop.texture,nullptr,nullptr)==0,"SDL_RenderCopy");
        SDL_RenderPresent(desktop.renderer); ++frames;
        double elapsed=double(SDL_GetPerformanceCounter()-started)/double(SDL_GetPerformanceFrequency());
        if(smoke && elapsed>1 && !minimized) { SDL_MinimizeWindow(desktop.window); minimized=true; }
        if(smoke && elapsed>1.5 && !restored) {
            SDL_RestoreWindow(desktop.window); SDL_SetWindowSize(desktop.window,1040,600);
            SDL_WarpMouseInWindow(desktop.window,260,150); restored=true;
        }
        if(duration>0 && elapsed>=duration) {
            durationReached=true;
            SDL_Event quit{}; quit.type=SDL_KEYDOWN; quit.key.keysym.sym=SDLK_ESCAPE; SDL_PushEvent(&quit);
        }
        // Original: one elapsed-time frame, THEN sleep 20 ms. No fixed timestep,
        // catch-up updates or vsync imposed on the per-update particle movement.
        SDL_Delay(20);
    }
    if(!capture.empty()) {
        SDL_Surface* surface=SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(),520,300,32,520*4,SDL_PIXELFORMAT_ARGB8888);
        require(surface!=nullptr,"capture surface"); int result=SDL_SaveBMP(surface,capture.c_str()); SDL_FreeSurface(surface);
        require(result==0,"SDL_SaveBMP");
    }
    demo.stop();
    double elapsed=double(SDL_GetPerformanceCounter()-started)/double(SDL_GetPerformanceFrequency());
    std::cout<<"Stopped cleanly after "<<frames<<" frames in "<<elapsed<<" seconds; "<<loops<<" timeline loops; "<<quitReason
        <<(smoke ? "; minimize, restore, resize, pointer and Escape exercised" : "")<<".\n";
    return 0;
} catch(const std::exception& e) {
    std::cerr<<"Code Police: "<<e.what()<<'\n'; return 1;
}
