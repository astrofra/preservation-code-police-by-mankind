/* Code Police — renderer core (C). See renderer.h. */
#include "renderer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static double dabs(double x) { return x < 0.0 ? -x : x; }

static kpixel **alloc_pretable(jint max_pre_x, jint max_pre_y) {
    jint n, i, count = jmul(max_pre_x, max_pre_y);
    kpixel **table;
    if (count <= 0) return NULL;
    table = (kpixel **)malloc((size_t)count * sizeof(kpixel *));
    if (!table) return NULL;
    for (i = 0; i < count; ++i) {
        table[i] = (kpixel *)calloc(1, sizeof(kpixel));
        if (!table[i]) {
            jint k;
            for (k = 0; k < i; ++k) free(table[k]);
            free(table);
            return NULL;
        }
    }
    (void)n;
    return table;
}

static void free_pretable(rcontext *rc) {
    jint i, count;
    if (!rc->Pretable) return;
    count = jmul(rc->PretableX, rc->PretableY);
    for (i = 0; i < count; ++i) free(rc->Pretable[i]);
    free(rc->Pretable);
    rc->Pretable = NULL;
}

void rcr_init_root(rcontext *rc, jint width, jint height) {
    jint n = 0, n2, n3, n4, n5, n6, n7, n8, n9, n10;
    double d, d2;
    memset(rc, 0, sizeof *rc);
    rc->ClipX1 = 0;
    rc->ClipY1 = 0;
    rc->ClipX2 = width;
    rc->ClipY2 = height;
    n4 = jsub(rc->ClipX2, rc->ClipX1);
    n5 = jsub(rc->ClipY2, rc->ClipY1);
    rc->drawable = 1;
    d = 1.0 / (double)n4;
    d2 = 1.0 / (double)n5;
    rc->Modulo = rc->ClipX2;
    rc->Height = rc->ClipY2;
    rc->Chunk2 = (int32_t *)calloc((size_t)jmul(rc->Modulo, rc->Height), sizeof(int32_t));
    rc->CurrentChunk = (int32_t *)calloc((size_t)jmul(rc->Modulo, rc->Height), sizeof(int32_t));
    rc->RC_RootContext = rc;
    rc->MaxX = rc->Modulo;
    rc->MaxY = rc->Height;
    rc->MaxPreX = jadd(jshr(rc->MaxX, 4), 2);
    rc->MaxPreY = jadd(jshr(rc->MaxY, 4), 2);
    rc->PretableX = rc->MaxPreX;
    rc->PretableY = rc->MaxPreY;
    rc->Pretable = alloc_pretable(rc->MaxPreX, rc->MaxPreY);
    n6 = jshr(n4, 1);
    n7 = jshr(n5, 1);
    n2 = 16;
    n8 = 0;
    n9 = 0;
    while (n9 < jadd(n5, n2)) {
        if (jadd(n9, 16) > n5) n2 = jsub(n5, n9);
        n3 = 16;
        n = 0;
        n10 = 0;
        while (n10 < jadd(n4, n3)) {
            if (jadd(n10, 16) > n4) n3 = jsub(n4, n10);
            rc->Pretable[n + jmul(n8, rc->PretableX)]->XI = (double)jsub(n10, n6) * d;
            rc->Pretable[n + jmul(n8, rc->PretableX)]->YI = (double)jsub(n9, n7) * d2;
            ++n;
            n10 = jadd(n10, n3);
        }
        ++n8;
        n9 = jadd(n9, n2);
    }
    rc->MaxPreX = n;
    rc->MaxPreY = n8;
}

void rcr_init_child(rcontext *rc, rcontext *parent) {
    memset(rc, 0, sizeof *rc);
    rc->ClipX1 = parent->ClipX1;
    rc->ClipY1 = parent->ClipY1;
    rc->ClipX2 = parent->ClipX2;
    rc->ClipY2 = parent->ClipY2;
    rc->Modulo = parent->Modulo;
    rc->Height = parent->Height;
    rc->RC_RootContext = parent;
    rc->RC_FatherContext = parent;
    rc->drawable = parent->drawable;
}

void rcr_init_rect(rcontext *rc, rcontext *parent, double d, double d2, double d3, double d4) {
    memset(rc, 0, sizeof *rc);
    rc->RC_RootContext = parent->RC_RootContext;
    rc->PretableX = parent->RC_RootContext->PretableX;
    rc->PretableY = parent->RC_RootContext->PretableY;
    /* The C++ constructor allocates the grid before SetRect indexes it. */
    rc->Pretable = alloc_pretable(rc->PretableX, rc->PretableY);
    rcr_setrect(rc, parent, d, d2, d3, d4);
}

void rcr_setrect(rcontext *rc, rcontext *parent, double d, double d2, double d3, double d4) {
    jint n = 0, n2 = 0, n3 = 0, n4, n5, n6, n7, n8, n9, n10;
    double d5 = (double)jsub(parent->ClipX2, parent->ClipX1);
    double d6 = (double)jsub(parent->ClipY2, parent->ClipY1);
    rc->drawable = 1;
    if (d < 0.0) d = 0.0;
    if (d2 < 0.0) d2 = 0.0;
    if (d3 > 1.0) d3 = 1.0;
    if (d4 > 1.0) d4 = 1.0;
    rc->ClipX1 = jadd(parent->ClipX1, jint_of(d5 * d));
    rc->ClipY1 = jadd(parent->ClipY1, jint_of(d6 * d2));
    rc->ClipX2 = jadd(parent->ClipX1, jint_of(d5 * d3));
    rc->ClipY2 = jadd(parent->ClipY1, jint_of(d6 * d4));
    rc->Modulo = parent->Modulo;
    rc->Height = parent->Height;
    n4 = jsub(rc->ClipX2, rc->ClipX1);
    n5 = jsub(rc->ClipY2, rc->ClipY1);
    {
        double d7 = 1.0 / (double)n4;
        double d8 = 1.0 / (double)n5;
        if (n4 <= 0 || n5 <= 0 ||
            rc->ClipX1 < parent->ClipX1 || rc->ClipY1 < parent->ClipY1 ||
            rc->ClipX2 > parent->ClipX2 || rc->ClipY2 > parent->ClipY2) {
            rc->drawable = 0;
            return;
        }
        rc->MaxX = n4;
        rc->MaxY = n5;
        n6 = jshr(n4, 1);
        n7 = jshr(n5, 1);
        n2 = 16;
        n8 = 0;
        n9 = 0;
        while (n9 < jadd(n5, n2)) {
            if (jadd(n9, 16) > n5) n2 = jsub(n5, n9);
            n3 = 16;
            n = 0;
            n10 = 0;
            while (n10 < jadd(n4, n3)) {
                if (jadd(n10, 16) > n4) n3 = jsub(n4, n10);
                rc->Pretable[n + jmul(n8, rc->PretableX)]->XI = (double)jsub(n10, n6) * d7;
                rc->Pretable[n + jmul(n8, rc->PretableX)]->YI = (double)jsub(n9, n7) * d8;
                ++n;
                n10 = jadd(n10, n3);
            }
            ++n8;
            n9 = jadd(n9, n2);
        }
        rc->MaxPreX = n;
        rc->MaxPreY = n8;
    }
}

void rcr_setsub(rcontext *rc, rcontext *parent, jint n, jint n2, jint n3, jint n4) {
    rc->ClipX1 = n;
    rc->ClipY1 = n2;
    rc->ClipX2 = n3;
    rc->ClipY2 = n4;
    rc->CurrentChunk = parent->CurrentChunk;
    rc->RC_RootContext = parent->RC_RootContext;
}

int32_t *rcr_create_image(rcontext *rc) {
    int32_t *nArray = rc->CurrentChunk;
    rc->CurrentChunk = rc->Chunk2;
    rc->Chunk2 = nArray;
    return rc->Chunk2;
}

void rcr_free(rcontext *rc) {
    free_pretable(rc);
    free(rc->Chunk2);
    free(rc->CurrentChunk);
    rc->Chunk2 = NULL;
    rc->CurrentChunk = NULL;
}

void rcs_init(rcaster *rcs, rcontext *root) {
    rcs->Rootrc = root;
    rcs->OneRec = (rcontext *)malloc(sizeof(rcontext));
    if (rcs->OneRec) rcr_init_child(rcs->OneRec, root);
}

/* ---- rasterisation primitives ---- */
static void drawrec_uvs(rcontext *rc, texture *tx, kpixel *p2, kpixel *p3, kpixel *p4, kpixel *p5);
static void drawrec_uvsa(rcontext *rc, texture *tx, kpixel *p2, kpixel *p3, kpixel *p4, kpixel *p5, jint n);
static void drawrec_uvm(rcontext *rc, texture *tx, kpixel *p2, kpixel *p3, kpixel *p4, kpixel *p5);
static void drawrec_uvrgb(rcontext *rc, texture *tx, kpixel *p2, kpixel *p3, kpixel *p4, kpixel *p5);

/* raycasting.draw — rasterise a texture over the context's UV grid. */
void rcs_draw(rcaster *rcs, texture *tx, rcontext *rc) {
    jint n = jsub(rc->ClipX2, rc->ClipX1);
    jint n2 = jsub(rc->ClipY2, rc->ClipY1);
    jint n5 = 0;
    if (!rc->drawable) return;
    while (n5 < n2) {
        jint n3 = (jadd(n5, 16) < n2) ? 16 : jsub(n2, n5);
        jint n6 = 0;
        while (n6 < n) {
            jint n4 = (jadd(n6, 16) < n) ? 16 : jsub(n, n6);
            jint bx = jshr(n6, 4), by = jshr(n5, 4);
            kpixel *p00 = rc->Pretable[bx + jmul(by, rc->PretableX)];
            kpixel *p10 = rc->Pretable[jadd(bx, 1) + jmul(by, rc->PretableX)];
            kpixel *p01 = rc->Pretable[bx + jmul(jadd(by, 1), rc->PretableX)];
            kpixel *p11 = rc->Pretable[jadd(bx, 1) + jmul(jadd(by, 1), rc->PretableX)];
            rcr_setsub(rcs->OneRec, rc, jadd(n6, rc->ClipX1), jadd(n5, rc->ClipY1),
                       jadd(jadd(n6, n4), rc->ClipX1), jadd(jadd(n5, n3), rc->ClipY1));
            drawrec_uvrgb(rcs->OneRec, tx, p00, p10, p01, p11);
            n6 = jadd(n6, 16);
        }
        n5 = jadd(n5, 16);
    }
}

/* raycasting.MotionBlur */
void rcs_motion_blur(rcaster *rcs, rcontext *rc) {
    jint n = jsub(rc->ClipX2, rc->ClipX1);
    jint n2 = jsub(rc->ClipY2, rc->ClipY1);
    jint n3 = rc->Modulo;
    jint n4 = jadd(rc->ClipX1, jmul(rc->ClipY1, rc->Modulo));
    int32_t *cur = rc->RC_RootContext->CurrentChunk;
    int32_t *prev = rc->RC_RootContext->Chunk2;
    jint n5, n6, n7, n8, n9;
    (void)rcs;
    if (n == 0 || n2 == 0) return;
    n5 = 0;
    while (n5 < n2) {
        n6 = n4;
        n7 = 0;
        while (n7 < n) {
            n8 = (jint)((uint32_t)prev[n6] & 0xFCFCFFu);
            n9 = (jint)((uint32_t)cur[n6] & 0xFCFCFFu);
            cur[n6] = (int32_t)(0xFF000000u | ((uint32_t)jshr(jadd(n8, n9), 1)));
            ++n6;
            ++n7;
        }
        n4 = jadd(n4, n3);
        ++n5;
    }
}

/* raycasting.drawrecUV — opaque texel copy, index -1 transparent. */
static void drawrec_uv(rcontext *rc, texture *tx, kpixel *p2, kpixel *p3, kpixel *p4, kpixel *p5) {
    jint n = jsub(rc->ClipX2, rc->ClipX1);
    jint n2 = jsub(rc->ClipY2, rc->ClipY1);
    jint n3 = rc->Modulo;
    jint n4 = jadd(rc->ClipX1, jmul(rc->ClipY1, rc->Modulo));
    int32_t *dst = rc->RC_RootContext->CurrentChunk;
    int32_t *tex = tx->pixels;
    jint n5 = p2->u, n6 = p2->v;
    jint n7 = jdiv(jsub(p4->u, n5), n2);
    jint n8 = jdiv(jsub(p4->v, n6), n2);
    jint n9 = p3->u, n10 = p3->v;
    jint n11 = jdiv(jsub(p5->u, n9), n2);
    jint n12 = jdiv(jsub(p5->v, n10), n2);
    jint n13;
    if (n == 0 || n2 == 0) return;
    for (n13 = 0; n13 < n2; ++n13) {
        jint n14 = n4, n15 = n5, n16 = n6;
        jint n17 = jdiv(jsub(n9, n15), n);
        jint n18 = jdiv(jsub(n10, n16), n);
        jint n19;
        for (n19 = 0; n19 < n; ++n19) {
            dst[n14++] = tex[jabs_or(jand(jshr(n16, 8), 0xFF00), jand(jshr(n15, 16), 0xFF))];
            n15 = jadd(n15, n17);
            n16 = jadd(n16, n18);
        }
        n5 = jadd(n5, n7); n6 = jadd(n6, n8);
        n9 = jadd(n9, n11); n10 = jadd(n10, n12);
        n4 = jadd(n4, n3);
    }
}

/* raycasting.drawrecUVS — opaque copy, index -1 transparent. */
static void drawrec_uvs(rcontext *rc, texture *tx, kpixel *p2, kpixel *p3, kpixel *p4, kpixel *p5) {
    jint n = jsub(rc->ClipX2, rc->ClipX1);
    jint n2 = jsub(rc->ClipY2, rc->ClipY1);
    jint n3 = rc->Modulo;
    jint n4 = jadd(rc->ClipX1, jmul(rc->ClipY1, rc->Modulo));
    int32_t *dst = rc->RC_RootContext->CurrentChunk;
    int32_t *tex = tx->pixels;
    jint n5 = tx->Modulo;
    jint n6, n7, n8, n9, n10, n11, n12, n13, n14;
    if (n == 0 || n2 == 0) return;
    n6 = p2->u; n7 = p2->v;
    n8 = jdiv(jsub(p4->u, n6), n2);
    n9 = jdiv(jsub(p4->v, n7), n2);
    n10 = p3->u; n11 = p3->v;
    n12 = jdiv(jsub(p5->u, n10), n2);
    n13 = jdiv(jsub(p5->v, n11), n2);
    for (n14 = 0; n14 < n2; ++n14) {
        jint n15 = n4, n16 = n6, n17 = n7;
        jint n18 = jdiv(jsub(n10, n16), n);
        jint n19 = jdiv(jsub(n11, n17), n);
        jint n20;
        for (n20 = 0; n20 < n; ++n20) {
            jint n21 = tex[jadd(jmul(jshr(n17, 16), n5), jshr(n16, 16))];
            if (n21 != -1) dst[n15] = n21;
            ++n15;
            n16 = jadd(n16, n18);
            n17 = jadd(n17, n19);
        }
        n6 = jadd(n6, n8); n7 = jadd(n7, n9);
        n10 = jadd(n10, n12); n11 = jadd(n11, n13);
        n4 = jadd(n4, n3);
    }
}

/* raycasting.drawrecUVM — additive, clamped to 255. */
static void drawrec_uvm(rcontext *rc, texture *tx, kpixel *p2, kpixel *p3, kpixel *p4, kpixel *p5) {
    jint n = jsub(rc->ClipX2, rc->ClipX1);
    jint n2 = jsub(rc->ClipY2, rc->ClipY1);
    jint n3 = rc->Modulo;
    jint n4 = jadd(rc->ClipX1, jmul(rc->ClipY1, rc->Modulo));
    int32_t *dst = rc->RC_RootContext->CurrentChunk;
    int32_t *tex = tx->pixels;
    int b_first = 0;
    jint n5 = 255;
    jint n6 = tx->Modulo;
    jint n7, n8, n9, n10, n11, n12, n13, n14;
    (void)b_first;
    if (n == 0 || n2 == 0) return;
    n7 = p2->u; n8 = p2->v;
    n9 = jdiv(jsub(p4->u, n7), n2);
    n10 = jdiv(jsub(p4->v, n8), n2);
    n11 = p3->u; n12 = p3->v;
    n13 = jdiv(jsub(p5->u, n11), n2);
    n14 = jdiv(jsub(p5->v, n12), n2);
    {
        jint n15;
        for (n15 = 0; n15 < n2; ++n15) {
            jint n16 = n4, n17 = n7, n18 = n8;
            jint n19 = jdiv(jsub(n11, n17), n);
            jint n20 = jdiv(jsub(n12, n18), n);
            jint n21;
            for (n21 = 0; n21 < n; ++n21) {
                jint n22 = tex[jadd(jmul(jshr(n18, 16), n6), jshr(n17, 16))];
                jint n23 = dst[n16];
                jint n24 = jadd(jshr(jand(n22, 0xFF0000), 16), jshr(jand(n23, 0xFF0000), 16));
                jint n25 = jadd(jshr(jand(n22, 0xFF00), 8), jshr(jand(n23, 0xFF00), 8));
                n22 = jadd(jand(n22, 0xFF), jand(n23, 0xFF));
                if (n24 > n5) n24 = n5;
                if (n25 > n5) n25 = n5;
                if (n22 > n5) n22 = n5;
                dst[n16++] = (int32_t)(0xFF000000u | ((uint32_t)jshl(n24, 16)) |
                                       ((uint32_t)jshl(n25, 8)) | (uint32_t)n22);
                n17 = jadd(n17, n19);
                n18 = jadd(n18, n20);
            }
            n7 = jadd(n7, n9); n8 = jadd(n8, n10);
            n11 = jadd(n11, n13); n12 = jadd(n12, n14);
            n4 = jadd(n4, n3);
        }
    }
}

/* raycasting.drawrecUVRGB — additive with per-pixel RGB offsets, clamp 0..255. */
static void drawrec_uvrgb(rcontext *rc, texture *tx, kpixel *p2, kpixel *p3, kpixel *p4, kpixel *p5) {
    jint n = 0;
    jint n2 = 255;
    jint n3 = jsub(rc->ClipX2, rc->ClipX1);
    jint n4 = jsub(rc->ClipY2, rc->ClipY1);
    jint n5 = rc->Modulo;
    jint n6 = jadd(rc->ClipX1, jmul(rc->ClipY1, rc->Modulo));
    int32_t *dst = rc->RC_RootContext->CurrentChunk;
    int32_t *tex = tx->pixels;
    jint n7 = p2->u, n8 = p2->v, n9 = p2->r, n10 = p2->g, n11 = p2->b;
    jint n12 = jdiv(jsub(p4->u, n7), n4);
    jint n13 = jdiv(jsub(p4->v, n8), n4);
    jint n14 = jdiv(jsub(p4->r, n9), n4);
    jint n15 = jdiv(jsub(p4->g, n10), n4);
    jint n16 = jdiv(jsub(p4->b, n11), n4);
    jint n17 = p3->u, n18 = p3->v, n19 = p3->r, n20 = p3->g, n21 = p3->b;
    jint n22 = jdiv(jsub(p5->u, n17), n4);
    jint n23 = jdiv(jsub(p5->v, n18), n4);
    jint n24 = jdiv(jsub(p5->r, n19), n4);
    jint n25 = jdiv(jsub(p5->g, n20), n4);
    jint n26 = jdiv(jsub(p5->b, n21), n4);
    jint n27;
    for (n27 = 0; n27 < n4; ++n27) {
        jint n28 = n6, n29 = n7, n30 = n8, n31 = n9, n32 = n10, n33 = n11;
        jint n34 = jdiv(jsub(n17, n29), n3);
        jint n35 = jdiv(jsub(n18, n30), n3);
        jint n36 = jdiv(jsub(n19, n31), n3);
        jint n37 = jdiv(jsub(n20, n32), n3);
        jint n38 = jdiv(jsub(n21, n33), n3);
        jint n39;
        for (n39 = 0; n39 < n3; ++n39) {
            jint n40 = tex[jabs_or(jand(jshr(n30, 8), 0xFF00), jand(jshr(n29, 16), 0xFF))];
            jint n41 = jshr(jand(n31, (jint)0xFFFF0000), 16);
            jint n42 = jshr(jand(n32, (jint)0xFFFF0000), 16);
            jint n43 = jshr(jand(n33, (jint)0xFFFF0000), 16);
            jint n44 = jadd(jshr(jand(n40, 0xFF0000), 16), n41);
            jint n45 = jadd(jshr(jand(n40, 0xFF00), 8), n42);
            n40 = jadd(jand(n40, 0xFF), n43);
            if (n44 > n2) n44 = n2; else if (n44 < n) n44 = n;
            if (n45 > n2) n45 = n2; else if (n45 < n) n45 = n;
            if (n40 > n2) n40 = n2; else if (n40 < n) n40 = n;
            dst[n28++] = (int32_t)(0xFF000000u | ((uint32_t)jshl(n44, 16)) |
                                   ((uint32_t)jshl(n45, 8)) | (uint32_t)n40);
            n29 = jadd(n29, n34); n30 = jadd(n30, n35);
            n31 = jadd(n31, n36); n32 = jadd(n32, n37); n33 = jadd(n33, n38);
        }
        n7 = jadd(n7, n12); n8 = jadd(n8, n13); n9 = jadd(n9, n14);
        n10 = jadd(n10, n15); n11 = jadd(n11, n16);
        n17 = jadd(n17, n22); n18 = jadd(n18, n23); n19 = jadd(n19, n24);
        n20 = jadd(n20, n25); n21 = jadd(n21, n26);
        n6 = jadd(n6, n5);
    }
}

/* raycasting.drawrecUVSA — alpha blend, then drawrecUVS. */
static void drawrec_uvsa(rcontext *rc, texture *tx, kpixel *p2, kpixel *p3, kpixel *p4, kpixel *p5, jint n) {
    jint n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16;
    int32_t *dst;
    int32_t *tex;
    if (n <= 0) return;
    if (n >= 255) { drawrec_uvs(rc, tx, p2, p3, p4, p5); return; }
    n2 = jsub(rc->ClipX2, rc->ClipX1);
    n3 = jsub(rc->ClipY2, rc->ClipY1);
    n4 = rc->Modulo;
    n5 = jadd(rc->ClipX1, jmul(rc->ClipY1, rc->Modulo));
    dst = rc->RC_RootContext->CurrentChunk;
    tex = tx->pixels;
    n6 = tx->Modulo;
    n7 = jsub(255, n);
    if (n2 == 0 || n3 == 0) return;
    n8 = p2->u; n9 = p2->v;
    n10 = jdiv(jsub(p4->u, n8), n3);
    n11 = jdiv(jsub(p4->v, n9), n3);
    n12 = p3->u; n13 = p3->v;
    n14 = jdiv(jsub(p5->u, n12), n3);
    n15 = jdiv(jsub(p5->v, n13), n3);
    for (n16 = 0; n16 < n3; ++n16) {
        jint n17 = n5, n18 = n8, n19 = n9;
        jint n20 = jdiv(jsub(n12, n18), n2);
        jint n21 = jdiv(jsub(n13, n19), n2);
        jint n22;
        for (n22 = 0; n22 < n2; ++n22) {
            jint n23 = tex[jadd(jmul(jshr(n19, 16), n6), jshr(n18, 16))];
            if (n23 != -1) {
                jint n24 = dst[n17];
                jint n25 = jshr(jand(n23, 0xFF0000), 16);
                jint n26 = jshr(jand(n23, 0xFF00), 8);
                n23 = jand(n23, 0xFF);
                jint n27 = jshr(jand(n24, 0xFF0000), 16);
                jint n28 = jshr(jand(n24, 0xFF00), 8);
                n24 = jand(n24, 0xFF);
                n27 = jshr(jadd(jmul(n27, n7), jmul(n25, n)), 8);
                n28 = jshr(jadd(jmul(n28, n7), jmul(n26, n)), 8);
                n24 = jshr(jadd(jmul(n24, n7), jmul(n23, n)), 8);
                dst[n17] = (int32_t)(0xFF000000u | ((uint32_t)jshl(n27, 16)) |
                                     ((uint32_t)jshl(n28, 8)) | (uint32_t)n24);
            }
            ++n17;
            n18 = jadd(n18, n20);
            n19 = jadd(n19, n21);
        }
        n8 = jadd(n8, n10); n9 = jadd(n9, n11);
        n12 = jadd(n12, n14); n13 = jadd(n13, n15);
        n5 = jadd(n5, n4);
    }
}

/* raycasting.colorclip */
void rcs_colorclip(rcaster *rcs, helpfade *hf, kpixel *px, double d) {
    (void)rcs;
    px->r = jint_of(hf->Frv1);
    if (d > hf->Frd2) px->r = jint_of(hf->Frv2);
    else if (d > hf->Frd1)
        px->r = jint_of(hf->Frv1 + (((hf->Frv2 - hf->Frv1) * (d - hf->Frd1)) / (hf->Frd2 - hf->Frd1)));
    px->g = jint_of(hf->Fgv1);
    if (d > hf->Fgd2) px->g = jint_of(hf->Fgv2);
    else if (d > hf->Fgd1)
        px->g = jint_of(hf->Fgv1 + (((hf->Fgv2 - hf->Fgv1) * (d - hf->Fgd1)) / (hf->Fgd2 - hf->Fgd1)));
    px->b = jint_of(hf->Fbv1);
    if (d > hf->Fbd2) px->b = jint_of(hf->Fbv2);
    else if (d > hf->Fbd1)
        px->b = jint_of(hf->Fbv1 + (((hf->Fbv2 - hf->Fbv1) * (d - hf->Fbd1)) / (hf->Fbd2 - hf->Fbd1)));
}

/* raycasting.Position3D */
void rcs_position3d(rcaster *rcs, double d, double d2, double d3, double d4,
                    double d5, double d6, double d7, rcontext *rc) {
    double d8, d9, d10, d11, d12, d13;
    jint n, n2;
    if (!rc->drawable) return;
    d8 = cos(d4); d9 = sin(d4);
    d10 = cos(d5); d11 = sin(d5);
    d12 = cos(d6); d13 = sin(d6);
    rc->PosX = d; rc->PosY = d2; rc->PosZ = d3;
    for (n = 0; n < rc->MaxPreY; ++n) {
        for (n2 = 0; n2 < rc->MaxPreX; ++n2) {
            kpixel *px = rc->Pretable[n2 + jmul(n, rc->PretableX)];
            double d14 = px->XI, d15 = px->YI;
            double d16 = (d8 * d14) - (d9 * d15);
            double d17 = (d9 * d14) + (d8 * d15);
            double d18 = d7, d19 = d16;
            double d20 = (d10 * d17) - (d11 * d18);
            double d21 = (d11 * d17) + (d10 * d18);
            px->XF = (d13 * d21) + (d12 * d19);
            px->YF = d20;
            px->ZF = (d12 * d21) - (d13 * d19);
        }
    }
    (void)rcs;
}

/* Shared tail of Setzoom3D/SetzoomP: publish uv and rasterise the rectangle. */
static void setzoom_publish(rcaster *rcs, rcontext *rc, texture *tx, jint x1, jint y1,
                            jint x2, jint y2, double u1, double v1, double u2, double v2, jint alpha) {
    kpixel *p00 = rcs->Rootrc->Pretable[0];
    kpixel *p10 = rcs->Rootrc->Pretable[1];
    kpixel *p01 = rcs->Rootrc->Pretable[rcs->Rootrc->PretableX];
    kpixel *p11 = rcs->Rootrc->Pretable[rcs->Rootrc->PretableX + 1];
    p01->u = p00->u = jint_of(u1 * 65536.0);
    p11->u = p10->u = jint_of(u2 * 65536.0);
    p10->v = p00->v = jint_of(v1 * 65536.0);
    p11->v = p01->v = jint_of(v2 * 65536.0);
    rcr_setsub(rcs->OneRec, rc, x1, y1, x2, y2);
    drawrec_uvsa(rcs->OneRec, tx, p00, p10, p01, p11, alpha);
}

void rcs_setzoom3d(rcaster *rcs, double d, double d2, double d3, double d4,
                   double d5, double d6, rcontext *rc, texture *tx, double d7) {
    double d8 = 0.0, d9 = 0.0, d10 = 0.0, d11, d12, d13, d14, d15, d16, d17, d18 = 0.0,
           d19 = 0.0, d20, d21;
    if (!rc->drawable) return;
    d11 = (double)jsub(rc->ClipX2, rc->ClipX1);
    d12 = (double)jsub(rc->ClipY2, rc->ClipY1);
    d13 = d6 / d3;
    if (d3 <= 0.01) return;
    d14 = (double)tx->Modulo / d11;
    d15 = (double)tx->Height / d12;
    d16 = (d10 = d * d13);
    d17 = (d9 = d2 * d13);
    d20 = (double)jsub(tx->Modulo, 1);
    d21 = (double)jsub(tx->Height, 1);
    d14 = ((d14 * 0.5) * d4) * d13;
    d15 = ((d15 * 0.5) * d5) * d13;
    d10 -= d14; d9 -= d15; d16 += d14; d17 += d15;
    d10 = ((d10 * d11) + (double)rc->ClipX1) + (d11 * 0.5);
    d9 = ((d9 * d12) + (double)rc->ClipY1) + (d12 * 0.5);
    d16 = ((d16 * d11) + (double)rc->ClipX1) + (d11 * 0.5);
    d17 = ((d17 * d12) + (double)rc->ClipY1) + (d12 * 0.5);
    if (d10 > d16) { d8 = d16; d16 = d10; d10 = d8; d8 = d20; d20 = d18; d18 = d8; }
    if (d9 > d17) { d8 = d17; d17 = d9; d9 = d8; d8 = d21; d21 = d19; d19 = d8; }
    if (d16 > (double)rc->ClipX2) { d20 = (((double)rc->ClipX2 - d10) * (d20 - d18)) / (d16 - d10); d16 = rc->ClipX2; }
    if (d10 < (double)rc->ClipX1) { d18 = (((double)rc->ClipX1 - d10) * (d20 - d18)) / (d16 - d10); d10 = rc->ClipX1; }
    if (d17 > (double)rc->ClipY2) { d21 = (((double)rc->ClipY2 - d9) * (d21 - d19)) / (d17 - d9); d17 = rc->ClipY2; }
    if (d9 < (double)rc->ClipY1) { d19 = (((double)rc->ClipY1 - d9) * (d21 - d19)) / (d17 - d9); d9 = rc->ClipY1; }
    setzoom_publish(rcs, rc, tx, jint_of(d10), jint_of(d9), jint_of(d16), jint_of(d17),
                    d18, d19, d20, d21, jint_of(d7));
}

void rcs_setzoomp(rcaster *rcs, double d, double d2, double d3, double d4,
                  rcontext *rc, texture *tx, double d5) {
    double d6 = 0.0, d7, d8, d9 = 0.0, d10 = 0.0, d11, d12;
    if (!rc->drawable) return;
    d7 = (double)jsub(rc->ClipX2, rc->ClipX1);
    d8 = (double)jsub(rc->ClipY2, rc->ClipY1);
    d11 = (double)jsub(tx->Modulo, 1);
    d12 = (double)jsub(tx->Height, 1);
    d = (d * d7) + (double)rc->ClipX1;
    d2 = (d2 * d8) + (double)rc->ClipY1;
    d3 = (d3 * d7) + (double)rc->ClipX1;
    d4 = (d4 * d8) + (double)rc->ClipY1;
    if (d > d3) { d6 = d3; d3 = d; d = d6; d6 = d11; d11 = d9; d9 = d6; }
    if (d2 > d4) { d6 = d4; d4 = d2; d2 = d6; d6 = d12; d12 = d10; d10 = d6; }
    if (d3 > (double)rc->ClipX2) { d11 = (((double)rc->ClipX2 - d) * (d11 - d9)) / (d3 - d); d3 = rc->ClipX2; }
    if (d < (double)rc->ClipX1) { d9 = (((double)rc->ClipX1 - d) * (d11 - d9)) / (d3 - d); d = rc->ClipX1; }
    if (d4 > (double)rc->ClipY2) { d12 = (((double)rc->ClipY2 - d2) * (d12 - d10)) / (d4 - d2); d4 = rc->ClipY2; }
    if (d2 < (double)rc->ClipY1) { d10 = (((double)rc->ClipY1 - d2) * (d12 - d10)) / (d4 - d2); d2 = rc->ClipY1; }
    setzoom_publish(rcs, rc, tx, jint_of(d), jint_of(d2), jint_of(d3), jint_of(d4),
                    d9, d10, d11, d12, jint_of(d5));
}

/* raycasting.flare3D — same geometry as Setzoom3D, then drawrecUVM. */
void rcs_flare3d(rcaster *rcs, double d, double d2, double d3, double d4,
                 double d5, double d6, rcontext *rc, texture *tx) {
    double d7 = 0.0, d8 = 0.0, d9 = 0.0, d10, d11, d12, d13, d14, d15, d16, d17 = 0.0,
           d18 = 0.0, d19, d20;
    if (!rc->drawable) return;
    d10 = (double)jsub(rc->ClipX2, rc->ClipX1);
    d11 = (double)jsub(rc->ClipY2, rc->ClipY1);
    d12 = d6 / d3;
    if (d3 <= 0.01) return;
    d13 = (double)tx->Modulo / d10;
    d14 = (double)tx->Height / d11;
    d15 = (d9 = d * d12);
    d16 = (d8 = d2 * d12);
    d19 = (double)jsub(tx->Modulo, 1);
    d20 = (double)jsub(tx->Height, 1);
    d13 = ((d13 * 0.5) * d4) * d12;
    d14 = ((d14 * 0.5) * d5) * d12;
    d9 -= d13; d8 -= d14; d15 += d13; d16 += d14;
    d9 = ((d9 * d10) + (double)rc->ClipX1) + (d10 * 0.5);
    d8 = ((d8 * d11) + (double)rc->ClipY1) + (d11 * 0.5);
    d15 = ((d15 * d10) + (double)rc->ClipX1) + (d10 * 0.5);
    d16 = ((d16 * d11) + (double)rc->ClipY1) + (d11 * 0.5);
    if (d9 > d15) { d7 = d15; d15 = d9; d9 = d7; d7 = d19; d19 = d17; d17 = d7; }
    if (d8 > d16) { d7 = d16; d16 = d8; d8 = d7; d7 = d20; d20 = d18; d18 = d7; }
    if (d15 > (double)rc->ClipX2) { d19 = (((double)rc->ClipX2 - d9) * (d19 - d17)) / (d15 - d9); d15 = rc->ClipX2; }
    if (d9 < (double)rc->ClipX1) { d17 = (((double)rc->ClipX1 - d9) * (d19 - d17)) / (d15 - d9); d9 = rc->ClipX1; }
    if (d16 > (double)rc->ClipY2) { d20 = (((double)rc->ClipY2 - d8) * (d20 - d18)) / (d16 - d8); d16 = rc->ClipY2; }
    if (d8 < (double)rc->ClipY1) { d18 = (((double)rc->ClipY1 - d8) * (d20 - d18)) / (d16 - d8); d8 = rc->ClipY1; }
    {
        kpixel *p00 = rcs->Rootrc->Pretable[0];
        kpixel *p10 = rcs->Rootrc->Pretable[1];
        kpixel *p01 = rcs->Rootrc->Pretable[rcs->Rootrc->PretableX];
        kpixel *p11 = rcs->Rootrc->Pretable[rcs->Rootrc->PretableX + 1];
        p01->u = p00->u = jint_of(d17 * 65536.0);
        p11->u = p10->u = jint_of(d19 * 65536.0);
        p10->v = p00->v = jint_of(d18 * 65536.0);
        p11->v = p01->v = jint_of(d20 * 65536.0);
        rcr_setsub(rcs->OneRec, rc, jint_of(d9), jint_of(d8), jint_of(d15), jint_of(d16));
        drawrec_uvm(rcs->OneRec, tx, p00, p10, p01, p11);
    }
}

/* raycasting.findfloor */
void rcs_findfloor(rcaster *rcs, double d, double d2, double d3, double d4,
                   rcontext *rc, helpfade *hf) {
    jint n, n2;
    if (!rc->drawable) return;
    hf->Frv1 *= 65536.0; hf->Frv2 *= 65536.0;
    hf->Fgv1 *= 65536.0; hf->Fgv2 *= 65536.0;
    hf->Fbv1 *= 65536.0; hf->Fbv2 *= 65536.0;
    d3 *= 65536.0; d4 *= 65536.0;
    d += rc->PosY; d2 -= rc->PosY;
    for (n = 0; n < rc->MaxPreY; ++n) {
        for (n2 = 0; n2 < rc->MaxPreX; ++n2) {
            kpixel *px = rc->Pretable[n2 + jmul(n, rc->PretableX)];
            double d9 = px->XF, d10 = px->YF, d11 = px->ZF;
            double d12, d6, d7, d8;
            if (d10 == 0.0) d10 = 1.0E-8;
            d12 = 1.0 / d10;
            if (d10 < 0.0) {
                d8 = (d11 * d) * d12;
                d7 = (d9 * d) * d12;
                px->u = jint_of((d8 - rc->PosZ) * d3);
                px->v = jint_of((d7 - rc->PosX) * d4);
                d6 = sqrt((d8 * d8) + (d7 * d7) + (d * d));
            } else {
                d8 = (d11 * d2) * d12;
                d7 = (d9 * d2) * d12;
                px->u = jint_of(((-d8) - rc->PosZ) * d3);
                px->v = jint_of(((-d7) - rc->PosX) * d4);
                d6 = sqrt((d8 * d8) + (d7 * d7) + (d2 * d2));
            }
            rcs_colorclip(rcs, hf, px, d6);
        }
    }
}

/* raycasting.findtunnel */
void rcs_findtunnel(rcaster *rcs, double d, double d2, rcontext *rc, helpfade *hf) {
    double d3 = 65536.0;
    double d4 = 5340353.715440872;
    jint n, n2;
    if (!rc->drawable) return;
    hf->Frv1 *= d3; hf->Frv2 *= d3;
    hf->Fgv1 *= d3; hf->Fgv2 *= d3;
    hf->Fbv1 *= d3; hf->Fbv2 *= d3;
    d2 *= 65536.0;
    for (n = 0; n < rc->MaxPreY; ++n) {
        for (n2 = 0; n2 < rc->MaxPreX; ++n2) {
            kpixel *px = rc->Pretable[n2 + jmul(n, rc->PretableX)];
            double d5 = px->XF, d6 = dabs(px->YF), d7 = px->ZF;
            double d8 = sqrt((d5 * d5) + (d6 * d6));
            double d9, d10, d11, d12;
            if (d8 == 0.0) d8 = 1.0E-7;
            d9 = atan2(d5, d6);
            d10 = cos(d9) * d;
            d11 = sin(d9) * d;
            d12 = (d7 * d) / d8;
            px->u = jint_of((d12 + rc->PosZ) * d2);
            px->v = jadd(jint_of(d9 * d4), (jint)0x800000);
            d8 = sqrt((d12 * d12) + (d10 * d10) + (d11 * d11));
            rcs_colorclip(rcs, hf, px, d8);
        }
    }
}

/* raycasting.findtwirl */
void rcs_findtwirl(rcaster *rcs, double d, double d2, double d3, double d4,
                   double d5, double d6, rcontext *rc) {
    double d7 = 5.0, d8 = 750.0;
    double d9 = d8 - d7;
    double d10 = 5340353.715440872;
    jint n, n2, n3;
    (void)d9;
    (void)rcs;
    if (!rc->drawable) return;
    n = jand(jint_of(rc->PosZ * 65536.0), (jint)0xFFFFFF);
    (void)n;
    for (n2 = 0; n2 < rc->MaxPreY; ++n2) {
        for (n3 = 0; n3 < rc->MaxPreX; ++n3) {
            kpixel *px = rc->Pretable[n3 + jmul(n2, rc->PretableX)];
            double d11 = px->XF * 256.0;
            double d12 = px->YF * 256.0;
            double d13 = atan2(d11, d12);
            double d14 = sqrt((d11 * d11) + (d12 * d12));
            double d15;
            jint n4;
            d13 += d3 * sin((d + ((d14 * 3.141592653589793) / 128.0)) * d2);
            d14 += d6 * sin((d4 + ((d14 * 3.141592653589793) / 128.0)) * d5);
            px->u = jint_of((sin(d13) * d14 * 65536.0) + (rc->PosX * 65536.0));
            px->v = jint_of((cos(d13) * d14 * 65536.0) + (rc->PosY * 65536.0));
            d15 = d12 - 8.0;
            d13 = atan2(d11, d15);
            d14 = sqrt((d11 * d11) + (d15 * d15));
            d13 += d3 * sin((d + ((d14 * 3.141592653589793) / 128.0)) * d2);
            d14 += d6 * sin((d4 + ((d14 * 3.141592653589793) / 128.0)) * d5);
            n4 = jsub(px->u, jint_of((sin(d13) * d14 * 65536.0) + (rc->PosX * 65536.0)));
            px->r = jmul(n4, 26);
            px->g = jmul(n4, 24);
            px->b = jmul(n4, 23);
        }
    }
    (void)d10;
}
