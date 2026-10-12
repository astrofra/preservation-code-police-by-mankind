/* Code Police — CPIX loader (milestone N1).
 *
 * Reads the offline indexed part files produced by amiga/tools/build_assets.py
 * (format in amiga/src/agart.h). This is the only asset path the native runtime
 * has: no PNG, no zlib, no 32-bit pixel, no floating point. The reader walks the
 * header, checks every offset and size against the file length (so a truncated
 * or corrupt file is rejected rather than trusted), and hands back views into a
 * single allocation that agart_free releases.
 *
 * File layout (big-endian multi-byte fields), offsets in bytes:
 *   0   magic "CPIX"
 *   4   version (u8), 5 flags (u8), 6 count (u16)
 *   8   16 x RGB triples (the AGA palette)
 *   56  count x 32-byte entries:
 *         0  name[16] NUL-padded
 *         16 role (u8), 17 flags (u8)
 *         18 width (u16), 20 height (u16), 22 stride (u16)
 *         24 offset (u32, from file start), 28 size (u32)
 *   then the packed 4bpp pixels
 */
#include "agart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AGART_HEADER 56
#define AGART_ENTRY 32

static uint16_t be16(const uint8_t *p) { return (uint16_t)((p[0] << 8) | p[1]); }

static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

/* Parses a pack. When `owns` is non-zero the blob is heap-owned and is released by
 * agart_free; the embedded (rodata) pack passes 0 so its bytes are never freed. */
static AgartPart *agart_parse(uint8_t *bytes, size_t length, int owns) {
    AgartPart *part;
    int count, i;
    size_t data_start;
    if (length < AGART_HEADER) { if (owns) free(bytes); return NULL; }
    if (memcmp(bytes, AGART_MAGIC, 4) != 0 || bytes[4] != AGART_VERSION) {
        if (owns) free(bytes);
        return NULL;
    }
    count = be16(bytes + 6);
    data_start = AGART_HEADER + (size_t)count * AGART_ENTRY;
    if (data_start > length) { if (owns) free(bytes); return NULL; }
    part = (AgartPart *)calloc(1, sizeof(AgartPart));
    if (!part) { if (owns) free(bytes); return NULL; }
    part->count = count;
    part->blob = bytes;
    part->owns_blob = owns;
    for (i = 0; i < AGART_PALETTE_SIZE; ++i) {
        const uint8_t *c = bytes + 8 + i * 3;
        part->palette_rgb[i][0] = c[0];
        part->palette_rgb[i][1] = c[1];
        part->palette_rgb[i][2] = c[2];
        /* Snap the 8-bit palette to the AGA 12-bit register space, as the
         * screen host writes it (4 bits per channel). */
        part->palette[i] = (uint16_t)(((c[0] >> 4) << 8) | ((c[1] >> 4) << 4) | (c[2] >> 4));
    }
    if (count) {
        part->textures = (AgartTexture *)calloc((size_t)count, sizeof(AgartTexture));
        if (!part->textures) { agart_free(part); return NULL; }
    }
    for (i = 0; i < count; ++i) {
        const uint8_t *e = bytes + AGART_HEADER + (size_t)i * AGART_ENTRY;
        AgartTexture *t = &part->textures[i];
        uint32_t offset = be32(e + 24);
        uint32_t size = be32(e + 28);
        t->name = (const char *)e;         /* NUL-padded in the file */
        t->role = e[16];
        t->flags = e[17];
        t->width = be16(e + 18);
        t->height = be16(e + 20);
        t->stride = be16(e + 22);
        t->pixels = bytes + offset;
        if (!t->name[0] || !t->width || !t->height || offset < data_start ||
            size != (uint32_t)t->stride * t->height ||
            (uint64_t)offset + size > (uint64_t)length) {
            agart_free(part);
            return NULL;
        }
    }
    return part;
}

AgartPart *agart_load(const char *path) {
    FILE *file;
    long length;
    uint8_t *bytes;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return NULL; }
    length = ftell(file);
    if (length < AGART_HEADER) { fclose(file); return NULL; }
    rewind(file);
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes) { fclose(file); return NULL; }
    if (fread(bytes, 1, (size_t)length, file) != (size_t)length) {
        free(bytes); fclose(file); return NULL;
    }
    fclose(file);
    return agart_parse(bytes, (size_t)length, 1);
}

AgartPart *agart_load_memory(uint8_t *blob, size_t size) {
    return agart_parse(blob, size, 1);
}

void agart_free(AgartPart *part) {
    if (!part) return;
    free(part->textures);
    if (part->owns_blob) free(part->blob);
    free(part);
}

const AgartTexture *agart_find(const AgartPart *part, const char *name) {
    int i;
    if (!part) return NULL;
    for (i = 0; i < part->count; ++i)
        if (strcmp(part->textures[i].name, name) == 0) return &part->textures[i];
    return NULL;
}
