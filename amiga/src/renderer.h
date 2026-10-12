/* Code Police — renderer core (C), milestone 3.
 *
 * A direct port of the kRenderContext / raycasting classes in
 * native-sdl2/src/engine.cpp. Struct fields and function names track the C++
 * originals so each function can be diffed against its source. Buffers are
 * heap arrays indexed by jint, exactly as the originals' Array<jint>.
 */
#ifndef RENDERER_H
#define RENDERER_H

#include "jint_compat.h"
#include <stddef.h>

typedef struct kpixel {
    jint u, v, r, g, b;
    double XI, YI, ZI, XF, YF, ZF;
} kpixel;

typedef struct texture {
    jint Modulo, Height;
    int32_t *pixels;
} texture;

typedef struct helpfade {
    double Frd1, Frv1, Frd2, Frv2;
    double Fgd1, Fgv1, Fgd2, Fgv2;
    double Fbd1, Fbv1, Fbd2, Fbv2;
} helpfade;

typedef struct rcontext {
    jint ClipX1, ClipY1, ClipX2, ClipY2;
    jint Modulo, Height;
    int32_t *Chunk2;
    int32_t *CurrentChunk;
    struct rcontext *RC_RootContext;
    struct rcontext *RC_FatherContext;
    jint MaxX, MaxY, MaxPreX, MaxPreY;
    kpixel **Pretable;             /* MaxPreX * MaxPreY, row-major [x + y*MaxPreX] */
    jint PretableX, PretableY;     /* allocation extents, for safe indexing */
    double PosX, PosY, PosZ;
    double floorv, ceiling;
    int drawable;
} rcontext;

typedef struct rcaster {
    rcontext *Rootrc;
    rcontext *OneRec;
} rcaster;

/* kRenderContext(owner, applet): the screen-sized root, allocating the two
 * chunky buffers and the pixel grid from the screen size. */
void rcr_init_root(rcontext *rc, jint width, jint height);
/* kRenderContext(owner, root): the shallow copy used for OneRec. */
void rcr_init_child(rcontext *rc, rcontext *parent);
/* kRenderContext(owner, root, d, d2, d3, d4): a sub-rectangle. */
void rcr_init_rect(rcontext *rc, rcontext *parent, double d, double d2, double d3, double d4);
void rcr_setrect(rcontext *rc, rcontext *parent, double d, double d2, double d3, double d4);
void rcr_setsub(rcontext *rc, rcontext *parent, jint x1, jint y1, jint x2, jint y2);
int32_t *rcr_create_image(rcontext *rc);
void rcr_free(rcontext *rc);

void rcs_init(rcaster *rcs, rcontext *root);

/* raycasting.MotionBlur */
void rcs_motion_blur(rcaster *rcs, rcontext *rc);
/* raycasting.Position3D */
void rcs_position3d(rcaster *rcs, double d, double d2, double d3, double d4,
                    double d5, double d6, double d7, rcontext *rc);
/* raycasting.Setzoom3D / SetzoomP */
void rcs_setzoom3d(rcaster *rcs, double d, double d2, double d3, double d4,
                   double d5, double d6, rcontext *rc, texture *tx, double d7);
void rcs_setzoomp(rcaster *rcs, double d, double d2, double d3, double d4,
                  rcontext *rc, texture *tx, double d5);
/* raycasting.flare3D */
void rcs_flare3d(rcaster *rcs, double d, double d2, double d3, double d4,
                 double d5, double d6, rcontext *rc, texture *tx);
/* raycasting.findfloor / findtunnel / findtwirl */
void rcs_findfloor(rcaster *rcs, double d, double d2, double d3, double d4,
                   rcontext *rc, helpfade *hf);
void rcs_findtunnel(rcaster *rcs, double d, double d2, rcontext *rc, helpfade *hf);
void rcs_findtwirl(rcaster *rcs, double d, double d2, double d3, double d4,
                   double d5, double d6, rcontext *rc);
/* raycasting.colorclip */
void rcs_colorclip(rcaster *rcs, helpfade *hf, kpixel *px, double d);

#endif /* RENDERER_H */
