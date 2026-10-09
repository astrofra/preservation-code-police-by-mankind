#pragma once
#include "java_semantics.hpp"
#include <functional>

struct Image : Object {
    jint width, height;
    Array<jint> pixels;
};
struct DemoAudio : Object {
    virtual void play() = 0;
    virtual void loop() = 0;
    virtual void stop() = 0;
};
// The renderer has no SDL, OS, filesystem, event-loop or thread dependency.
struct Platform {
    virtual ~Platform() = default;
    virtual int64_t currentTimeMillis() = 0;
    virtual Ref<Image> image(const String& relative) = 0;
    virtual Ref<DemoAudio> audio(const String& relative) = 0;
    virtual String script() = 0;
};
template<class T> T* raw(T* p) { return p; }
template<class T> T* raw(const Ref<T>& p) { return p.get(); }
struct kraycasting;
struct EngineObject : Object {
    kraycasting* owner;
    explicit EngineObject(kraycasting* p) : owner(p) {}
};
