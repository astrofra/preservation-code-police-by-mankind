/* Code Police — indexed scene interpreter (milestone N4).
 *
 * Drives the verified object graph (objects.c) into the indexed 4-bit context
 * (agart_ctx.c) using the fixed-point scene maths (agart_gen.c, agart_proj.c). It
 * is the native counterpart of jworld_frame: same tick seeding, same part
 * selection, same effect order, but every primitive writes palette indices and no
 * floating point enters the pixel path.
 *
 * All buffers are owned by the scene and allocated once (documentation §3): there
 * is no allocation inside the frame loop.
 */
#ifndef AGA_SCENE_H
#define AGA_SCENE_H

#include "agart.h"
#include "agart_ctx.h"
#include "agart_gen.h"
#include "objects.h"

typedef struct AgartScene {
    const AgartPart *pack;
    AgartTables tables;
    AgartColourLut lut;
    uint8_t *blend_lut;          /* 256 alpha tables, built once at startup */
    AgartContext ctx;
    uint8_t *front;              /* current frame indices */
    uint8_t *back;               /* previous frame, for motion blur */
    fix16 *rays;                 /* block grid rays */
    AgartBlock *blocks;          /* generator output */
    int width, height;
    int max_blocks;              /* capacity of rays/blocks */
    int draws;                   /* diagnostics: primitives drawn this frame */
} AgartScene;

/* Allocates the scene for a pack. Returns NULL on failure. */
AgartScene *aga_scene_create(const AgartPart *pack, int width, int height);
void aga_scene_free(AgartScene *s);

/* Renders one whole-script frame at `tick` into the scene's front buffer, exactly
 * as jworld_frame does (reseed, pick the part, run it, swap buffers). Returns the
 * number of primitives drawn. */
int aga_scene_frame(AgartScene *s, jworld *w, jint tick);

#endif /* AGA_SCENE_H */
