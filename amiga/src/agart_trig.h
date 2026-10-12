/* Code Police — integer trigonometry (milestone N3, documentation §5).
 *
 * findtunnel needs atan2 only (its cos/sin cancel inside the depth), findtwirl
 * needs sin, cos and atan2. All three are implemented with fixed-point
 * polynomials so the runtime links no libm and calls no float routine. The
 * accuracy target is loose on purpose: the results feed texture coordinates, and
 * amiga/tests/fix_trig_check.c measures the error against libm.
 *
 * Angles are Q16.16 radians, like every other quantity in the frame loop.
 */
#ifndef AGART_TRIG_H
#define AGART_TRIG_H

#include "agart_fix.h"

#define FIX16_PI ((fix16)205887)        /* pi * 65536, rounded */
#define FIX16_HALF_PI ((fix16)102944)
#define FIX16_TWO_PI ((fix16)411775)

/* Sine and cosine of a Q16.16 radian angle. */
fix16 fix16_sin(fix16 x);
fix16 fix16_cos(fix16 x);

/* atan2(y, x) in Q16.16 radians, in [-pi, pi]. x == y == 0 gives 0. */
fix16 fix16_atan2(fix16 y, fix16 x);

#endif /* AGART_TRIG_H */
