#include "engine.hpp"
#include "assets.hpp"
#include "display.hpp"
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
    std::string assets,capture,captureDisplay;
    bool mute=false,smoke=false,fullscreen=false;
    double duration=0,offset=0;
    Display::Size requested={1040,600};
    for(int i=1;i<argc;++i) {
        std::string arg=argv[i];
        auto value=[&]() -> std::string { if(++i==argc) throw std::runtime_error("Missing value for "+arg); return argv[i]; };
        if(arg=="--assets") assets=value();
        else if(arg=="--resolution") requested=Display::resolution(value());
        else if(arg=="--scale") {
            auto text=value();
            if(text.size()!=1 || text[0]<'1' || text[0]>'4') throw std::runtime_error("--scale requires 1, 2, 3 or 4");
            int scale=text[0]-'0'; requested={520*scale,300*scale};
        }
        else if(arg=="--fullscreen") fullscreen=true;
        else if(arg=="--mute") mute=true;
        else if(arg=="--duration") duration=std::stod(value());
        else if(arg=="--offset") offset=std::stod(value());
        else if(arg=="--capture") capture=value();
        else if(arg=="--capture-display") captureDisplay=value();
        else if(arg=="--smoke-test") { smoke=true; duration=4; }
        else if(arg=="--help" || arg=="-h") {
            std::cout<<"Code Police / Mankind — C++11/SDL2\n"
                <<"Usage: code-police [--resolution WIDTHxHEIGHT] [--scale 1..4] [--fullscreen] [--mute] [--assets DIRECTORY]\n"
                <<"Window: fit 520:300 inside the requested dimensions, using nearest neighbour.\n"
                <<"Fullscreen: maximize the image on the current monitor with black bars; monitor resolution never changes.\n"
                <<"Escape or close window: quit. F11: fullscreen.\n"
                <<"Diagnostics: --duration SECONDS --offset SECONDS --capture FILE.bmp --capture-display FILE.bmp --smoke-test\n";
            return 0;
        } else throw std::runtime_error("Unknown option: "+arg);
    }
    if(!std::isfinite(duration) || !std::isfinite(offset) || duration<0 || offset<0 || offset>86400)
        throw std::runtime_error("Invalid scale, duration or offset");
    require(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|(mute ? 0 : SDL_INIT_AUDIO))==0,"SDL_Init");
    // Borderless desktop fullscreen, without asynchronous macOS Space changes.
    SDL_SetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES,"0");
    const bool initialFullscreen=fullscreen;
    if(assets.empty()) {
        char* base=SDL_GetBasePath(); require(base!=nullptr,"SDL_GetBasePath");
        assets=std::string(base)+"assets"; SDL_free(base);
    }
    FilePlatform platform(assets);
    // Desktop is destroyed first: callback closes before its platform storage.
    Desktop desktop;
    auto windowSize=Display::windowSize(requested.width,requested.height);
    desktop.window=SDL_CreateWindow("Code Police — Mankind",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        windowSize.width,windowSize.height,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_HIDDEN);
    require(desktop.window!=nullptr,"SDL_CreateWindow");
    int displayIndex=SDL_GetWindowDisplayIndex(desktop.window);
    SDL_DisplayMode initialMode{};
    require(SDL_GetCurrentDisplayMode(displayIndex,&initialMode)==0,"SDL_GetCurrentDisplayMode");
    int windowX=0,windowY=0;
    SDL_GetWindowPosition(desktop.window,&windowX,&windowY);
    auto toggleFullscreen=[&](bool enabled) {
        if(enabled) SDL_GetWindowPosition(desktop.window,&windowX,&windowY);
        require(SDL_SetWindowFullscreen(desktop.window,enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)==0,"fullscreen");
        fullscreen=enabled;
        if(!enabled) {
            SDL_SetWindowSize(desktop.window,windowSize.width,windowSize.height);
            SDL_SetWindowPosition(desktop.window,windowX,windowY);
        }
    };
    if(fullscreen) toggleFullscreen(true);
    SDL_ShowWindow(desktop.window);
    desktop.renderer=SDL_CreateRenderer(desktop.window,-1,SDL_RENDERER_ACCELERATED);
    if(!desktop.renderer) desktop.renderer=SDL_CreateRenderer(desktop.window,-1,SDL_RENDERER_SOFTWARE);
    require(desktop.renderer!=nullptr,"SDL_CreateRenderer");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
    desktop.texture=SDL_CreateTexture(desktop.renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,520,300);
    require(desktop.texture!=nullptr,"SDL_CreateTexture");
    require(SDL_SetTextureBlendMode(desktop.texture,SDL_BLENDMODE_NONE)==0,"SDL_SetTextureBlendMode");
    require(SDL_SetTextureScaleMode(desktop.texture,SDL_ScaleModeNearest)==0,"SDL_SetTextureScaleMode");
    auto draw=[&]() {
        SDL_SetRenderDrawColor(desktop.renderer,0,0,0,255);
        require(SDL_RenderClear(desktop.renderer)==0,"SDL_RenderClear");
        int width=0,height=0;
        require(SDL_GetRendererOutputSize(desktop.renderer,&width,&height)==0,"SDL_GetRendererOutputSize");
        auto area=fullscreen ? Display::viewport(width,height) : Display::Rect{0,0,width,height};
        SDL_Rect destination={area.x,area.y,area.width,area.height};
        require(SDL_RenderCopy(desktop.renderer,desktop.texture,nullptr,&destination)==0,"SDL_RenderCopy");
        return Display::Size{width,height};
    };
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
    std::cout<<"Window size "<<windowSize.width<<'x'<<windowSize.height<<"; fullscreen="<<fullscreen
        <<"; nearest neighbour; monitor resolution unchanged.\n";
    std::vector<uint32_t> pixels(520*300);
    Uint64 started=SDL_GetPerformanceCounter();
    bool running=true,minimized=false,restored=false,firstToggle=false,secondToggle=false;
    int toggles=0;
    Display::Size sizeBeforeToggle=windowSize;
    Uint64 lastToggle=started;
    size_t frames=0,loops=0;
    bool durationReached=false;
    std::string quitReason="script";
    while(running && !demo.boolend) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type==SDL_QUIT) { running=false; quitReason="window close"; }
            if(event.type==SDL_KEYDOWN) {
                if(event.key.keysym.sym==SDLK_ESCAPE) { running=false; quitReason=durationReached ? "requested duration" : "Escape"; }
                if(event.key.keysym.sym==SDLK_F11 && !event.key.repeat) {
                    if(toggles==0) sizeBeforeToggle=windowSize;
                    toggleFullscreen(!fullscreen); ++toggles; lastToggle=SDL_GetPerformanceCounter();
                }
            }
        }
        if(!running) break;
        if(!fullscreen && !(SDL_GetWindowFlags(desktop.window)&SDL_WINDOW_MINIMIZED)) {
            int width=0,height=0; SDL_GetWindowSize(desktop.window,&width,&height);
            if(width!=windowSize.width || height!=windowSize.height) {
                windowSize=Display::windowSize(width,height);
                if(width!=windowSize.width || height!=windowSize.height)
                    SDL_SetWindowSize(desktop.window,windowSize.width,windowSize.height);
            }
        }
        int64_t timerStart=demo.kTime->StartTime;
        auto frame=demo.frame();
        if(timerStart!=demo.kTime->StartTime) { ++loops; std::cout<<"Timeline loop "<<loops<<'\n'; }
        for(size_t i=0;i<pixels.size();++i) pixels[i]=(*frame.storage)[i].bits|0xff000000u;
        require(SDL_UpdateTexture(desktop.texture,nullptr,pixels.data(),520*int(sizeof(uint32_t)))==0,"SDL_UpdateTexture");
        draw();
        SDL_RenderPresent(desktop.renderer); ++frames;
        double elapsed=double(SDL_GetPerformanceCounter()-started)/double(SDL_GetPerformanceFrequency());
        if(smoke && elapsed>1 && !minimized) { SDL_MinimizeWindow(desktop.window); minimized=true; }
        if(smoke && elapsed>1.5 && !restored) {
            SDL_RestoreWindow(desktop.window);
            if(!fullscreen) SDL_SetWindowSize(desktop.window,1040,600);
            SDL_WarpMouseInWindow(desktop.window,260,150); restored=true;
        }
        if(smoke && ((elapsed>2.2 && !firstToggle) || (elapsed>2.8 && !secondToggle))) {
            SDL_Event toggle{}; toggle.type=SDL_KEYDOWN; toggle.key.keysym.sym=SDLK_F11; SDL_PushEvent(&toggle);
            if(!firstToggle) firstToggle=true; else secondToggle=true;
        }
        bool smokeFinished=!smoke || (toggles>=2 && double(SDL_GetPerformanceCounter()-lastToggle)/SDL_GetPerformanceFrequency()>=0.5);
        if(duration>0 && elapsed>=duration && smokeFinished) {
            durationReached=true;
            SDL_Event quit{}; quit.type=SDL_KEYDOWN; quit.key.keysym.sym=SDLK_ESCAPE; SDL_PushEvent(&quit);
        }
        // Original: one elapsed-time frame, THEN sleep 20 ms. No fixed timestep,
        // catch-up updates or vsync imposed on the per-update particle movement.
        SDL_Delay(20);
    }
    if(smoke) {
        require(toggles==2 && fullscreen==initialFullscreen,"fullscreen round trip");
        if(!fullscreen) {
            int w=0,h=0; SDL_GetWindowSize(desktop.window,&w,&h);
            require(w==sizeBeforeToggle.width && h==sizeBeforeToggle.height,"window size not restored after F11");
        }
        SDL_DisplayMode after{}; require(SDL_GetCurrentDisplayMode(displayIndex,&after)==0,"SDL_GetCurrentDisplayMode");
        require(after.w==initialMode.w && after.h==initialMode.h && after.refresh_rate==initialMode.refresh_rate
            && after.format==initialMode.format,"monitor mode changed");
        SDL_ScaleMode filter=SDL_ScaleModeLinear;
        require(SDL_GetTextureScaleMode(desktop.texture,&filter)==0 && filter==SDL_ScaleModeNearest,"nearest-neighbour filter");
    }
    if(fullscreen) {
        require((SDL_GetWindowFlags(desktop.window)&SDL_WINDOW_FULLSCREEN_DESKTOP)==SDL_WINDOW_FULLSCREEN_DESKTOP,
            "desktop fullscreen flag");
        // The drawable can exclude the macOS notch/menu safe area. Scale to
        // SDL's actual output size, never stretch to assumed display bounds.
    }
    if(!captureDisplay.empty()) {
        auto size=draw();
        SDL_Surface* surface=SDL_CreateRGBSurfaceWithFormat(0,size.width,size.height,32,SDL_PIXELFORMAT_ARGB8888);
        require(surface!=nullptr,"display capture surface");
        int read=SDL_RenderReadPixels(desktop.renderer,nullptr,surface->format->format,surface->pixels,surface->pitch);
        int saved=read==0 ? SDL_SaveBMP(surface,captureDisplay.c_str()) : -1;
        SDL_FreeSurface(surface);
        require(read==0 && saved==0,"display capture");
        auto area=fullscreen ? Display::viewport(size.width,size.height) : Display::Rect{0,0,size.width,size.height};
        std::cout<<"Display capture "<<size.width<<'x'<<size.height<<"; viewport="<<area.x<<','<<area.y<<','<<area.width<<','<<area.height<<'\n';
    }
    if(!capture.empty()) {
        SDL_Surface* surface=SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(),520,300,32,520*4,SDL_PIXELFORMAT_ARGB8888);
        require(surface!=nullptr,"capture surface"); int result=SDL_SaveBMP(surface,capture.c_str()); SDL_FreeSurface(surface);
        require(result==0,"SDL_SaveBMP");
    }
    demo.stop();
    double elapsed=double(SDL_GetPerformanceCounter()-started)/double(SDL_GetPerformanceFrequency());
    std::cout<<"Stopped cleanly after "<<frames<<" frames in "<<elapsed<<" seconds; "<<loops<<" timeline loops; "<<quitReason
        <<(smoke ? "; minimize, restore, resize, pointer, F11 twice and Escape exercised; monitor mode unchanged; nearest neighbour verified" : "")<<".\n";
    return 0;
} catch(const std::exception& e) {
    std::cerr<<"Code Police: "<<e.what()<<'\n'; return 1;
}
