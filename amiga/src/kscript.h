/* Code Police — KarateScript parser (C), milestone 2.
 *
 * A direct C port of the tokeniser in native-sdl2/src/engine.cpp:
 * FindHowMany, FindInto and SplitSTag, which the whole script grammar is built
 * on. These are byte-exact ports, including the off-by-one behaviour of the
 * originals, so the C parser and the verified C++ engine agree on every field.
 */
#ifndef KSCRIPT_H
#define KSCRIPT_H

#include <stddef.h>

/* A borrowed or owned byte range. Owned ranges are freed with kscript_str_free. */
typedef struct KsStr {
    char *data;
    int len;
} KsStr;

/* The parsed script skeleton: counts and part timings, enough to validate the
 * tokeniser against evidence/diagnostic.json before the object graph exists. */
typedef struct KsPart {
    int time_length;   /* <D> first field, the part's end time in ticks */
    int time_start;    /* <D> second field, the part's start time in ticks */
    int sound_count;   /* number of <S> entries */
    int effect_count;  /* number of <Fx> entries */
    int effect_codes[64]; /* keffectcode per effect, clamped to 64 */
} KsPart;

typedef struct KsScript {
    int image_count;
    int sound_count;
    int table_count;
    int ticle_count;
    int camera_count;
    int rect_count;
    int part_count;      /* real parts, excluding the trailing sentinel */
    int end_pref;        /* 1 when <KEND> is "loop" */
    KsPart *parts;

    /* Retained raw declaration strings, so objects.c can build the graph. */
    KsStr *decl_images;
    KsStr *decl_sounds;
    KsStr *decl_tables;
    KsStr *decl_tice;
    KsStr *decl_cams;
    KsStr *decl_rects;
    KsStr *decl_parts;
} KsScript;

/* Primitives, ported verbatim from kraycasting. */
int ks_find_how_many(const char *text, int len, const char *needle);
KsStr ks_find_into(const char *text, int len, int index, const char *sep);
KsStr *ks_split_stag(const char *text, int len, const char *tag, int *count);

/* Frees a string returned by ks_find_into and releases the array. */
void ks_str_free(KsStr s);
/* Frees an array returned by ks_split_stag. */
void ks_str_array_free(KsStr *array, int count);

/* Parses the whole script text. Returns NULL on allocation failure. */
KsScript *ks_parse(const char *text, int len);
void ks_free(KsScript *script);

/* Trims leading/trailing whitespace in place, returning a new view. */
KsStr ks_trim(const KsStr *s);

#endif /* KSCRIPT_H */
