/* Code Police — fixed-point camera projection and sprite UV (milestone N4).
 *
 * Two numeric cores stand between the validated generators (N3) and a whole scene
 * interpreter: the camera projection that fills each block's ray (engine
 * raycasting.Position3D) and the sprite placement that produces the clipped screen
 * rectangle and the four texture corners (engine raycasting.Setzoom3D / SetzoomP).
 * Both are ported here in Q16.16 so no floating point is needed to drive the
 * effect tree.
 *
 * The engine's arguments stay doubles at the effect boundary (they come from the
 * parameter tree); fix16_from_double converts them once per effect call.
 */
#ifndef AGART_PROJ_H
#define AGART_PROJ_H

#include "agart_fix.h"

/* The camera an effect sets through Setcamcoord / Setcamtarget. */
typedef struct AgartCamera {
    fix16 X, Y, Z, O1, O2, O3, FOV;
} AgartCamera;

/* The rectangle a SetRect effect selects, in screen pixels. */
typedef struct AgartBox {
    int x1, y1, x2, y2;
} AgartBox;

/* raycasting.Position3D: fills rays[i*3..] with XF,YF,ZF for the block grid of
 * `box` (mx by my blocks). XI/YI are recomputed from the box, matching the
 * engine's Pretable layout. */
void agart_position3d_fix(const AgartCamera *cam, const AgartBox *box,
                          int mx, int my, fix16 *rays);

/* raycasting.Setzoom3D. Returns 1 when the sprite is drawable, 0 otherwise, and
 * fills out_box (the clipped screen rectangle) plus the four corners in the
 * primitive's order (top-left, top-right, bottom-left, bottom-right): out_u[0..3]
 * and out_v[0..3] are the Q16.16 texel coordinates of each. d7 is the alpha. */
int agart_setzoom3d_fix(fix16 d, fix16 d2, fix16 d3, fix16 d4, fix16 d5, fix16 d6,
                        int tex_modulo, int tex_height, const AgartBox *clip,
                        AgartBox *out_box, fix16 out_u[4], fix16 out_v[4]);

/* raycasting.SetzoomP: the 2D sprite variant; d..d4 are normalized screen
 * fractions, d5 is the alpha. Same output convention. */
int agart_setzoomp_fix(fix16 d, fix16 d2, fix16 d3, fix16 d4,
                       int tex_modulo, int tex_height, const AgartBox *clip,
                       AgartBox *out_box, fix16 out_u[4], fix16 out_v[4]);

#endif /* AGART_PROJ_H */
