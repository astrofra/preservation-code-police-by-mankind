/* Code Police — Amiga platform boundary (C).
 *
 * Mirrors native-sdl2/src/platform.hpp, minus std::mutex and the C++ types.
 * The engine calls through this boundary; only the host implementation touches
 * AmigaOS. A headless implementation (the one here) uses stdio, so the same
 * code runs under vamos and under FS-UAE.
 */
#ifndef AMIGA_PLATFORM_H
#define AMIGA_PLATFORM_H

#include <stddef.h>
#include <stdint.h>

/* An image as the engine sees it: 32-bit 0xAARRGGBB pixels, row-major. */
typedef struct AmigaImage {
    int32_t width, height;
    uint32_t *pixels;
} AmigaImage;

/* The host boundary. */
typedef struct AmigaPlatform {
    void *user;
    /* Reads a whole file into a freshly allocated buffer. NULL on failure. */
    uint8_t *(*read_file)(void *user, const char *path, size_t *size);
    void (*free_file)(void *user, uint8_t *data);
    /* Loads and decodes an image relative to the asset root. */
    AmigaImage *(*load_image)(void *user, const char *relative);
    void (*free_image)(void *user, AmigaImage *image);
    /* Milliseconds since an arbitrary but fixed epoch. */
    uint32_t (*time_ms)(void *user);
} AmigaPlatform;

/* The headless file-backed host: assets live under one directory. */
typedef struct AmigaFileHost {
    char directory[256];
    uint32_t clock_offset_ms;
} AmigaFileHost;

void amiga_file_host_init(AmigaFileHost *host, const char *directory);
AmigaPlatform amiga_file_host_platform(AmigaFileHost *host);

/* Decodes PNG bytes (as produced by the C++ tooling) into an ARGB image. */
AmigaImage *amiga_decode_image(const uint8_t *bytes, size_t size);

/* Allocates a zeroed image of the given size. */
AmigaImage *amiga_alloc_image(int32_t width, int32_t height);
void amiga_free_image(AmigaImage *image);

/* FNV-1a over the pixels, matching verify.cpp's frame hash. */
uint64_t amiga_hash_image(const AmigaImage *image);

#endif /* AMIGA_PLATFORM_H */
