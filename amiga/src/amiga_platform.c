/* Code Police — Amiga platform boundary (C). */
#include "amiga_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_STDIO
#include "stb_image.h"

static uint8_t *file_read(void *user, const char *path, size_t *size) {
    AmigaFileHost *host = (AmigaFileHost *)user;
    char full[512];
    FILE *file;
    long length;
    uint8_t *data;
    if (strlen(host->directory) + strlen(path) + 2 > sizeof full) return NULL;
    snprintf(full, sizeof full, "%s/%s", host->directory, path);
    file = fopen(full, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0) { fclose(file); return NULL; }
    length = ftell(file);
    if (length < 0) { fclose(file); return NULL; }
    rewind(file);
    data = (uint8_t *)malloc((size_t)length + 1);
    if (!data) { fclose(file); return NULL; }
    if (fread(data, 1, (size_t)length, file) != (size_t)length) {
        free(data); fclose(file); return NULL;
    }
    fclose(file);
    if (size) *size = (size_t)length;
    return data;
}

static void file_free(void *user, uint8_t *data) {
    (void)user;
    free(data);
}

static uint32_t host_time_ms(void *user) {
    AmigaFileHost *host = (AmigaFileHost *)user;
    return host->clock_offset_ms;
}

AmigaImage *amiga_alloc_image(int32_t width, int32_t height) {
    AmigaImage *image = (AmigaImage *)malloc(sizeof(AmigaImage));
    if (!image) return NULL;
    image->width = width;
    image->height = height;
    image->pixels = (uint32_t *)calloc((size_t)width * (size_t)height, sizeof(uint32_t));
    if (!image->pixels) { free(image); return NULL; }
    return image;
}

void amiga_free_image(AmigaImage *image) {
    if (!image) return;
    free(image->pixels);
    free(image);
}

/* The engine's images are RGBA PNGs; stb gives us 4 bytes per pixel. The
 * engine stores 0xFF000000 | (r<<16) | (g<<8) | b, matching assets.cpp. */
AmigaImage *amiga_decode_image(const uint8_t *bytes, size_t size) {
    int width = 0, height = 0, channels = 0;
    AmigaImage *image;
    stbi_uc *rgba;
    int i, count;
    if (size > (size_t)0x7fffffff) return NULL;
    rgba = stbi_load_from_memory(bytes, (int)size, &width, &height, &channels, 4);
    if (!rgba) return NULL;
    image = amiga_alloc_image(width, height);
    if (!image) { stbi_image_free(rgba); return NULL; }
    count = width * height;
    for (i = 0; i < count; ++i) {
        image->pixels[i] = ((uint32_t)rgba[i * 4 + 3] << 24) |
                           ((uint32_t)rgba[i * 4 + 0] << 16) |
                           ((uint32_t)rgba[i * 4 + 1] << 8) |
                           ((uint32_t)rgba[i * 4 + 2]);
    }
    stbi_image_free(rgba);
    return image;
}

static uint16_t cpil_u16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t cpil_u32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

/* CPIL: version, flags (bit 0 = LZ4 block), big-endian dimensions and sizes,
 * then ARGB32 bytes. The target does not need PNG/zlib; the tiny LZ4 decoder
 * expands directly into one temporary image buffer. */
static int lz4_decode(const uint8_t *src, size_t src_size, uint8_t *dst,
                      size_t dst_size) {
    const uint8_t *ip = src, *end = src + src_size;
    uint8_t *op = dst, *out_end = dst + dst_size;
    while (ip < end) {
        unsigned token = *ip++;
        size_t literal = token >> 4;
        size_t match;
        size_t offset;
        uint8_t *match_ptr;
        if (literal == 15) {
            unsigned n;
            do { if (ip >= end) return 0; n = *ip++; literal += n; } while (n == 255);
        }
        if (literal > (size_t)(end - ip) || literal > (size_t)(out_end - op)) return 0;
        memcpy(op, ip, literal); ip += literal; op += literal;
        if (ip == end) break;
        if (end - ip < 2) return 0;
        offset = (size_t)ip[0] | ((size_t)ip[1] << 8); ip += 2;
        if (!offset || offset > (size_t)(op - dst)) return 0;
        match = (token & 15) + 4;
        if ((token & 15) == 15) {
            unsigned n;
            do { if (ip >= end) return 0; n = *ip++; match += n; } while (n == 255);
        }
        if (match > (size_t)(out_end - op)) return 0;
        match_ptr = op - offset;
        while (match--) *op++ = *match_ptr++;
    }
    return op == out_end;
}

static AmigaImage *amiga_decode_cpil(const uint8_t *bytes, size_t size) {
    uint16_t width, height;
    uint8_t flags;
    uint32_t raw_size, payload_size;
    size_t count, i;
    AmigaImage *image;
    uint8_t *raw;
    if (size < 18 || memcmp(bytes, "CPIL", 4) != 0 || bytes[4] != 1) return NULL;
    flags = bytes[5];
    width = cpil_u16(bytes + 6);
    height = cpil_u16(bytes + 8);
    raw_size = ((uint32_t)bytes[10] << 24) | ((uint32_t)bytes[11] << 16) |
               ((uint32_t)bytes[12] << 8) | bytes[13];
    payload_size = ((uint32_t)bytes[14] << 24) | ((uint32_t)bytes[15] << 16) |
                   ((uint32_t)bytes[16] << 8) | bytes[17];
    count = (size_t)width * (size_t)height;
    if (!width || !height || raw_size != count * 4 || payload_size != size - 18)
        return NULL;
    raw = (uint8_t *)malloc(raw_size);
    if (!raw) return NULL;
    if (flags & 1) {
        if (!lz4_decode(bytes + 18, payload_size, raw, raw_size)) { free(raw); return NULL; }
    } else {
        if (payload_size != raw_size) { free(raw); return NULL; }
        memcpy(raw, bytes + 18, raw_size);
    }
    image = amiga_alloc_image(width, height);
    if (!image) { free(raw); return NULL; }
    for (i = 0; i < count; ++i) image->pixels[i] = cpil_u32(raw + i * 4);
    free(raw);
    return image;
}

static AmigaImage *host_load_image(void *user, const char *relative) {
    char packed[512];
    size_t size = 0;
    size_t length = strlen(relative);
    uint8_t *bytes;
    AmigaImage *image;
    if (length >= 4 && length + 2 < sizeof packed &&
        !strcmp(relative + length - 4, ".png")) {
        memcpy(packed, relative, length - 4);
        memcpy(packed + length - 4, ".cpil", 6);
        bytes = file_read(user, packed, &size);
        if (bytes) {
            image = amiga_decode_cpil(bytes, size);
            free(bytes);
            if (image) return image;
        }
    }
    bytes = file_read(user, relative, &size);
    if (!bytes) return NULL;
    image = amiga_decode_image(bytes, size);
    free(bytes);
    return image;
}

static void host_free_image(void *user, AmigaImage *image) {
    (void)user;
    amiga_free_image(image);
}

void amiga_file_host_init(AmigaFileHost *host, const char *directory) {
    strncpy(host->directory, directory, sizeof host->directory - 1);
    host->directory[sizeof host->directory - 1] = '\0';
    host->clock_offset_ms = 0;
}

AmigaPlatform amiga_file_host_platform(AmigaFileHost *host) {
    AmigaPlatform platform;
    platform.user = host;
    platform.read_file = file_read;
    platform.free_file = file_free;
    platform.load_image = host_load_image;
    platform.free_image = host_free_image;
    platform.time_ms = host_time_ms;
    return platform;
}

uint64_t amiga_hash_image(const AmigaImage *image) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    int32_t i, count = image->width * image->height;
    for (i = 0; i < count; ++i) {
        hash ^= (uint64_t)(image->pixels[i] | 0xff000000u);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}
