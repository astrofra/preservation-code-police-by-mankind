// Browser platform adapter. The translated artwork and resource decoder are shared
// unchanged with native-sdl2. No simulation step is invented for presentation.
#include "engine.hpp"
#include "assets.hpp"
#include <SDL.h>
#include <emscripten.h>
#include <iomanip>
#include <memory>
#include <sstream>

namespace {
EM_JS(void, reportError, (const char* message), {
    if (Module.onDemoError) Module.onDemoError(UTF8ToString(message));
});
EM_JS(void, presented, (), {
    if (Module.onDemoFrame) Module.onDemoFrame();
});
EM_JS(void, resumeAudio, (), {
    var SDL2 = Module['SDL2'];
    if (SDL2.audioContext) SDL2.audioContext.resume().catch(function(error) {
        if (Module.onDemoError) Module.onDemoError('Audio: ' + error.message);
    });
});
// 0 no context, 1 running, 2 suspended, 3 closed. iOS starts suspended and only
// resumes inside a user gesture, so this is reported even when audio is muted.
EM_JS(int, audioState, (), {
    var context = Module['SDL2'] && Module['SDL2'].audioContext;
    if (!context) return 0;
    if (context.state === 'running') return 1;
    if (context.state === 'suspended') return 2;
    if (context.state === 'closed') return 3;
    return 4;
});
void require(bool ok, const char* operation) {
    if (!ok) throw std::runtime_error(std::string(operation) + ": " + SDL_GetError());
}
struct BrowserPlatform : FilePlatform {
    BrowserPlatform() : FilePlatform("/assets") {}
    Ref<DemoAudio> audio(const String& relative) override {
        try { return FilePlatform::audio(relative); }
        catch (const std::exception& error) {
            throw std::runtime_error(std::string(relative) + ": " + error.what());
        }
    }
};
struct Application {
    BrowserPlatform platform;
    std::unique_ptr<kraycasting> demo;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_AudioDeviceID device = 0;
    std::vector<uint32_t> pixels = std::vector<uint32_t>(520 * 300, 0xff000000);
    bool running = false, suspended = false, manual = false, muted = false;
    int frames = 0, tick = 0, starts = 0;
    int64_t hiddenAt = 0;
    std::string text;

    Application() {
        SDL_SetMainReady();
        SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        require(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) == 0, "SDL_Init");
        window = SDL_CreateWindow("Code Police — Mankind", 0, 0, 520, 300, 0);
        require(window, "SDL_CreateWindow");
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        require(renderer, "SDL_CreateRenderer software");
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 520, 300);
        require(texture, "SDL_CreateTexture");
        require(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE) == 0, "SDL_SetTextureBlendMode");
        require(SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest) == 0, "SDL_SetTextureScaleMode");
        // Decode before enabling Start so errors cannot silently omit a resource.
        reset(888);
    }
    void reset(uint64_t seed) {
        if (device) SDL_PauseAudioDevice(device, 1);
        demo.reset(); // Stops old voices before releasing their platform storage.
        platform.sounds.clear();
        platform.events.clear();
        platform.clockOffset = 0;
        demo.reset(new kraycasting(platform, seed));
        demo->init();
        frames = tick = 0;
        suspended = false;
    }
    void openAudio() {
        if (device) return;
        SDL_AudioSpec wanted{};
        wanted.freq = 8000; wanted.format = AUDIO_S16SYS; wanted.channels = 1; wanted.samples = 1024;
        wanted.userdata = &platform;
        wanted.callback = [](void* p, Uint8* bytes, int count) {
            static_cast<FilePlatform*>(p)->mix(reinterpret_cast<int16_t*>(bytes), size_t(count) / 2);
        };
        device = SDL_OpenAudioDevice(nullptr, 0, &wanted, nullptr, 0);
        require(device != 0, "SDL_OpenAudioDevice");
    }
    void present(const Array<jint>& frame) {
        for (size_t i = 0; i < pixels.size(); ++i) pixels[i] = (*frame.storage)[i].bits | 0xff000000u;
        require(SDL_UpdateTexture(texture, nullptr, pixels.data(), 520 * sizeof(uint32_t)) == 0, "SDL_UpdateTexture");
        require(SDL_RenderCopy(renderer, texture, nullptr, nullptr) == 0, "SDL_RenderCopy");
        SDL_RenderPresent(renderer);
        ++frames;
        presented();
    }
    void frame(int at) { tick = at; present(demo->frameAt(jint(tick))); }
};
std::unique_ptr<Application> app;
void fail(const std::exception& error) {
    if (app) { app->running = false; if (app->device) SDL_PauseAudioDevice(app->device, 1); }
    reportError(error.what());
}
void callback(void*) {
    try {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {} // Passive demo; controls are ordinary HTML.
        if (app && app->running && !app->suspended && !app->manual)
            app->frame(int32_t(app->demo->kTime->getTime()));
    } catch (const std::exception& error) { fail(error); }
    // Original run(): render, then sleep(20). Schedule AFTER the completed frame;
    // using a 50 Hz interval or a requestAnimationFrame gate changes that cadence.
    emscripten_async_call(callback, nullptr, 20);
}
}

extern "C" {
EMSCRIPTEN_KEEPALIVE int cp_start(int mute) {
    try {
        app->manual = false; app->muted = mute != 0;
        app->reset(uint64_t(app->platform.currentTimeMillis()));
        if (!app->muted) { app->openAudio(); resumeAudio(); SDL_PauseAudioDevice(app->device, 0); }
        app->running = true; ++app->starts;
        app->frame(0);
        return 1;
    } catch (const std::exception& error) { fail(error); return 0; }
}
EMSCRIPTEN_KEEPALIVE void cp_stop() {
    app->running = false;
    if (app->device) SDL_PauseAudioDevice(app->device, 1);
    app->demo->stop();
}
EMSCRIPTEN_KEEPALIVE void cp_suspend(int hidden) {
    if (!app->running || app->manual || app->suspended == bool(hidden)) return;
    if (hidden) {
        app->hiddenAt = app->platform.currentTimeMillis();
        if (app->device) SDL_PauseAudioDevice(app->device, 1);
    } else {
        app->platform.clockOffset -= app->platform.currentTimeMillis() - app->hiddenAt;
        if (app->device && !app->muted) { resumeAudio(); SDL_PauseAudioDevice(app->device, 0); }
    }
    app->suspended = hidden != 0;
}
EMSCRIPTEN_KEEPALIVE const char* cp_status() {
    std::ostringstream out;
    out << "{\"running\":" << app->running << ",\"suspended\":" << app->suspended
        << ",\"frames\":" << app->frames << ",\"tick\":" << app->tick
        << ",\"starts\":" << app->starts << ",\"audio\":" << (app->device != 0)
        << ",\"audioState\":" << audioState() << '}';
    app->text = out.str(); return app->text.c_str();
}
// Deterministic diagnostics are explicitly invoked by the verifier, never by the UI.
EMSCRIPTEN_KEEPALIVE void cp_test_reset() {
    cp_stop(); app->manual = true; app->platform.recordEvents = true; app->reset(888);
}
EMSCRIPTEN_KEEPALIVE const char* cp_test_frame(int tick) {
    app->demo->random.seed(100000 + tick);
    app->platform.events.clear();
    app->frame(tick);
    uint64_t h = 0xcbf29ce484222325ULL;
    for (uint32_t pixel : app->pixels) { h ^= pixel; h *= 0x100000001b3ULL; }
    std::ostringstream out; out << std::hex << std::setw(16) << std::setfill('0') << h;
    app->text = out.str(); return app->text.c_str();
}
EMSCRIPTEN_KEEPALIVE const char* cp_test_state() {
    std::ostringstream out; out << std::setprecision(17) << "{\"values\":[";
    bool first = true;
    auto number = [&](double d) { if (!first) out << ','; first = false; out << d; };
    for (auto& item : *app->demo->CamerHash.entries) {
        auto c = std::static_pointer_cast<kcamera>(item.second);
        for (double d : {c->FOV, c->O1, c->O2, c->O3, c->X, c->Y, c->Z}) number(d);
    }
    for (auto& item : *app->demo->partcHash.entries) {
        auto p = std::static_pointer_cast<kparticle>(item.second);
        for (auto a : {p->Ax, p->Ay, p->Az, p->Vx, p->Vy, p->Vz}) for (double d : *a.storage) number(d);
        for (double d : {p->aveX, p->aveY, p->aveZ}) number(d);
        for (double d : *p->force.storage) number(d);
        number(int32_t(p->nbparticle));
        for (auto a : {p->posx, p->posy, p->posz}) for (double d : *a.storage) number(d);
    }
    out << "],\"events\":["; first = true;
    for (auto& e : app->platform.events) {
        if (!first) out << ','; first = false;
        out << "[\"" << e.first << "\",\"" << e.second << "\"]";
    }
    out << "]}"; app->text = out.str(); return app->text.c_str();
}
EMSCRIPTEN_KEEPALIVE const char* cp_test_events() {
    std::ostringstream out; out << '['; bool first = true;
    for (auto& e : app->platform.events) {
        if (!first) out << ','; first = false;
        out << "[\"" << e.first << "\",\"" << e.second << "\"]";
    }
    out << ']'; app->text = out.str(); return app->text.c_str();
}
EMSCRIPTEN_KEEPALIVE const int16_t* cp_test_pcm(int index) { return app->platform.sounds.at(index)->pcm.data(); }
EMSCRIPTEN_KEEPALIVE int cp_test_pcm_size(int index) { return int(app->platform.sounds.at(index)->pcm.size()); }
EMSCRIPTEN_KEEPALIVE const char* cp_test_pcm_name(int index) { return app->platform.sounds.at(index)->name.c_str(); }
}
int main() {
    try { app.reset(new Application()); emscripten_async_call(callback, nullptr, 20); return 0; }
    catch (const std::exception& error) { reportError(error.what()); return 1; }
}
