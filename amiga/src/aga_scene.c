/* Code Police — indexed scene interpreter (milestone N4). See aga_scene.h.
 *
 * The native counterpart of objects.c's effect_rendah: for every effect in the
 * part, evaluate the parameters through the shared tree, resolve the objects,
 * and call the indexed primitive. Geometry comes from agart_proj.c, the generators
 * from agart_gen.c, and the pixels land in the 4-bit context of agart_ctx.c.
 */
#include "aga_scene.h"
#include "agart_proj.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Effect codes, matching objects.c. */
enum {
    FX_TWIRL = 1, FX_SPRIT = 2, FX_TUNNEL = 3, FX_GROUND = 4, FX_SPRIT3D = 5,
    FX_SETRECT = 6, FX_FLARE3D = 7, FX_MOTIONBLUR = 8, FX_PARTICLE = 9,
    FX_SETCAMCOORD = 10, FX_SETCAMTARGET = 11, FX_EVESOUND = 12
};

#define F(x) ((fix16)((int64_t)llround(x * 65536.0)))
#define CLAMP255(v) ((int)((v) < 0 ? 0 : (v) > 255 ? 255 : (v)))

static const AgartTexture *tex_of(jworld *w, const AgartPart *pack, const void *obj) {
    int i;
    for (i = 0; i < w->texture_count; ++i)
        if (w->textures[i] == obj) return agart_find(pack, w->texture_names[i]);
    return NULL;
}

static void fill_clip(const AgartScene *s, AgartBox *box) {
    box->x1 = 0; box->y1 = 0;
    box->x2 = s->width; box->y2 = s->height;
}

/* The fade helper an effect carries (helpfade): pairs of (depth, colour). */
static void fade_set(AgartFade *hf, jworld *w, jeffect *e, int first, jint d2) {
    hf->Frd1 = F(jparam_doubleout(e->params[first + 0], w, (double)d2));
    hf->Frv1 = F(jparam_doubleout(e->params[first + 1], w, (double)d2));
    hf->Frd2 = F(jparam_doubleout(e->params[first + 2], w, (double)d2));
    hf->Frv2 = F(jparam_doubleout(e->params[first + 3], w, (double)d2));
    hf->Fgd1 = F(jparam_doubleout(e->params[first + 4], w, (double)d2));
    hf->Fgv1 = F(jparam_doubleout(e->params[first + 5], w, (double)d2));
    hf->Fgd2 = F(jparam_doubleout(e->params[first + 6], w, (double)d2));
    hf->Fgv2 = F(jparam_doubleout(e->params[first + 7], w, (double)d2));
    hf->Fbd1 = F(jparam_doubleout(e->params[first + 8], w, (double)d2));
    hf->Fbv1 = F(jparam_doubleout(e->params[first + 9], w, (double)d2));
    hf->Fbd2 = F(jparam_doubleout(e->params[first + 10], w, (double)d2));
    hf->Fbv2 = F(jparam_doubleout(e->params[first + 11], w, (double)d2));
}

/* A camera parameter is one of the object pointers; read it in fixed point. */
static void read_cam(jworld *w, jcamera *cam, AgartCamera *out) {
    out->X = F(cam->X); out->Y = F(cam->Y); out->Z = F(cam->Z);
    out->O1 = F(cam->O1); out->O2 = F(cam->O2); out->O3 = F(cam->O3);
    out->FOV = F(cam->FOV);
    (void)w;
}

/* The generator effects (ground, tunnel, twirl): project the camera, fill a block
 * grid, then draw it with the block shading colour (drawrecUVRGB). */
static void scene_generator(AgartScene *s, jworld *w, jeffect *e, jint d2) {
    int mx = (s->width + 15) / 16, my = (s->height + 15) / 16;
    AgartFade hf;
    AgartBox box;
    AgartCamera ac;
    const AgartTexture *t;
    fill_clip(s, &box);
    if (e->keffectcode == FX_GROUND) {
        jcamera *cam = (jcamera *)jparam_out(e->params[0], w);
        read_cam(w, cam, &ac);
        agart_position3d_fix(&ac, &box, mx, my, s->rays);
        fade_set(&hf, w, e, 7, d2);
        agart_floor_fix(s->rays, mx, my, ac.X, ac.Y, ac.Z,
                        F(jparam_doubleout(e->params[1], w, (double)d2)),
                        F(jparam_doubleout(e->params[2], w, (double)d2)),
                        F(jparam_doubleout(e->params[5], w, (double)d2)),
                        F(jparam_doubleout(e->params[6], w, (double)d2)),
                        &hf, &s->lut, s->blocks);
        t = tex_of(w, s->pack, jparam_out(e->params[3], w));
    } else if (e->keffectcode == FX_TUNNEL) {
        jcamera *cam = (jcamera *)jparam_out(e->params[0], w);
        read_cam(w, cam, &ac);
        agart_position3d_fix(&ac, &box, mx, my, s->rays);
        fade_set(&hf, w, e, 5, d2);
        agart_tunnel_fix(s->rays, mx, my, ac.Z,
                         F(jparam_doubleout(e->params[1], w, (double)d2)),
                         F(jparam_doubleout(e->params[2], w, (double)d2)),
                         &hf, &s->lut, s->blocks);
        t = tex_of(w, s->pack, jparam_out(e->params[3], w));
    } else { /* twirl: no camera, PosX/PosY come from parameters 0 and 1. */
        ac.X = F(jparam_doubleout(e->params[0], w, (double)d2));
        ac.Y = F(jparam_doubleout(e->params[1], w, (double)d2));
        ac.Z = 0; ac.O1 = 0; ac.O2 = 0; ac.O3 = 0; ac.FOV = 0;
        agart_position3d_fix(&ac, &box, mx, my, s->rays);
        agart_twirl_fix(s->rays, mx, my, ac.X, ac.Y,
                        F(jparam_doubleout(e->params[2], w, (double)d2)),
                        F(jparam_doubleout(e->params[3], w, (double)d2)),
                        F(jparam_doubleout(e->params[4], w, (double)d2)),
                        F(jparam_doubleout(e->params[5], w, (double)d2)),
                        F(jparam_doubleout(e->params[6], w, (double)d2)),
                        F(jparam_doubleout(e->params[7], w, (double)d2)),
                        &s->lut, s->blocks);
        t = tex_of(w, s->pack, jparam_out(e->params[8], w));
    }
    if (t) {
        s->ctx.rect = s->ctx.clip;
        agart_grid_colour(&s->ctx, t, s->blocks, mx, my, &s->tables);
        ++s->draws;
    }
}

/* Effect dispatcher, mirroring effect_rendah but in the indexed world. */
static void scene_effect(AgartScene *s, jworld *w, jeffect *e, jint n) {
    jint d2 = n;
    switch (e->keffectcode) {
    case FX_SPRIT: { /* 2D sprite: SetzoomP, alpha blend */
        const AgartTexture *t = tex_of(w, s->pack, jparam_out(e->params[0], w));
        double alpha = jparam_doubleout(e->params[6], w, (double)d2);
        if (t) {
            AgartBox clip, out;
            fix16 u[4], v[4];
            uint8_t blend[16][16];
            AgartUV uv[4];
            fill_clip(s, &clip);
            if (agart_setzoomp_fix(
                    F(jparam_doubleout(e->params[1], w, (double)d2)),
                    F(jparam_doubleout(e->params[2], w, (double)d2)),
                    F(jparam_doubleout(e->params[3], w, (double)d2)),
                    F(jparam_doubleout(e->params[4], w, (double)d2)),
                    t->width, t->height, &clip, &out, u, v)) {
                agart_blend_build(blend, s->pack, CLAMP255((int)alpha));
                s->ctx.rect.x1 = out.x1; s->ctx.rect.y1 = out.y1;
                s->ctx.rect.x2 = out.x2; s->ctx.rect.y2 = out.y2;
                uv[0].u = u[0]; uv[0].v = v[0];
                uv[1].u = u[1]; uv[1].v = v[1];
                uv[2].u = u[2]; uv[2].v = v[2];
                uv[3].u = u[3]; uv[3].v = v[3];
                agart_blit_alpha(&s->ctx, t, uv, blend);
                ++s->draws;
            }
        }
        break;
    }
    case FX_SPRIT3D: { /* 3D sprite: Setzoom3D, alpha blend */
        const AgartTexture *t = tex_of(w, s->pack, jparam_out(e->params[6], w));
        double alpha = jparam_doubleout(e->params[8], w, (double)d2);
        if (t) {
            AgartBox clip, out;
            fix16 u[4], v[4];
            uint8_t blend[16][16];
            AgartUV uv[4];
            fill_clip(s, &clip);
            if (agart_setzoom3d_fix(
                    F(jparam_doubleout(e->params[0], w, (double)d2)),
                    F(jparam_doubleout(e->params[1], w, (double)d2)),
                    F(jparam_doubleout(e->params[2], w, (double)d2)),
                    F(jparam_doubleout(e->params[3], w, (double)d2)),
                    F(jparam_doubleout(e->params[4], w, (double)d2)),
                    F(jparam_doubleout(e->params[5], w, (double)d2)),
                    t->width, t->height, &clip, &out, u, v)) {
                agart_blend_build(blend, s->pack, CLAMP255((int)alpha));
                s->ctx.rect.x1 = out.x1; s->ctx.rect.y1 = out.y1;
                s->ctx.rect.x2 = out.x2; s->ctx.rect.y2 = out.y2;
                uv[0].u = u[0]; uv[0].v = v[0];
                uv[1].u = u[1]; uv[1].v = v[1];
                uv[2].u = u[2]; uv[2].v = v[2];
                uv[3].u = u[3]; uv[3].v = v[3];
                agart_blit_alpha(&s->ctx, t, uv, blend);
                ++s->draws;
            }
        }
        break;
    }
    case FX_GROUND:
    case FX_TUNNEL:
    case FX_TWIRL:
        scene_generator(s, w, e, d2);
        break;
    case FX_FLARE3D: {
        const AgartTexture *t = tex_of(w, s->pack, jparam_out(e->params[6], w));
        if (t) {
            AgartBox clip, out;
            fix16 u[4], v[4];
            AgartUV uv[4];
            fill_clip(s, &clip);
            if (agart_setzoom3d_fix(
                    F(jparam_doubleout(e->params[0], w, (double)d2)),
                    F(jparam_doubleout(e->params[1], w, (double)d2)),
                    F(jparam_doubleout(e->params[2], w, (double)d2)),
                    F(jparam_doubleout(e->params[3], w, (double)d2)),
                    F(jparam_doubleout(e->params[4], w, (double)d2)),
                    F(jparam_doubleout(e->params[5], w, (double)d2)),
                    t->width, t->height, &clip, &out, u, v)) {
                s->ctx.rect.x1 = out.x1; s->ctx.rect.y1 = out.y1;
                s->ctx.rect.x2 = out.x2; s->ctx.rect.y2 = out.y2;
                uv[0].u = u[0]; uv[0].v = v[0];
                uv[1].u = u[1]; uv[1].v = v[1];
                uv[2].u = u[2]; uv[2].v = v[2];
                uv[3].u = u[3]; uv[3].v = v[3];
                agart_blit_add(&s->ctx, t, uv, &s->tables);
                ++s->draws;
            }
        }
        break;
    }
    case FX_MOTIONBLUR:
        s->ctx.rect = s->ctx.clip;
        agart_motion_blur(&s->ctx, &s->tables);
        ++s->draws;
        break;
    default:
        break;   /* setrect, particle, camera and sound are handled elsewhere. */
    }
}

AgartScene *aga_scene_create(const AgartPart *pack, int width, int height) {
    AgartScene *s = (AgartScene *)calloc(1, sizeof(AgartScene));
    int max_blocks;
    if (!s) return NULL;
    s->pack = pack;
    s->width = width;
    s->height = height;
    max_blocks = ((width + 15) / 16 + 1) * ((height + 15) / 16 + 1);
    s->front = (uint8_t *)calloc((size_t)width * height, 1);
    s->back = (uint8_t *)calloc((size_t)width * height, 1);
    s->rays = (fix16 *)calloc((size_t)max_blocks * 3, sizeof(fix16));
    s->blocks = (AgartBlock *)calloc((size_t)max_blocks, sizeof(AgartBlock));
    if (!s->front || !s->back || !s->rays || !s->blocks) { aga_scene_free(s); return NULL; }
    s->max_blocks = max_blocks;
    agart_tables_build(&s->tables, pack);
    agart_colour_lut_build(&s->lut, pack->palette_rgb);
    /* The frame is drawn into `back`; `front` holds the previous frame for motion
     * blur. After each frame they swap, so `front` always holds the last result. */
    s->ctx.pixels = s->back;
    s->ctx.previous = s->front;
    s->ctx.width = width;
    s->ctx.height = height;
    s->ctx.clip.x1 = 0; s->ctx.clip.y1 = 0;
    s->ctx.clip.x2 = width; s->ctx.clip.y2 = height;
    s->ctx.rect = s->ctx.clip;
    return s;
}

void aga_scene_free(AgartScene *s) {
    if (!s) return;
    free(s->front);
    free(s->back);
    free(s->rays);
    free(s->blocks);
    free(s);
}

int aga_scene_frame(AgartScene *s, jworld *w, jint tick) {
    jpart *part;
    int i;
    s->draws = 0;
    /* The frame clears to black, the palette's index 0, before the part runs. */
    agart_clear(&s->ctx, AGART_BLACK_INDEX);
    jr_seed(&w->random, 100000u + (uint64_t)tick);
    part = jworld_part(w, tick);
    {
        jint n2 = (tick - part->time_length) + part->time_start;
        for (i = 0; i < part->nb_effect; ++i) scene_effect(s, w, part->effects[i], n2);
    }
    /* Present: swap front/back so the just-drawn buffer becomes `front` (the one a
     * host displays and, next tick, the motion-blur source), as rcr_create_image
     * swaps Chunk and Chunk2. */
    {
        uint8_t *t = s->front;
        s->front = s->back;
        s->back = t;
    }
    s->ctx.pixels = s->back;
    s->ctx.previous = s->front;
    return s->draws;
}
