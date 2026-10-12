/* Code Police — the object graph (C), milestone 4.
 *
 * Builds the objects the script declares: textures (KIMAGES), spline tables
 * (KTABLE), cameras (KCAM), particle systems (KTICLE), render rectangles
 * (KRECT), the effect/parameter tree (KPART Fx Pa), and the part list.
 * A direct port of kScript.kScript and the parameter, effect and part classes
 * in native-sdl2/src/engine.cpp.
 */
#ifndef OBJECTS_H
#define OBJECTS_H

#include "renderer.h"
#include "java_random.h"
#include "kscript.h"
#include "amiga_platform.h"

#define MAX_NAME 64

typedef struct jtexture {
    jint Modulo, Height;
    int32_t *pixels;       /* owned */
} jtexture;

typedef struct jtable {
    jint lengthtable;
    double *datetable;
    double *floattable;
} jtable;

typedef struct jcamera {
    double X, Y, Z, O1, O2, O3, FOV;
} jcamera;

typedef struct jparticle {
    int nbparticle;
    double *posx, *posy, *posz, *Vx, *Vy, *Vz, *Ax, *Ay, *Az, *force;
    double aveX, aveY, aveZ;
} jparticle;

/* Parameter codes, matching kParameterX's CodeParam fields. */
enum {
    PARAM_OBJ = 1, PARAM_COS = 2, PARAM_SIN = 3, PARAM_CTE = 4,
    PARAM_RND = 5, PARAM_SPL = 6, PARAM_AFF = 7, PARAM_REC = 8,
    PARAM_PAR = 9, PARAM_CAM = 10, PARAM_SND = 11
};

typedef struct jparam {
    int code;              /* PARAM_* */
    double a, b, c;
    int obj_kind;          /* 1 texture, 2 camera, 3 particle, 4 rect, 5 root */
    void *obj;
} jparam;

typedef struct jeffect {
    int keffectcode;
    int nbparam;
    jparam **params;
    helpfade hhf;
} jeffect;

typedef struct jsndcmd {
    int commandcode;       /* 0 stop, 1 play, 2 loop */
    int sound_index;
} jsndcmd;

typedef struct jpart {
    int nb_effect;
    jeffect **effects;
    jint time_length, time_start;
    int nb_sound_cmd;
    jsndcmd *sounds;
} jpart;

typedef struct jworld {
    java_random random;
    int width, height;
    rcontext root;
    rcaster rcs;

    jtexture **textures;
    int texture_count;

    struct jtable **tables;
    int table_count;

    struct jcamera **cameras;
    int camera_count;

    struct jparticle **particles;
    int particle_count;

    rcontext **rects;
    int rect_count;

    jpart **parts;
    int part_count;
    int end_pref;
    int end_date;

    /* name->index lookups, so an effect parameter can resolve IMA|/CAM|/PAR|... */
    char **texture_names;
    char **table_names;
    char **camera_names;
    char **particle_names;
    char **rect_names;
} jworld;

/* Loads textures via the platform, then parses the script and builds everything.
 * Returns NULL on allocation failure. The script text is not retained. */
jworld *jworld_create(const KsScript *script, const AmigaPlatform *platform);
void jworld_free(jworld *w);

/* kScript.part: the part whose [time_length, start) range contains n. */
jpart *jworld_part(jworld *w, jint n);
/* kPartX.rendah: run every effect in the part at local time n2. */
void jworld_rendah(jworld *w, jpart *part, jint n, int *last_part_played);

/* Renders one whole-script frame: reseed to 100000+tick, pick the part, run it,
 * then swap the chunky buffer, matching kraycasting::frameAt. */
void jworld_frame(jworld *w, jint tick);
/* FNV-1a of the front buffer, matching verify.cpp's frame hash. */
uint64_t jworld_hash(jworld *w);

#endif /* OBJECTS_H */
