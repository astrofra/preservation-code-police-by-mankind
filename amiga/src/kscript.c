/* Code Police — KarateScript parser (C). See kscript.h. */
#include "kscript.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* indexOf(text, len, needle, from): first offset >= from, or -1.
 * Mirrors java.lang.String.indexOf. */
static int ks_index_of(const char *text, int len, const char *needle, int from) {
    int nlen = (int)strlen(needle);
    int i;
    if (nlen == 0) return from <= len ? from : -1;
    for (i = from; i + nlen <= len; ++i) {
        if (memcmp(text + i, needle, (size_t)nlen) == 0) return i;
    }
    return -1;
}

/* kraycasting.FindHowMany — reconstructed Java line 30.
 * Counts one plus the number of occurrences, never below 1. */
int ks_find_how_many(const char *text, int len, const char *needle) {
    int count = 1;
    int found = 0;
    int i = 0;
    while (found != -1 && i < len) {
        found = ks_index_of(text, len, needle, i);
        if (found == -1) continue;
        ++count;
        i = found + 1;
    }
    return count;
}

/* kraycasting.FindInto — reconstructed Java line 44.
 * Returns the substring between the index-th and (index+1)-th separators. */
KsStr ks_find_into(const char *text, int len, int index, const char *sep) {
    int slots = 2;
    int pos = 1;
    int found = 0;
    int i = 0;
    int *array;
    int start, end;
    KsStr result;
    KsStr empty = {NULL, 0};

    while (found != -1 && i < len) {
        found = ks_index_of(text, len, sep, i);
        if (found == -1) continue;
        ++slots;
        i = found + 1;
    }
    array = (int *)malloc((size_t)slots * sizeof(int));
    if (!array) return empty;
    array[0] = -1;
    array[slots - 1] = len;
    i = 0;
    found = 0;
    while (found != -1 && i < len) {
        found = ks_index_of(text, len, sep, i);
        if (found == -1) continue;
        array[pos] = found;
        ++pos;
        i = found + 1;
    }
    if (index < 0 || index + 1 >= slots) { free(array); return empty; }
    start = array[index] + 1;
    end = array[index + 1];
    if (start < 0 || end < start || end > len) { free(array); return empty; }
    result.len = end - start;
    result.data = (char *)malloc((size_t)result.len + 1);
    if (!result.data) { free(array); return empty; }
    memcpy(result.data, text + start, (size_t)result.len);
    result.data[result.len] = '\0';
    free(array);
    return result;
}

/* kraycasting.SplitSTag — reconstructed Java line 71.
 * Splits on <TAG>...</TAG>, returning NULL when the tag is absent. */
KsStr *ks_split_stag(const char *text, int len, const char *tag, int *count) {
    char open_tag[64];
    char close_tag[64];
    int open_len;
    int found = 0;
    int total = 0;
    int i = 0;
    int out = 0;
    KsStr *array;

    if ((int)strlen(tag) + 4 > (int)sizeof open_tag) return NULL;
    open_len = snprintf(open_tag, sizeof open_tag, "<%s>", tag);
    snprintf(close_tag, sizeof close_tag, "</%s>", tag);

    while (found != -1 && i < len) {
        found = ks_index_of(text, len, open_tag, i);
        if (found == -1) continue;
        ++total;
        i = found + open_len;
    }
    if (total == 0) { *count = 0; return NULL; }

    array = (KsStr *)calloc((size_t)total, sizeof(KsStr));
    if (!array) { *count = 0; return NULL; }
    i = 0;
    found = 0;
    while (found != -1 && i < len) {
        int start, end;
        found = ks_index_of(text, len, open_tag, i);
        if (found == -1) continue;
        i = found + open_len;
        end = ks_index_of(text, len, close_tag, i);
        if (end == -1) end = len;
        start = i;
        array[out].len = end - start;
        array[out].data = (char *)malloc((size_t)array[out].len + 1);
        if (!array[out].data) { ks_str_array_free(array, out); *count = 0; return NULL; }
        memcpy(array[out].data, text + start, (size_t)array[out].len);
        array[out].data[array[out].len] = '\0';
        ++out;
        i = end;
    }
    *count = out;
    return array;
}

void ks_str_free(KsStr s) { free(s.data); }

void ks_str_array_free(KsStr *array, int count) {
    int i;
    if (!array) return;
    for (i = 0; i < count; ++i) free(array[i].data);
    free(array);
}

/* The effect name -> keffectcode map from kScript.kScript (engine.cpp). */
static int ks_effect_code(const KsStr *name) {
    static const struct { const char *name; int code; } map[] = {
        {"twirl", 1}, {"sprit", 2}, {"tunnel", 3}, {"ground", 4},
        {"sprit3d", 5}, {"setrect", 6}, {"flare3d", 7}, {"motionblur", 8},
        {"particle", 9}, {"setcamcoord", 10}, {"setcamtarget", 11}, {"evesound", 12},
    };
    char scratch[32];
    int i, len = name->len < 31 ? name->len : 31;
    for (i = 0; i < len; ++i) {
        char c = name->data[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
        scratch[i] = c;
    }
    scratch[len] = '\0';
    for (i = 0; i < (int)(sizeof map / sizeof map[0]); ++i) {
        if (strcmp(scratch, map[i].name) == 0) return map[i].code;
    }
    return 0;
}

/* Parses "<D>a,b</D>" first fields into two integers. */
static void ks_parse_d(const KsStr *part, int *time_length, int *time_start) {
    int dummy;
    KsStr d;
    KsStr *items;
    *time_length = 0;
    *time_start = 0;
    items = ks_split_stag(part->data, part->len, "D", &dummy);
    if (!items || dummy < 1) { ks_str_array_free(items, dummy); return; }
    /* FindInto(items[0], 0, ",") and FindInto(items[0], 1, ","). */
    d = ks_find_into(items[0].data, items[0].len, 0, ",");
    if (d.data) { *time_length = atoi(d.data); ks_str_free(d); }
    d = ks_find_into(items[0].data, items[0].len, 1, ",");
    if (d.data) { *time_start = atoi(d.data); ks_str_free(d); }
    ks_str_array_free(items, dummy);
}

KsScript *ks_parse(const char *text, int len) {
    KsScript *script;
    KsStr *kimages, *ksounds, *krect, *ktable, *kscript, *ktice, *kcam;
    KsStr *kparts;
    int n_images = 0, n_sounds = 0, n_rect = 0, n_table = 0, n_script = 0,
        n_tice = 0, n_cam = 0, n_parts = 0;
    int i;

    script = (KsScript *)calloc(1, sizeof(KsScript));
    if (!script) return NULL;

    kimages = ks_split_stag(text, len, "KIMAGES", &n_images);
    ksounds = ks_split_stag(text, len, "KSOUNDS", &n_sounds);
    krect = ks_split_stag(text, len, "KRECT", &n_rect);
    ktable = ks_split_stag(text, len, "KTABLE", &n_table);
    kscript = ks_split_stag(text, len, "KSCRIPT", &n_script);
    ktice = ks_split_stag(text, len, "KTICLE", &n_tice);
    kcam = ks_split_stag(text, len, "KCAM", &n_cam);

    script->image_count = n_images;
    script->sound_count = n_sounds;
    script->rect_count = n_rect;
    script->table_count = n_table;
    script->ticle_count = n_tice;
    script->camera_count = n_cam;

    if (n_script >= 1) {
        KsStr *kend;
        int n_kend = 0;
        kparts = ks_split_stag(kscript[0].data, kscript[0].len, "KPART", &n_parts);
        script->part_count = n_parts;
        script->parts = (KsPart *)calloc((size_t)n_parts + 1, sizeof(KsPart));
        for (i = 0; i < n_parts && script->parts; ++i) {
            KsStr *fx;
            KsStr *sounds;
            int n_fx = 0, n_snd = 0, j;
            ks_parse_d(&kparts[i], &script->parts[i].time_length,
                       &script->parts[i].time_start);
            fx = ks_split_stag(kparts[i].data, kparts[i].len, "Fx", &n_fx);
            script->parts[i].effect_count = n_fx;
            for (j = 0; j < n_fx && j < 64; ++j) {
                KsStr *params;
                int n_params = 0;
                params = ks_split_stag(fx[j].data, fx[j].len, "Pa", &n_params);
                if (n_params >= 1) {
                    KsStr name = ks_find_into(params[0].data, params[0].len, 0, "|");
                    if (name.data) {
                        script->parts[i].effect_codes[j] = ks_effect_code(&name);
                        ks_str_free(name);
                    }
                }
                ks_str_array_free(params, n_params);
            }
            ks_str_array_free(fx, n_fx);
            sounds = ks_split_stag(kparts[i].data, kparts[i].len, "S", &n_snd);
            script->parts[i].sound_count = n_snd;
            ks_str_array_free(sounds, n_snd);
        }
        ks_str_array_free(kparts, n_parts);
        kend = ks_split_stag(kscript[0].data, kscript[0].len, "KEND", &n_kend);
        if (n_kend >= 1) {
            /* The engine trims, lower-cases and compares to "loop". */
            int a = 0, b = kend[0].len;
            while (a < b && (kend[0].data[a] == ' ' || kend[0].data[a] == '\t' ||
                             kend[0].data[a] == '\r' || kend[0].data[a] == '\n')) ++a;
            while (b > a && (kend[0].data[b - 1] == ' ' || kend[0].data[b - 1] == '\t' ||
                             kend[0].data[b - 1] == '\r' || kend[0].data[b - 1] == '\n')) --b;
            if (b - a >= 4) {
                char w[5];
                int k;
                for (k = 0; k < 4; ++k) {
                    char c = kend[0].data[a + k];
                    if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
                    w[k] = c;
                }
                w[4] = '\0';
                if (strcmp(w, "loop") == 0) script->end_pref = 1;
            }
        }
        ks_str_array_free(kend, n_kend);
    }

    ks_str_array_free(kimages, n_images);
    ks_str_array_free(ksounds, n_sounds);
    ks_str_array_free(krect, n_rect);
    ks_str_array_free(ktable, n_table);
    ks_str_array_free(kscript, n_script);
    ks_str_array_free(ktice, n_tice);
    ks_str_array_free(kcam, n_cam);
    return script;
}

void ks_free(KsScript *script) {
    if (!script) return;
    free(script->parts);
    free(script);
}
