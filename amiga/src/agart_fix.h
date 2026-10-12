/* Code Police — Q16.16 fixed-point core (milestone N3, documentation §5).
 *
 * The native runtime forbids floating point in the frame loop. The engine's
 * generators (findfloor, findtunnel, findtwirl) already store their texture
 * coordinates as Q16.16 integers (jint(u * 65536)), so the port only has to
 * reproduce those integers with integer arithmetic: the representation is the
 * same, only the intermediate maths changes.
 *
 * The type is a signed 32-bit fix16, i.e. the value x represents x / 65536. On the
 * 68020 and 68060 this is the native operand width, so the hot paths stay in
 * registers. Every helper widens to 64 bits where a product would otherwise
 * overflow, and the tests in amiga/tests/fix_check.c pin the rounding.
 */
#ifndef AGART_FIX_H
#define AGART_FIX_H

#include <stdint.h>

typedef int32_t fix16;

#define FIX16_SHIFT 16
#define FIX16_ONE ((fix16)1 << FIX16_SHIFT)

/* Q16.16 multiply and divide, truncating toward zero like the engine's jint().
 * The 64-bit intermediates keep a product of two ~32767 values exact. */
fix16 fix16_mul(fix16 a, fix16 b);
fix16 fix16_div(fix16 a, fix16 b);

/* Square root of a non-negative Q16.16 value, result Q16.16. Uses a 64-bit
 * integer square root of (a << 16), which is exact to the last bit. */
fix16 fix16_sqrt(fix16 a);

/* sqrt(x*x + y*y + z*z) with the sum accumulated at 64 bits so the squares of
 * values above ~256 do not overflow before the root is taken. This is the depth
 * the generators feed to colorclip. */
fix16 fix16_len3(fix16 x, fix16 y, fix16 z);

/* Clamps to [lo, hi]; the generators rely on the engine's implicit range. */
fix16 fix16_clamp(fix16 v, fix16 lo, fix16 hi);

#endif /* AGART_FIX_H */
