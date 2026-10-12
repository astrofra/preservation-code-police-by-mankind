/* Code Police — native indexed asset format (CPIX), format version 1.
 *
 * documentation/amiga-native-runtime.md §4 makes the Amiga path indexed: the
 * runtime opens no PNG, keeps no 32-bit ARGB pixel and never quantises during a
 * frame. This header is the on-disk contract between the offline preparation
 * tool (amiga/tools/build_assets.py) and the runtime loader (amiga/src/agart.c).
 *
 * One file holds the whole demo: a single global 16-colour AGA palette and one
 * indexed 4-bit variant per texture name the script uses. A single palette is the
 * user's choice; it costs some fidelity against per-scene palettes but removes the
 * per-part texture duplication and keeps every scene consistent.
 *
 * Indexing rules reserve two of the 16 indices for the framebuffer:
 *   index 0  = black; the buffer clears to it, and it is the neutral element of
 *              the additive pair table, so the additive operators
 *              (drawrecUVRGB, drawrecUVM) keep their meaning;
 *   index 15 = the transparency key. The engine keys on 0xFFFFFFFF
 *              (drawrecUVS/drawrecUVSA skip a texel whose ARGB is -1); opaque
 *              textures map exactly those pixels here, and the primitive skips
 *              index 15 instead of comparing a 32-bit value.
 * Indices 1..14 are the 14 derived scene colours.
 *
 * Two operators need different texel mappings from the same hardware palette:
 *   role 0 (opaque)   : key -> 15, every other colour -> nearest of 0..14;
 *   role 1 (additive) : no key, every colour -> nearest of 0..15 (a near-white
 *                       texel may land on 15, which adds white, as the original
 *                       additive path would).
 * A texture has a single role across the visible parts; this was measured across
 * the script and holds without exception, so no texel needs both mappings at once.
 *
 * Texels are packed two per byte, leftmost pixel in the high nibble, so the
 * format matches the planar bit order the C2P uses. Rows are padded to an even
 * byte count. All multi-byte fields are big-endian.
 */
#ifndef AGART_H
#define AGART_H

#include <stddef.h>
#include <stdint.h>

#define AGART_MAGIC "CPIX"
#define AGART_VERSION 1
#define AGART_PALETTE_SIZE 16
#define AGART_BLACK_INDEX 0
#define AGART_KEY_INDEX 15

/* role */
#define AGART_ROLE_OPAQUE 0
#define AGART_ROLE_ADDITIVE 1

#define AGART_NAME_LEN 16

/* Packed texture header, exactly as it lies in the file. */
typedef struct AgartEntry {
    char name[AGART_NAME_LEN];  /* NUL-padded, lower-case, as IMA| spells it */
    uint8_t role;               /* AGART_ROLE_* */
    uint8_t flags;              /* bit 0: keyed (opaque textures only) */
    uint16_t width;
    uint16_t height;
    uint16_t stride;            /* bytes per row, even */
    uint32_t offset;            /* byte offset of the pixels within the file */
    uint32_t size;              /* stride * height */
} AgartEntry;

/* In-memory indexed texture, the runtime view of one AgartEntry. */
typedef struct AgartTexture {
    const char *name;
    uint8_t role;
    uint8_t flags;
    uint16_t width, height;
    uint16_t stride;
    const uint8_t *pixels;      /* points into the loaded pack */
} AgartTexture;

/* The loaded pack: the global palette as AGA 12-bit registers plus every indexed
 * texture. The whole blob is one allocation; free with agart_free. */
typedef struct AgartPart {
    uint8_t palette_rgb[AGART_PALETTE_SIZE][3];  /* 8-bit, for the pair tables */
    uint16_t palette[AGART_PALETTE_SIZE];        /* AGA 12-bit registers */
    int count;
    int owns_blob;             /* 0 for the embedded (rodata) pack */
    AgartTexture *textures;
    uint8_t *blob;             /* the pack bytes; released only if owns_blob */
} AgartPart;

/* Loads the pack from a file. Returns NULL on a bad magic, a wrong version, a
 * truncated file or an allocation failure. */
AgartPart *agart_load(const char *path);

/* Parses a pack already in memory. `blob` must stay valid for the part's life
 * (the textures are views into it) and is freed by agart_free. Used for the
 * embedded build, where the pack is linked into the executable. */
AgartPart *agart_load_memory(uint8_t *blob, size_t size);

/* The embedded pack, when the executable was built with the blob linked in
 * (build.sh EMBED=1). Returns NULL otherwise. */
AgartPart *agart_load_embedded(void);

void agart_free(AgartPart *part);

/* Looks up a texture by its IMA| name. Returns NULL when the pack does not
 * contain that texture. */
const AgartTexture *agart_find(const AgartPart *part, const char *name);

/* Reads one packed texel: index 0..15 of pixel (x, y), high nibble first. */
static inline uint8_t agart_texel(const AgartTexture *t, unsigned x, unsigned y) {
    const uint8_t b = t->pixels[(size_t)y * t->stride + (x >> 1)];
    return (uint8_t)((x & 1u) ? (b & 0x0Fu) : (b >> 4));
}

#endif /* AGART_H */
