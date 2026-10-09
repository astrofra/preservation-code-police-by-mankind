// Translated from the verified CFR reconstruction. See tools/translate.py.
#pragma once
#include "platform.hpp"
struct kraycasting;
struct kTexture;
struct kRenderContext;
struct kcamera;
struct kparticle;
struct kTable;
struct kTimer;
struct kPixel;
struct helpfade;
struct kParameterX;
struct kParameterXCTE;
struct kParameterXAFF;
struct kParameterXRND;
struct kParameterXSPLINE;
struct kParameterXSIN;
struct kParameterXCOS;
struct kParameterXOB;
struct kEffectX;
struct kSoundCommand;
struct kPartX;
struct kScript;
struct raycasting;
struct kSound;

struct kraycasting : Object {
    Platform& platform;
    JavaRandom random;
    kSound* firstkSound = nullptr;
    bool stopped = false;
    explicit kraycasting(Platform& p, uint64_t seed) : platform(p) { random.seed(seed); }
    ~kraycasting();
    Array<jint> frame();
    Array<jint> frameAt(jint tick);
    void stop();
    String getParameter(String) { return platform.script(); }
    String getDocumentBase() { return {}; }
    Ref<Image> getImage(String, String path) { return platform.image(path); }
    Ref<DemoAudio> getAudioClip(String, String path) { return platform.audio(path); }
    struct Size { jint width = 520, height = 300; Size* operator->() { return this; } };
    Size getSize() { return {}; }
    Ref<kScript> THEkScript = {};
    Ref<kRenderContext> RenderContext = {};
    Ref<raycasting> rcs = {};
    Array<jint> applet_image = {};
    bool boolend = false;
    Ref<kTimer> kTime = {};
    Array<int8_t> ct = {};
    Hashtable partcHash = {};
    Hashtable ImageHash = {};
    Hashtable RectaHash = {};
    Hashtable SoundHash = {};
    Hashtable TableHash = {};
    Hashtable CamerHash = {};
    jint FindHowMany(String string, String string2);
    String FindInto(String string, jint n, String string2);
    Array<String> SplitSTag(String string, String string2);
    void init();
};

struct kTexture : EngineObject {
    jint Modulo = {};
    jint Height = {};
    Array<jint> pixels = {};
    kTexture(kraycasting* owner, Ref<Image> image);
};

struct kRenderContext : EngineObject {
    jint ClipX1 = {};
    jint ClipY1 = {};
    jint ClipX2 = {};
    jint ClipY2 = {};
    jint Modulo = {};
    jint Height = {};
    Array<jint> Chunk2 = {};
    Array<jint> CurrentChunk = {};
    kraycasting* app = {};
    kRenderContext* RC_RootContext = {};
    kRenderContext* RC_FatherContext = {};
    jint MaxX = {};
    jint MaxY = {};
    jint MaxPreX = {};
    jint MaxPreY = {};
    Array<Array<Ref<kPixel>>> Pretable = {};
    jint CurPreX = {};
    jint CurPreY = {};
    jint CurX = {};
    jint CurY = {};
    double PosX = {};
    double PosY = {};
    double PosZ = {};
    double floor = {};
    double ceiling = {};
    bool drawable = {};
    kRenderContext(kraycasting* owner, kraycasting* applet);
    kRenderContext(kraycasting* owner, Ref<kRenderContext> kRenderContext2);
    kRenderContext(kraycasting* owner, Ref<kRenderContext> kRenderContext2, double d, double d2, double d3, double d4);
    void SetRect(Ref<kRenderContext> kRenderContext2, double d, double d2, double d3, double d4);
    void SetSub(Ref<kRenderContext> kRenderContext2, jint n, jint n2, jint n3, jint n4);
    Array<jint> createImage();
};

struct kcamera : EngineObject {
    double X = {};
    double Y = {};
    double Z = {};
    double O1 = {};
    double O2 = {};
    double O3 = {};
    double FOV = {};
    kcamera(kraycasting* owner, double d, double d2, double d3, double d4, double d5, double d6, double d7);
    void set(double d, double d2, double d3, double d4, double d5, double d6, double d7);
    void target(double d, double d2, double d3, double d4, double d5, double d6, double d7, double d8);
};

struct kparticle : EngineObject {
    Array<double> posx = {};
    Array<double> posy = {};
    Array<double> posz = {};
    Array<double> Vx = {};
    Array<double> Vy = {};
    Array<double> Vz = {};
    Array<double> Ax = {};
    Array<double> Ay = {};
    Array<double> Az = {};
    double aveX = {};
    double aveY = {};
    double aveZ = {};
    Array<double> force = {};
    jint nbparticle = {};
    kparticle(kraycasting* owner, jint n, double d, double d2, double d3, double d4, double d5, double d6, double d7, double d8);
    void drawflare(Ref<kcamera> kcamera2, Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, double d, double d2);
    void drawsprit(Ref<kcamera> kcamera2, Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, double d, double d2);
    void move();
    void mvtfloor(double d, double d2, double d3, double d4);
    void mvtgalax();
    void reset(double d, double d2, double d3, double d4, double d5, double d6, double d7, double d8);
};

struct kTable : EngineObject {
    Array<double> datetable = {};
    Array<double> floattable = {};
    jint lengthtable = {};
    kTable(kraycasting* owner, String string);
    double spline(double d);
};

struct kTimer : EngineObject {
    int64_t StartTime = owner->platform.currentTimeMillis();
    explicit kTimer(kraycasting* p) : EngineObject(p) {}
    jint getTime();
    void reset();
};

struct kPixel : EngineObject {
    jint u = jint(0);
    jint v = jint(0);
    jint r = jint(0);
    jint g = jint(0);
    jint b = jint(0);
    double XI = {};
    double YI = {};
    double ZI = {};
    double XF = {};
    double YF = {};
    double ZF = {};
    explicit kPixel(kraycasting* p) : EngineObject(p) {}
};

struct helpfade : EngineObject {
    double Frd1 = {};
    double Frv1 = {};
    double Frd2 = {};
    double Frv2 = {};
    double Fgd1 = {};
    double Fgv1 = {};
    double Fgd2 = {};
    double Fgv2 = {};
    double Fbd1 = {};
    double Fbv1 = {};
    double Fbd2 = {};
    double Fbv2 = {};
    explicit helpfade(kraycasting* p) : EngineObject(p) {}
};

struct kParameterX : EngineObject {
    jint CodeParam = {};
    double a = {};
    double b = {};
    double c = {};
    jint dtlength = {};
    Ref<kTable> spltable = {};
    Ref<Object> kOBJ = {};
    kParameterX(kraycasting* owner);
    virtual Ref<Object> Out() = 0;
    virtual double doubleout(double var1) = 0;
};

struct kParameterXCTE : kParameterX {
    kParameterXCTE(kraycasting* owner);
    virtual Ref<Object> Out();
    virtual double doubleout(double d);
};

struct kParameterXAFF : kParameterX {
    kParameterXAFF(kraycasting* owner);
    virtual Ref<Object> Out();
    virtual double doubleout(double d);
};

struct kParameterXRND : kParameterX {
    kParameterXRND(kraycasting* owner);
    virtual Ref<Object> Out();
    virtual double doubleout(double d);
};

struct kParameterXSPLINE : kParameterX {
    kParameterXSPLINE(kraycasting* owner);
    virtual Ref<Object> Out();
    virtual double doubleout(double d);
};

struct kParameterXSIN : kParameterX {
    kParameterXSIN(kraycasting* owner);
    virtual Ref<Object> Out();
    virtual double doubleout(double d);
};

struct kParameterXCOS : kParameterX {
    kParameterXCOS(kraycasting* owner);
    virtual Ref<Object> Out();
    virtual double doubleout(double d);
};

struct kParameterXOB : kParameterX {
    kParameterXOB(kraycasting* owner);
    virtual Ref<Object> Out();
    virtual double doubleout(double d);
};

struct kEffectX : EngineObject {
    jint keffectcode = {};
    jint knbparam = {};
    Array<Ref<kParameterX>> kParameterXEffectX = {};
    Ref<helpfade> hhf = {};
    kEffectX(kraycasting* owner);
    void rendah(jint n);
};

struct kSoundCommand : EngineObject {
    jint commandcode = {};
    Ref<kSound> snd = {};
    kSoundCommand(kraycasting* owner);
    void MakeSoundCommand();
};

struct kPartX : EngineObject {
    jint NBEffect = {};
    Array<Ref<kEffectX>> kEffectXPartX = {};
    jint TimeLength = {};
    jint TimeStart = {};
    jint NBSoundCommands = {};
    Array<Ref<kSoundCommand>> SoundCommands = {};
    kPartX(kraycasting* owner);
    void rendah(jint n);
};

struct kScript : EngineObject {
    jint NBPart = {};
    jint EndDate = {};
    jint EndPref = {};
    Array<Ref<kPartX>> kPartXscript = {};
    kPartX* lastpartplayed = {};
    kScript(kraycasting* owner, String string);
    Ref<kPartX> part(jint n);
};

struct raycasting : EngineObject {
    Ref<kRenderContext> OneRec = {};
    Ref<kRenderContext> Rootrc = {};
    raycasting(kraycasting* owner, Ref<kRenderContext> kRenderContext2);
    void MotionBlur(Ref<kRenderContext> kRenderContext2);
    void Position3D(double d, double d2, double d3, double d4, double d5, double d6, double d7, Ref<kRenderContext> kRenderContext2);
    void Setzoom3D(double d, double d2, double d3, double d4, double d5, double d6, Ref<kRenderContext> kRenderContext2, Ref<kTexture> kTexture2, double d7);
    void SetzoomP(double d, double d2, double d3, double d4, Ref<kRenderContext> kRenderContext2, Ref<kTexture> kTexture2, double d5);
    void colorclip(Ref<helpfade> helpfade2, Ref<kPixel> kPixel2, double d);
    void draw(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2);
    void drawrecUV(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5);
    void drawrecUVL(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5);
    void drawrecUVM(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5);
    void drawrecUVRGB(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5);
    void drawrecUVS(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5);
    void drawrecUVSA(Ref<kTexture> kTexture2, Ref<kRenderContext> kRenderContext2, Ref<kPixel> kPixel2, Ref<kPixel> kPixel3, Ref<kPixel> kPixel4, Ref<kPixel> kPixel5, jint n);
    void findfloor(double d, double d2, double d3, double d4, Ref<kRenderContext> kRenderContext2, Ref<helpfade> helpfade2);
    void findtunnel(double d, double d2, Ref<kRenderContext> kRenderContext2, Ref<helpfade> helpfade2);
    void findtwirl(double d, double d2, double d3, double d4, double d5, double d6, Ref<kRenderContext> kRenderContext2);
    void flare3D(double d, double d2, double d3, double d4, double d5, double d6, Ref<kRenderContext> kRenderContext2, Ref<kTexture> kTexture2);
};

struct kSound : EngineObject {
    Ref<DemoAudio> OurAudio = {};
    bool soundIsOn = {};
    bool playedonce = {};
    kSound* nextkSound = nullptr;
    kSound(kraycasting* owner, Ref<DemoAudio> audioClip);
    void LoopOne();
    void PlayOne();
    void StopAll();
    void StopOne();
};
