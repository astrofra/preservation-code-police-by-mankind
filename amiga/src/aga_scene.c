/* Code Police — indexed scene interpreter (milestone N4). See aga_scene.h.
 *
 * The native counterpart of objects.c's effect_rendah: for every effect in the
 * part, evaluate the parameters through the shared tree, resolve the objects,
 * and call the indexed primitive. Geometry comes from agart_proj.c, the generators
 * from agart_gen.c, and the pixels land in the 4-bit context of agart_ctx.c.
 */
#include "aga_scene.h"
#include "agart_proj.h"
#include "agart_trig.h"

#include <stdlib.h>
#include <string.h>

/* Effect codes, matching objects.c. */
enum {
    FX_TWIRL = 1, FX_SPRIT = 2, FX_TUNNEL = 3, FX_GROUND = 4, FX_SPRIT3D = 5,
    FX_SETRECT = 6, FX_FLARE3D = 7, FX_MOTIONBLUR = 8, FX_PARTICLE = 9,
    FX_SETCAMCOORD = 10, FX_SETCAMTARGET = 11, FX_EVESOUND = 12
};

#define CLAMP255(v) ((int)((v) < 0 ? 0 : (v) > 255 ? 255 : (v)))

static fix16 tick_fix(jint tick) {
    return (fix16)((int64_t)tick << FIX16_SHIFT);
}

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

/* Alpha is quantised to the same byte the original effect exposes. Keeping all
 * 256 tables in Fast RAM removes a 16x16x16 colour-distance search from every
 * sprite draw. The returned table is read-only for the whole scene lifetime. */
static const uint8_t (*scene_blend(const AgartScene *s, int alpha))[16] {
    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;
    return (const uint8_t (*)[16])(s->blend_lut + (size_t)alpha * 16u * 16u);
}

/* The fade helper an effect carries (helpfade): pairs of (depth, colour). */
static void fade_set(AgartFade *hf, jworld *w, jeffect *e, int first, fix16 time) {
    hf->Frd1 = jparam_fixout(e->params[first + 0], w, time);
    hf->Frv1 = jparam_fixout(e->params[first + 1], w, time);
    hf->Frd2 = jparam_fixout(e->params[first + 2], w, time);
    hf->Frv2 = jparam_fixout(e->params[first + 3], w, time);
    hf->Fgd1 = jparam_fixout(e->params[first + 4], w, time);
    hf->Fgv1 = jparam_fixout(e->params[first + 5], w, time);
    hf->Fgd2 = jparam_fixout(e->params[first + 6], w, time);
    hf->Fgv2 = jparam_fixout(e->params[first + 7], w, time);
    hf->Fbd1 = jparam_fixout(e->params[first + 8], w, time);
    hf->Fbv1 = jparam_fixout(e->params[first + 9], w, time);
    hf->Fbd2 = jparam_fixout(e->params[first + 10], w, time);
    hf->Fbv2 = jparam_fixout(e->params[first + 11], w, time);
}

/* A camera parameter is one of the object pointers; read it in fixed point. */
static void read_cam(jworld *w, jcamera *cam, AgartCamera *out) {
    out->X = cam->fX; out->Y = cam->fY; out->Z = cam->fZ;
    out->O1 = cam->fO1; out->O2 = cam->fO2; out->O3 = cam->fO3;
    out->FOV = cam->fFOV;
    (void)w;
}

/* Fixed counterpart of kCamera.setTarget. The extra turns keep the same
 * orientation convention as the Java port; angles are allowed to wrap because
 * the fixed trig helpers reduce them before evaluation. */
static void camera_target_fix(jcamera *cam, fix16 x, fix16 y, fix16 z,
                              fix16 target_x, fix16 target_y, fix16 target_z,
                              fix16 fov, fix16 roll) {
    fix16 dx = target_x - x, dy = target_y - y, dz = target_z - z;
    fix16 heading, forward, pitch;
    if (dz == 0) dz = 1;
    heading = fix16_atan2(dx, dz);
    heading += dz > 0 ? FIX16_TWO_PI : FIX16_PI;
    forward = fix16_mul(dz, fix16_cos(heading)) + fix16_mul(dx, fix16_sin(heading));
    if (forward == 0) forward = 1;
    pitch = fix16_atan2(dy, forward);
    pitch += forward > 0 ? FIX16_TWO_PI : FIX16_PI;
    cam->fX = x; cam->fY = y; cam->fZ = z;
    cam->fFOV = fov; cam->fO1 = roll; cam->fO3 = heading; cam->fO2 = -pitch;
}

/* The generator effects (ground, tunnel, twirl): project the camera, fill a block
 * grid, then draw it with the block shading colour (drawrecUVRGB). */
static void scene_generator(AgartScene *s, jworld *w, jeffect *e, fix16 time) {
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
        fade_set(&hf, w, e, 7, time);
        agart_floor_fix(s->rays, mx, my, ac.X, ac.Y, ac.Z,
                        jparam_fixout(e->params[1], w, time),
                        jparam_fixout(e->params[2], w, time),
                        jparam_fixout(e->params[5], w, time),
                        jparam_fixout(e->params[6], w, time),
                        &hf, &s->lut, s->blocks);
        t = tex_of(w, s->pack, jparam_out(e->params[3], w));
    } else if (e->keffectcode == FX_TUNNEL) {
        jcamera *cam = (jcamera *)jparam_out(e->params[0], w);
        read_cam(w, cam, &ac);
        agart_position3d_fix(&ac, &box, mx, my, s->rays);
        fade_set(&hf, w, e, 5, time);
        agart_tunnel_fix(s->rays, mx, my, ac.Z,
                         jparam_fixout(e->params[1], w, time),
                         jparam_fixout(e->params[2], w, time),
                         &hf, &s->lut, s->blocks);
        t = tex_of(w, s->pack, jparam_out(e->params[3], w));
    } else { /* twirl: no camera, PosX/PosY come from parameters 0 and 1. */
        ac.X = jparam_fixout(e->params[0], w, time);
        ac.Y = jparam_fixout(e->params[1], w, time);
        ac.Z = 0; ac.O1 = 0; ac.O2 = 0; ac.O3 = 0; ac.FOV = 0;
        agart_position3d_fix(&ac, &box, mx, my, s->rays);
        agart_twirl_fix(s->rays, mx, my, ac.X, ac.Y,
                        jparam_fixout(e->params[2], w, time),
                        jparam_fixout(e->params[3], w, time),
                        jparam_fixout(e->params[4], w, time),
                        jparam_fixout(e->params[5], w, time),
                        jparam_fixout(e->params[6], w, time),
                        jparam_fixout(e->params[7], w, time),
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
static void scene_effect(AgartScene *s, jworld *w, jeffect *e, fix16 time) {
    switch (e->keffectcode) {
    case FX_SPRIT: { /* 2D sprite: SetzoomP, alpha blend */
        const AgartTexture *t = tex_of(w, s->pack, jparam_out(e->params[0], w));
        fix16 alpha = jparam_fixout(e->params[6], w, time);
        if (t) {
            AgartBox clip, out;
            fix16 u[4], v[4];
            AgartUV uv[4];
            fill_clip(s, &clip);
            if (agart_setzoomp_fix(
                    jparam_fixout(e->params[1], w, time),
                    jparam_fixout(e->params[2], w, time),
                    jparam_fixout(e->params[3], w, time),
                    jparam_fixout(e->params[4], w, time),
                    t->width, t->height, &clip, &out, u, v)) {
                s->ctx.rect.x1 = out.x1; s->ctx.rect.y1 = out.y1;
                s->ctx.rect.x2 = out.x2; s->ctx.rect.y2 = out.y2;
                uv[0].u = u[0]; uv[0].v = v[0];
                uv[1].u = u[1]; uv[1].v = v[1];
                uv[2].u = u[2]; uv[2].v = v[2];
                uv[3].u = u[3]; uv[3].v = v[3];
                agart_blit_alpha(&s->ctx, t, uv,
                                 scene_blend(s, CLAMP255((int)(alpha >> 16))));
                ++s->draws;
            }
        }
        break;
    }
    case FX_SPRIT3D: { /* 3D sprite: Setzoom3D, alpha blend */
        const AgartTexture *t = tex_of(w, s->pack, jparam_out(e->params[6], w));
        fix16 alpha = jparam_fixout(e->params[8], w, time);
        if (t) {
            AgartBox clip, out;
            fix16 u[4], v[4];
            AgartUV uv[4];
            fill_clip(s, &clip);
            if (agart_setzoom3d_fix(
                    jparam_fixout(e->params[0], w, time),
                    jparam_fixout(e->params[1], w, time),
                    jparam_fixout(e->params[2], w, time),
                    jparam_fixout(e->params[3], w, time),
                    jparam_fixout(e->params[4], w, time),
                    jparam_fixout(e->params[5], w, time),
                    t->width, t->height, &clip, &out, u, v)) {
                s->ctx.rect.x1 = out.x1; s->ctx.rect.y1 = out.y1;
                s->ctx.rect.x2 = out.x2; s->ctx.rect.y2 = out.y2;
                uv[0].u = u[0]; uv[0].v = v[0];
                uv[1].u = u[1]; uv[1].v = v[1];
                uv[2].u = u[2]; uv[2].v = v[2];
                uv[3].u = u[3]; uv[3].v = v[3];
                agart_blit_alpha(&s->ctx, t, uv,
                                 scene_blend(s, CLAMP255((int)(alpha >> 16))));
                ++s->draws;
            }
        }
        break;
    }
    case FX_GROUND:
    case FX_TUNNEL:
    case FX_TWIRL:
        scene_generator(s, w, e, time);
        break;
    case FX_FLARE3D: {
        const AgartTexture *t = tex_of(w, s->pack, jparam_out(e->params[6], w));
        if (t) {
            AgartBox clip, out;
            fix16 u[4], v[4];
            AgartUV uv[4];
            fill_clip(s, &clip);
            if (agart_setzoom3d_fix(
                    jparam_fixout(e->params[0], w, time),
                    jparam_fixout(e->params[1], w, time),
                    jparam_fixout(e->params[2], w, time),
                    jparam_fixout(e->params[3], w, time),
                    jparam_fixout(e->params[4], w, time),
                    jparam_fixout(e->params[5], w, time),
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
    case FX_SETCAMCOORD: {
        jcamera *cam = (jcamera *)jparam_out(e->params[0], w);
        if (cam) {
            cam->fX = jparam_fixout(e->params[1], w, time);
            cam->fY = jparam_fixout(e->params[2], w, time);
            cam->fZ = jparam_fixout(e->params[3], w, time);
            cam->fO1 = jparam_fixout(e->params[4], w, time);
            cam->fO2 = jparam_fixout(e->params[5], w, time);
            cam->fO3 = jparam_fixout(e->params[6], w, time);
            cam->fFOV = jparam_fixout(e->params[7], w, time);
        }
        break;
    }
    case FX_SETCAMTARGET: {
        jcamera *cam = (jcamera *)jparam_out(e->params[0], w);
        if (cam) {
            camera_target_fix(cam,
                jparam_fixout(e->params[1], w, time),
                jparam_fixout(e->params[2], w, time),
                jparam_fixout(e->params[3], w, time),
                jparam_fixout(e->params[4], w, time),
                jparam_fixout(e->params[5], w, time),
                jparam_fixout(e->params[6], w, time),
                jparam_fixout(e->params[7], w, time),
                jparam_fixout(e->params[8], w, time));
        }
        break;
    }
    default:
        break;   /* setrect, particle and sound remain later milestones. */
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
    s->blend_lut = (uint8_t *)malloc(256u * 16u * 16u);
    s->rays = (fix16 *)calloc((size_t)max_blocks * 3, sizeof(fix16));
    s->blocks = (AgartBlock *)calloc((size_t)max_blocks, sizeof(AgartBlock));
    if (!s->front || !s->back || !s->blend_lut || !s->rays || !s->blocks) {
        aga_scene_free(s);
        return NULL;
    }
    s->max_blocks = max_blocks;
    agart_tables_build(&s->tables, pack);
    {
        int alpha;
        for (alpha = 0; alpha < 256; ++alpha)
            agart_blend_build((uint8_t (*)[16])(s->blend_lut +
                              (size_t)alpha * 16u * 16u),
                              pack, alpha);
    }
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
    free(s->blend_lut);
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
        for (i = 0; i < part->nb_effect; ++i)
            scene_effect(s, w, part->effects[i], tick_fix(n2));
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
