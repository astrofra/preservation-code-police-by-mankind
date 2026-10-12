/* Code Police — embedded CPIX pack access (milestone N1b, m68k only).
 *
 * When the executable is built with EMBED=1 (build.sh), the offline pack
 * (amiga/build/native-assets/assets.cpix) is assembled into read-only data by
 * amiga/build/gen_blob.sh and linked in. The target then loads its textures with
 * no file I/O at all: the pack is already in the image, so there is nothing to
 * open, decode or allocate during start-up but the texture views.
 *
 * On the host there is no embedded pack; agart_load_embedded returns NULL and the
 * callers fall back to loading the file.
 */
#include "agart.h"

#if defined(AGART_EMBEDDED)

/* Produced by amiga/build/gen_blob.sh; the labels carry the C ABI underscore
 * prefix (_agart_pack / _agart_pack_end), which the C names below resolve to. */
extern unsigned char agart_pack[];
extern unsigned char agart_pack_end[];

AgartPart *agart_load_embedded(void) {
    /* The pack is complete by construction, so the bound is just the distance
     * between the two labels; the parser still validates every entry offset
     * against it, so a corrupt link-time blob would be rejected. */
    return agart_load_memory(agart_pack, (size_t)(agart_pack_end - agart_pack));
}

#else

AgartPart *agart_load_embedded(void) { return NULL; }

#endif
