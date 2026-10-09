#pragma once
#include "platform.hpp"
#include <mutex>

struct Sound : DemoAudio {
    String name;
    std::vector<int16_t> pcm;
    size_t position = 0;
    bool playing = false, looping = false;
    std::mutex& mutex;
    std::function<void(const String&,const String&)> event;
    Sound(std::mutex& m) : mutex(m) {}
    void command(const String& op);
    void play() override { command("play"); }
    void loop() override { command("loop"); }
    void stop() override { command("stop"); }
};

struct FilePlatform : Platform {
    std::string directory;
    std::mutex audioMutex;
    std::vector<Ref<Sound>> sounds;
    std::vector<std::pair<String,String>> events;
    bool recordEvents = false;
    int64_t clockOffset = 0;
    explicit FilePlatform(std::string path) : directory(std::move(path)) {}
    int64_t currentTimeMillis() override;
    Ref<Image> image(const String& relative) override;
    Ref<DemoAudio> audio(const String& relative) override;
    String script() override;
    void mix(int16_t* destination,size_t count);
};
std::vector<uint8_t> readBytes(const std::string& path);
std::vector<int16_t> decodeAU(const std::vector<uint8_t>& bytes);
