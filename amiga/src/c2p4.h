/* Code Police — chunky-to-planar, 4 bitplanes (C).
 *
 * Converts a byte-per-pixel 4-bit chunky buffer to four separate bitplanes, the
 * AGA PF1 layout this port targets (documentation/amiga-aga-feasibility.md
 * §5). The contract matches the four-plane Kalms routine the study adopts:
 * plane 0 holds logical bit 0, plane 3 holds bit 3; bit 7 is the leftmost pixel
 * of each destination byte; only the low nibble of every source byte is used.
 *
 * This portable C version is the correctness oracle and a fallback. The Amiga
 * build selects the 68020+ Kalms transpose for widths divisible by 32; the
 * differential test in amiga/validate_c2p.py checks both host and m68k paths
 * against a Python reference.
 */
#ifndef C2P4_H
#define C2P4_H

#include <stddef.h>
#include <stdint.h>

#define C2P4_PLANES 4

/* Converts one viewport. chunky is width*height bytes (low nibble significant);
 * planes[p] receives width/8 bytes per row. width must be a non-zero multiple
 * of 8; chunky_stride >= width and plane_stride >= width/8 allow row padding.
 * The Amiga assembly path additionally relies on the renderer's invariant that
 * source bytes are already in 0..15. Returns 0 on success, non-zero on invalid
 * arguments. */
int c2p4_byte4(const uint8_t *chunky, size_t width, size_t height,
               size_t chunky_stride, uint8_t *planes[C2P4_PLANES],
               size_t plane_stride);

#endif /* C2P4_H */
