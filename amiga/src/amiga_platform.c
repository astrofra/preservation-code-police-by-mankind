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

static AmigaImage *host_load_image(void *user, const char *relative) {
    size_t size = 0;
    uint8_t *bytes = file_read(user, relative, &size);
    AmigaImage *image;
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
