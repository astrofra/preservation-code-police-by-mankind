/* Code Police — the object graph (C). See objects.h. */
#include "objects.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- string helpers over a KsStr view ---- */

/* Copies a KsStr to a NUL-terminated heap string. */
static char *dup_ks(const KsStr *s) {
    char *p = (char *)malloc((size_t)s->len + 1);
    if (!p) return NULL;
    memcpy(p, s->data, (size_t)s->len);
    p[s->len] = '\0';
    return p;
}

/* lower-case copy of a KsStr (trimmed), for keyword comparison */
static void lower_trim(const KsStr *s, char *out, int cap) {
    KsStr t = ks_trim(s);
    int i, n = t.len < cap - 1 ? t.len : cap - 1;
    for (i = 0; i < n; ++i) {
        char c = t.data[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
        out[i] = c;
    }
    out[n] = '\0';
}

static double parse_double(const KsStr *s) {
    char *p = dup_ks(s);
    double v = p ? atof(p) : 0.0;
    free(p);
    return v;
}

static jint parse_int(const KsStr *s) {
    char *p = dup_ks(s);
    long v = p ? atol(p) : 0;
    free(p);
    return (jint)v;
}

/* FindInto(field_index, sep) as an owned KsStr. */
static KsStr field(const char *data, int len, int index, const char *sep) {
    char *p = (char *)malloc((size_t)len + 1);
    KsStr r;
    if (!p) { r.data = NULL; r.len = 0; return r; }
    memcpy(p, data, (size_t)len);
    p[len] = '\0';
    r = ks_find_into(p, len, index, sep);
    free(p);
    return r;
}

/* Same, but trimmed, returning a NUL-terminated owned string. */
static char *field_str(const char *data, int len, int index, const char *sep) {
    KsStr r = field(data, len, index, sep);
    char *out;
    KsStr t;
    if (!r.data) return NULL;
    t = ks_trim(&r);
    out = (char *)malloc((size_t)t.len + 1);
    if (out) { memcpy(out, t.data, (size_t)t.len); out[t.len] = '\0'; }
    ks_str_free(r);
    return out;
}

static double field_atof(const char *data, int len, int index, const char *sep) {
    char *v = field_str(data, len, index, sep);
    double r = v ? atof(v) : 0.0;
    free(v);
    return r;
}

static int field_atoi(const char *data, int len, int index, const char *sep) {
    char *v = field_str(data, len, index, sep);
    int r = v ? atoi(v) : 0;
    free(v);
    return r;
}

/* ---- kTable ---- */

static jtable *table_create(const char *data, int len) {
    jtable *t = (jtable *)calloc(1, sizeof(jtable));
    int i;
    if (!t) return NULL;
    t->lengthtable = ks_find_how_many(data, len, "|") - 1;
    if (t->lengthtable < 1) { t->lengthtable = 0; return t; }
    t->datetable = (double *)calloc((size_t)t->lengthtable, sizeof(double));
    t->floattable = (double *)calloc((size_t)t->lengthtable, sizeof(double));
    if (!t->datetable || !t->floattable) return t; /* partial; harmless at 0 */
    for (i = 0; i < (int)t->lengthtable; ++i) {
        char *entry = field_str(data, len, 1 + i, "|");
        if (entry) {
            KsStr es = {entry, (int)strlen(entry)};
            KsStr d = ks_find_into(es.data, es.len, 0, ",");
            KsStr f = ks_find_into(es.data, es.len, 1, ",");
            if (d.data) { t->datetable[i] = parse_double(&d); ks_str_free(d); }
            if (f.data) { t->floattable[i] = parse_double(&f); ks_str_free(f); }
            free(entry);
        }
    }
    return t;
}

/* kTable.spline — reconstructed Java line 672. */
static double table_spline(const jtable *t, double d) {
    jint n = t->lengthtable - 1;
    jint n2, n3, n4, n5, n6, n7, n8;
    double d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14;
    if (t->lengthtable <= 0) return 0.0;
    if (d <= t->datetable[0]) return t->floattable[0];
    n2 = 0; n3 = n; n4 = (n2 + n3) >> 1; n5 = n3;
    if (d >= t->datetable[n5]) return t->floattable[n5];
    n5 = n4; n6 = n4 + 1;
    while (!(d >= t->datetable[n5]) || !(d < t->datetable[n6])) {
        if (d < t->datetable[n5]) n3 = n4; else n2 = n4;
        n5 = (n4 = (n2 + n3) >> 1);
        n6 = n4 + 1;
    }
    n7 = (n4 == 0) ? n5 : (n5 - 1);
    n8 = ((n4 + 1) == n) ? n6 : (n6 + 1);
    d2 = t->datetable[n5];
    d3 = t->datetable[n6];
    d4 = d3 - d2;
    d5 = (t->floattable[n6] - t->floattable[n7]) / (d3 - t->datetable[n7]);
    d6 = (t->floattable[n8] - t->floattable[n5]) / (t->datetable[n8] - d2);
    d7 = d5;
    d8 = t->floattable[n5];
    d9 = t->floattable[n6];
    d10 = d4;
    d11 = d10 * d10;
    d12 = (((-(d10 * d6)) - ((d7 * 2.0) * d10)) + (3.0 * (d9 - d8))) / d11;
    d13 = ((d6 - ((2.0 * d12) * d10)) - d7) / (3.0 * d11);
    d11 = d - d2;
    d9 = d11 * d11;
    d14 = d9 * d11;
    d10 = ((((d13 * d14) + (d12 * d9)) + (d7 * d11)) + d8);
    return d10;
}

/* ---- camera ---- */

static void camera_target(jcamera *c, double d, double d2, double d3, double d4,
                          double d5, double d6, double d7, double d8) {
    double d9, d10;
    c->X = d; c->Y = d2; c->Z = d3; c->FOV = d7; c->O1 = d8;
    d4 -= d; d5 -= d2;
    if ((d6 -= d3) == 0.0) d6 = 1.0E-6;
    d9 = d4 / d6;
    d9 = atan(d9);
    if (d6 > 0.0) d9 += 3.141592653589793;
    c->O3 = (d9 += 3.141592653589793);
    d10 = (d6 * cos(d9)) + (d4 * sin(d9));
    if (d10 == 0.0) d10 = 1.0E-6;
    d9 = d5 / d10;
    d9 = atan(d9);
    if (d10 > 0.0) d9 += 3.141592653589793;
    c->O2 = -(d9 += 3.141592653589793);
}

/* ---- particle ---- */

static void particle_reset(jparticle *p, java_random *rnd, double d, double d2,
                           double d3, double d4, double d5, double d6, double d7, double d8) {
    double d9 = 0.0, d10 = 0.0, d11 = 0.0;
    double d12 = 1.0 / (double)p->nbparticle;
    double d13 = 0.0, d14 = 0.0;
    double d15 = ((3.141592653589793 * 2) * d) * d12;
    double d16 = ((3.141592653589793 * 2) * d2) * d12;
    int n;
    for (n = 0; n < p->nbparticle; ++n) {
        double d17 = d3 + (jr_double(rnd) * (d4 - d3));
        double d18 = 0.0;
        double d19 = cos(d13) * d17;
        double d20 = sin(d13) * d17;
        p->posx[n] = (cos(d14) * d18) - (sin(d14) * d20);
        p->posy[n] = d19;
        p->posz[n] = (sin(d14) * d18) + (cos(d14) * d20);
        p->force[n] = 1.0;
        d17 = 0.05;
        p->Vx[n] = p->posz[n] * d17;
        p->Vy[n] = p->posx[n] * d17;
        p->Vz[n] = p->posy[n] * d17;
        d9 += p->posx[n]; d10 += p->posy[n]; d11 += p->posz[n];
        d13 += d15; d14 += d16;
    }
    p->aveX = d9 * d12; p->aveY = d10 * d12; p->aveZ = d11 * d12;
    (void)d5; (void)d6; (void)d7; (void)d8;
}

static jparticle *particle_create(java_random *rnd, jint n, double d, double d2,
                                  double d3, double d4, double d5, double d6,
                                  double d7, double d8) {
    jparticle *p = (jparticle *)calloc(1, sizeof(jparticle));
    int i;
    if (!p) return NULL;
    if (n == 0) n = 1;
    p->nbparticle = n;
    p->posx = (double *)calloc((size_t)n, sizeof(double));
    p->posy = (double *)calloc((size_t)n, sizeof(double));
    p->posz = (double *)calloc((size_t)n, sizeof(double));
    p->Vx = (double *)calloc((size_t)n, sizeof(double));
    p->Vy = (double *)calloc((size_t)n, sizeof(double));
    p->Vz = (double *)calloc((size_t)n, sizeof(double));
    p->Ax = (double *)calloc((size_t)n, sizeof(double));
    p->Ay = (double *)calloc((size_t)n, sizeof(double));
    p->Az = (double *)calloc((size_t)n, sizeof(double));
    p->force = (double *)calloc((size_t)n, sizeof(double));
    for (i = 0; i < n; ++i) p->force[i] = 0.0;
    particle_reset(p, rnd, d, d2, d3, d4, d5, d6, d7, d8);
    return p;
}

static void particle_move(jparticle *p) {
    double d = 0.0, d2 = 0.0, d3 = 0.0;
    double d4 = 1.0 / (double)p->nbparticle;
    int n;
    for (n = 0; n < p->nbparticle; ++n) {
        p->Vx[n] += p->Ax[n];
        p->Vy[n] += p->Ay[n];
        p->Vz[n] += p->Az[n];
        p->posx[n] += p->Vx[n];
        p->posy[n] += p->Vy[n];
        p->posz[n] += p->Vz[n];
        d += p->posx[n]; d2 += p->posy[n]; d3 += p->posz[n];
    }
    p->aveX = d * d4; p->aveY = d2 * d4; p->aveZ = d3 * d4;
}

static void particle_mvtgalax(jparticle *p) {
    int n;
    for (n = 0; n < p->nbparticle; ++n) {
        double d4 = p->posx[n], d5 = p->posy[n], d6 = p->posz[n];
        double d7 = pow(((d4 * d4) + (d5 * d5) + (d6 * d6)), 1.5) * 0.001;
        p->Ax[n] = (-d4) * d7;
        p->Ay[n] = (-d5) * d7;
        p->Az[n] = (-d6) * d7;
    }
    particle_move(p);
}

static void particle_mvtfloor(jparticle *p, double d, double d2, double d3, double d4) {
    int n;
    for (n = 0; n < p->nbparticle; ++n) {
        double d9 = p->posy[n];
        if (d9 < d) { p->posy[n] = d; p->Vy[n] *= -1.0; }
        if (d9 > d2) { p->posy[n] = d2; p->Vy[n] *= -1.0; }
        p->Vx[n] *= d4; p->Vy[n] *= d4; p->Vz[n] *= d4;
        p->Ax[n] = 0.0; p->Ay[n] = d3; p->Az[n] = 0.0;
    }
    particle_move(p);
}

/* forward */
static void effect_rendah(jworld *w, jeffect *e, jint n);

/* ---- parameter doubleout / Out ---- */

static double param_doubleout(jparam *p, jworld *w, double d) {
    switch (p->code) {
    case PARAM_COS: return p->a + (cos(p->b * d) * p->c);
    case PARAM_SIN: return p->a + (sin(p->b * d) * p->c);
    case PARAM_CTE: return p->a;
    case PARAM_RND: return p->a + (jr_double(&w->random) * (p->b - p->a));
    case PARAM_SPL:
        return table_spline((const jtable *)p->obj, d - p->a);
    case PARAM_AFF: return p->a + (p->b * d);
    default: return 0.0;
    }
}

/* Resolves an object parameter to its typed pointer. */
static void *param_out(jparam *p, jworld *w) {
    if (p->code == PARAM_OBJ) return p->obj ? p->obj : &w->root;
    return p->obj;
}

/* ---- effect ---- */

static void effect_rendah(jworld *w, jeffect *e, jint n) {
    double d;
    double d2 = (double)n;
    int code = e->keffectcode;
    if (code == 1) { /* twirl */
        rcontext *rc = (rcontext *)param_out(e->params[9], w);
        rcs_position3d(&w->rcs, param_doubleout(e->params[0], w, d2),
            param_doubleout(e->params[1], w, d2), 0.0, 0.0, 0.0, 0.0, 0.0, rc);
        rcs_findtwirl(&w->rcs, param_doubleout(e->params[2], w, d2),
            param_doubleout(e->params[3], w, d2), param_doubleout(e->params[4], w, d2),
            param_doubleout(e->params[5], w, d2), param_doubleout(e->params[6], w, d2),
            param_doubleout(e->params[7], w, d2), rc);
        rcs_draw(&w->rcs, (texture *)param_out(e->params[8], w), rc);
    }
    if (code == 2) { /* sprit */
        rcs_setzoomp(&w->rcs, param_doubleout(e->params[1], w, d2),
            param_doubleout(e->params[2], w, d2), param_doubleout(e->params[3], w, d2),
            param_doubleout(e->params[4], w, d2),
            (rcontext *)param_out(e->params[5], w),
            (texture *)param_out(e->params[0], w),
            param_doubleout(e->params[6], w, d2));
    }
    if (code == 3) { /* tunnel */
        jcamera *cam = (jcamera *)param_out(e->params[0], w);
        e->hhf.Frd1 = param_doubleout(e->params[5], w, d2);
        e->hhf.Frv1 = param_doubleout(e->params[6], w, d2);
        e->hhf.Frd2 = param_doubleout(e->params[7], w, d2);
        e->hhf.Frv2 = param_doubleout(e->params[8], w, d2);
        e->hhf.Fgd1 = param_doubleout(e->params[9], w, d2);
        e->hhf.Fgv1 = param_doubleout(e->params[10], w, d2);
        e->hhf.Fgd2 = param_doubleout(e->params[11], w, d2);
        e->hhf.Fgv2 = param_doubleout(e->params[12], w, d2);
        e->hhf.Fbd1 = param_doubleout(e->params[13], w, d2);
        e->hhf.Fbv1 = param_doubleout(e->params[14], w, d2);
        e->hhf.Fbd2 = param_doubleout(e->params[15], w, d2);
        e->hhf.Fbv2 = param_doubleout(e->params[16], w, d2);
        rcs_position3d(&w->rcs, 0.0, 0.0, cam->Z, cam->O1, cam->O2, cam->O3, cam->FOV,
            (rcontext *)param_out(e->params[4], w));
        rcs_findtunnel(&w->rcs, param_doubleout(e->params[1], w, d2),
            param_doubleout(e->params[2], w, d2),
            (rcontext *)param_out(e->params[4], w), &e->hhf);
        rcs_draw(&w->rcs, (texture *)param_out(e->params[3], w),
            (rcontext *)param_out(e->params[4], w));
    }
    if (code == 4) { /* ground */
        jcamera *cam = (jcamera *)param_out(e->params[0], w);
        e->hhf.Frd1 = param_doubleout(e->params[7], w, d2);
        e->hhf.Frv1 = param_doubleout(e->params[8], w, d2);
        e->hhf.Frd2 = param_doubleout(e->params[9], w, d2);
        e->hhf.Frv2 = param_doubleout(e->params[10], w, d2);
        e->hhf.Fgd1 = param_doubleout(e->params[11], w, d2);
        e->hhf.Fgv1 = param_doubleout(e->params[12], w, d2);
        e->hhf.Fgd2 = param_doubleout(e->params[13], w, d2);
        e->hhf.Fgv2 = param_doubleout(e->params[14], w, d2);
        e->hhf.Fbd1 = param_doubleout(e->params[15], w, d2);
        e->hhf.Fbv1 = param_doubleout(e->params[16], w, d2);
        e->hhf.Fbd2 = param_doubleout(e->params[17], w, d2);
        e->hhf.Fbv2 = param_doubleout(e->params[18], w, d2);
        rcs_position3d(&w->rcs, cam->X, cam->Y, cam->Z, cam->O1, cam->O2, cam->O3,
            cam->FOV, (rcontext *)param_out(e->params[4], w));
        rcs_findfloor(&w->rcs, param_doubleout(e->params[1], w, d2),
            param_doubleout(e->params[2], w, d2), param_doubleout(e->params[5], w, d2),
            param_doubleout(e->params[6], w, d2),
            (rcontext *)param_out(e->params[4], w), &e->hhf);
        rcs_draw(&w->rcs, (texture *)param_out(e->params[3], w),
            (rcontext *)param_out(e->params[4], w));
    }
    if (code == 5) { /* sprit3d */
        rcs_setzoom3d(&w->rcs, param_doubleout(e->params[0], w, d2),
            param_doubleout(e->params[1], w, d2), param_doubleout(e->params[2], w, d2),
            param_doubleout(e->params[3], w, d2), param_doubleout(e->params[4], w, d2),
            param_doubleout(e->params[5], w, d2),
            (rcontext *)param_out(e->params[7], w),
            (texture *)param_out(e->params[6], w),
            param_doubleout(e->params[8], w, d2));
    }
    if (code == 6) { /* setrect */
        rcontext *rc = (rcontext *)param_out(e->params[0], w);
        rcr_setrect(rc, &w->root, param_doubleout(e->params[1], w, d2),
            param_doubleout(e->params[2], w, d2), param_doubleout(e->params[3], w, d2),
            param_doubleout(e->params[4], w, d2));
    }
    if (code == 7) { /* flare3d */
        rcs_flare3d(&w->rcs, param_doubleout(e->params[0], w, d2),
            param_doubleout(e->params[1], w, d2), param_doubleout(e->params[2], w, d2),
            param_doubleout(e->params[3], w, d2), param_doubleout(e->params[4], w, d2),
            param_doubleout(e->params[5], w, d2),
            (rcontext *)param_out(e->params[7], w),
            (texture *)param_out(e->params[6], w));
    }
    if (code == 8) { /* motionblur */
        rcs_motion_blur(&w->rcs, (rcontext *)param_out(e->params[0], w));
    }
    if (code == 9) { /* particle */
        jparticle *p = (jparticle *)param_out(e->params[0], w);
        double d3 = param_doubleout(e->params[4], w, d2);
        if (d3 == 0.0) particle_mvtgalax(p);
        else if (d3 == 1.0) particle_mvtfloor(p,
            param_doubleout(e->params[9], w, d2), param_doubleout(e->params[10], w, d2),
            param_doubleout(e->params[11], w, d2), param_doubleout(e->params[12], w, d2));
        d = param_doubleout(e->params[5], w, d2);
        if (d == 0.0) {
            jcamera *cam = (jcamera *)param_out(e->params[6], w);
            double d4 = cos(cam->O1), d5 = sin(cam->O1);
            double d6 = cos(cam->O2), d7 = sin(cam->O2);
            double d8 = cos(cam->O3), d9 = sin(cam->O3);
            double da = param_doubleout(e->params[7], w, d2);
            double db = param_doubleout(e->params[8], w, d2);
            int i;
            for (i = 0; i < p->nbparticle; ++i) {
                double e9 = p->posx[i] - cam->X;
                double e10 = p->posy[i] - cam->Y;
                double e11 = p->posz[i] - cam->Z;
                double e12 = ((-d9) * e11) + (d8 * e9);
                double e13 = e10;
                double e14 = (d8 * e11) + (d9 * e9);
                double e15 = e12;
                double e16 = (d6 * e13) + (d7 * e14);
                double e17 = ((-d7) * e13) + (d6 * e14);
                e9 = (d4 * e15) + (d5 * e16);
                e10 = ((-d5) * e15) + (d4 * e16);
                e11 = e17;
                rcs_flare3d(&w->rcs, e9, e10, e11, da, db, cam->FOV,
                    (rcontext *)param_out(e->params[1], w),
                    (texture *)param_out(e->params[2], w));
            }
        } else if (d == 2.0) { /* drawsprit: Setzoom3D per particle, zoom 256 */
            jcamera *cam = (jcamera *)param_out(e->params[6], w);
            double d4 = cos(cam->O1), d5 = sin(cam->O1);
            double d6 = cos(cam->O2), d7 = sin(cam->O2);
            double d8 = cos(cam->O3), d9 = sin(cam->O3);
            double da = param_doubleout(e->params[7], w, d2);
            double db = param_doubleout(e->params[8], w, d2);
            int i;
            for (i = 0; i < p->nbparticle; ++i) {
                double e9 = p->posx[i] - cam->X;
                double e10 = p->posy[i] - cam->Y;
                double e11 = p->posz[i] - cam->Z;
                double e12 = ((-d9) * e11) + (d8 * e9);
                double e13 = e10;
                double e14 = (d8 * e11) + (d9 * e9);
                double e15 = e12;
                double e16 = (d6 * e13) + (d7 * e14);
                double e17 = ((-d7) * e13) + (d6 * e14);
                e9 = (d4 * e15) + (d5 * e16);
                e10 = ((-d5) * e15) + (d4 * e16);
                e11 = e17;
                rcs_setzoom3d(&w->rcs, e9, e10, e11, da, db, cam->FOV,
                    (rcontext *)param_out(e->params[1], w),
                    (texture *)param_out(e->params[2], w), 256.0);
            }
        }
    }
    if (code == 10) { /* setcamcoord */
        /* kcamera::set(d,d2,d3,d4,d5,d6,d7): X=d, Y=d2, Z=d3, FOV=d7, O1=d4,
         * O2=d5, O3=d6. The effect passes params[1..7] as d..d7. */
        jcamera *cam = (jcamera *)param_out(e->params[0], w);
        cam->X = param_doubleout(e->params[1], w, d2);
        cam->Y = param_doubleout(e->params[2], w, d2);
        cam->Z = param_doubleout(e->params[3], w, d2);
        cam->FOV = param_doubleout(e->params[7], w, d2);
        cam->O1 = param_doubleout(e->params[4], w, d2);
        cam->O2 = param_doubleout(e->params[5], w, d2);
        cam->O3 = param_doubleout(e->params[6], w, d2);
    }
    if (code == 11) { /* setcamtarget */
        jcamera *cam = (jcamera *)param_out(e->params[0], w);
        camera_target(cam, param_doubleout(e->params[1], w, d2),
            param_doubleout(e->params[2], w, d2), param_doubleout(e->params[3], w, d2),
            param_doubleout(e->params[4], w, d2), param_doubleout(e->params[5], w, d2),
            param_doubleout(e->params[6], w, d2), param_doubleout(e->params[7], w, d2),
            param_doubleout(e->params[8], w, d2));
    }
    /* code 12 (evesound) and code 2 draw() textures are milestone-5 items. */
}

/* ---- name lookup ---- */

static int find_name(char **names, int count, const KsStr *key) {
    KsStr t = ks_trim(key);
    int i;
    for (i = 0; i < count; ++i) {
        if ((int)strlen(names[i]) == t.len && strncmp(names[i], t.data, (size_t)t.len) == 0)
            return i;
    }
    return -1;
}

/* ---- building ---- */

static int add_texture(jworld *w, const AmigaPlatform *platform, const char *name,
                       const char *file) {
    char path[512];
    AmigaImage *img;
    jtexture *tx;
    int i, count;
    snprintf(path, sizeof path, "%s.png", file); /* FilePlatform::image appends .png */
    img = platform->load_image(platform->user, path);
    if (!img) { printf("missing texture %s -> %s\n", name, path); return -1; }
    tx = (jtexture *)calloc(1, sizeof(jtexture));
    tx->Modulo = img->width; tx->Height = img->height;
    count = img->width * img->height;
    tx->pixels = (int32_t *)malloc((size_t)count * sizeof(int32_t));
    if (!tx->pixels) { printf("texture alloc FAIL %s (%d px)\n", name, count); return -1; }
    for (i = 0; i < count; ++i) tx->pixels[i] = (int32_t)img->pixels[i];
    w->textures[w->texture_count] = tx;
    w->texture_names[w->texture_count] = strdup(name);
    ++w->texture_count;
    { FILE *m = fopen("CP:ran.txt", "w"); if (m) { fprintf(m, "texture %d/%d %s %dx%d\n", w->texture_count, 0, name, (int)tx->Modulo, (int)tx->Height); fclose(m); } }
    platform->free_image(platform->user, img);
    return 0;
}

jworld *jworld_create(const KsScript *script, const AmigaPlatform *platform) {
    jworld *w = (jworld *)calloc(1, sizeof(jworld));
    int i, j;
    if (!w) return NULL;
    jr_seed(&w->random, 888); /* the engine seeds 888 in init(), before building *//* the graph, because particle_reset consumes the RNG */
    w->width = 520; w->height = 300;
    rcr_init_root(&w->root, 520, 300);
    rcs_init(&w->rcs, &w->root);

    printf("graph stage: start\n");
    w->textures = (jtexture **)calloc((size_t)script->image_count + 1, sizeof(jtexture *));
    w->texture_names = (char **)calloc((size_t)script->image_count + 1, sizeof(char *));
    for (i = 0; i < script->image_count; ++i) {
        char *name = field_str(script->decl_images[i].data, script->decl_images[i].len, 0, "|");
        char *file = field_str(script->decl_images[i].data, script->decl_images[i].len, 1, "|");
        if (name && file) add_texture(w, platform, name, file);
        free(name); free(file);
        amiga_screen_set_progress(5 + (i + 1) * 45 /
            (script->image_count ? script->image_count : 1));
    }
    printf("graph stage: textures=%d\n", w->texture_count);

    w->tables = (struct jtable **)calloc((size_t)script->table_count + 1, sizeof(void *));
    w->table_names = (char **)calloc((size_t)script->table_count + 1, sizeof(char *));
    for (i = 0; i < script->table_count; ++i) {
        char *name = field_str(script->decl_tables[i].data, script->decl_tables[i].len, 0, "|");
        w->tables[w->table_count] = table_create(script->decl_tables[i].data, script->decl_tables[i].len);
        w->table_names[w->table_count] = name ? name : strdup("?");
        ++w->table_count;
        amiga_screen_set_progress(50 + (i + 1) * 25 /
            (script->table_count ? script->table_count : 1));
    }

    w->cameras = (struct jcamera **)calloc((size_t)script->camera_count + 1, sizeof(void *));
    w->camera_names = (char **)calloc((size_t)script->camera_count + 1, sizeof(char *));
    for (i = 0; i < script->camera_count; ++i) {
        const KsStr *d = &script->decl_cams[i];
        jcamera *c = (jcamera *)calloc(1, sizeof(jcamera));
        char *name = field_str(d->data, d->len, 0, "|");
        c->X = field_atof(d->data, d->len, 1, "|");
        c->Y = field_atof(d->data, d->len, 2, "|");
        c->Z = field_atof(d->data, d->len, 3, "|");
        c->O1 = field_atof(d->data, d->len, 4, "|");
        c->O2 = field_atof(d->data, d->len, 5, "|");
        c->O3 = field_atof(d->data, d->len, 6, "|");
        c->FOV = field_atof(d->data, d->len, 7, "|");
        w->cameras[w->camera_count] = c;
        w->camera_names[w->camera_count] = name ? name : strdup("?");
        ++w->camera_count;
    }

    w->particles = (struct jparticle **)calloc((size_t)script->ticle_count + 1, sizeof(void *));
    w->particle_names = (char **)calloc((size_t)script->ticle_count + 1, sizeof(char *));
    for (i = 0; i < script->ticle_count; ++i) {
        const KsStr *d = &script->decl_tice[i];
        char *name = field_str(d->data, d->len, 0, "|");
        jparticle *p = particle_create(&w->random,
            field_atoi(d->data, d->len, 1, "|"),
            field_atof(d->data, d->len, 2, "|"), field_atof(d->data, d->len, 3, "|"),
            field_atof(d->data, d->len, 4, "|"), field_atof(d->data, d->len, 5, "|"),
            field_atof(d->data, d->len, 6, "|"), field_atof(d->data, d->len, 7, "|"),
            field_atof(d->data, d->len, 8, "|"), field_atof(d->data, d->len, 9, "|"));
        w->particles[w->particle_count] = p;
        w->particle_names[w->particle_count] = name ? name : strdup("?");
        ++w->particle_count;
    }

    w->rects = (rcontext **)calloc((size_t)script->rect_count + 1, sizeof(rcontext *));
    w->rect_names = (char **)calloc((size_t)script->rect_count + 1, sizeof(char *));
    for (i = 0; i < script->rect_count; ++i) {
        const KsStr *d = &script->decl_rects[i];
        rcontext *rc = (rcontext *)calloc(1, sizeof(rcontext));
        char *name = field_str(d->data, d->len, 0, "|");
        rcr_init_rect(rc, &w->root,
            field_atof(d->data, d->len, 1, "|"), field_atof(d->data, d->len, 2, "|"),
            field_atof(d->data, d->len, 3, "|"), field_atof(d->data, d->len, 4, "|"));
        w->rects[w->rect_count] = rc;
        w->rect_names[w->rect_count] = name ? name : strdup("?");
        ++w->rect_count;
    }

    /* parts */
    printf("graph stage: before parts\n");
    w->part_count = script->part_count + 1;
    w->parts = (jpart **)calloc((size_t)script->part_count + 1, sizeof(jpart *));
    for (i = 0; i < script->part_count; ++i) {
        const KsStr *d = &script->decl_parts[i];
        jpart *part = (jpart *)calloc(1, sizeof(jpart));
        int n_fx = 0, n_snd = 0;
        KsStr *fx = ks_split_stag(d->data, d->len, "Fx", &n_fx);
        KsStr *snd = ks_split_stag(d->data, d->len, "S", &n_snd);
        part->nb_effect = n_fx;
        part->effects = (jeffect **)calloc((size_t)n_fx + 1, sizeof(jeffect *));
        for (j = 0; j < n_fx; ++j) {
            jeffect *e = (jeffect *)calloc(1, sizeof(jeffect));
            int n_pa = 0, k;
            KsStr *pa = ks_split_stag(fx[j].data, fx[j].len, "Pa", &n_pa);
            e->nbparam = n_pa;
            e->params = (jparam **)calloc((size_t)n_pa + 1, sizeof(jparam *));
            /* effect name -> code */
            if (n_pa >= 1) {
                char *nm = field_str(pa[0].data, pa[0].len, 0, "|");
                char low[32];
                if (nm) {
                    KsStr t = {(char *)nm, (int)strlen(nm)};
                    lower_trim(&t, low, sizeof low);
                    if (!strcmp(low, "twirl")) e->keffectcode = 1;
                    else if (!strcmp(low, "sprit")) e->keffectcode = 2;
                    else if (!strcmp(low, "tunnel")) e->keffectcode = 3;
                    else if (!strcmp(low, "ground")) e->keffectcode = 4;
                    else if (!strcmp(low, "sprit3d")) e->keffectcode = 5;
                    else if (!strcmp(low, "setrect")) e->keffectcode = 6;
                    else if (!strcmp(low, "flare3d")) e->keffectcode = 7;
                    else if (!strcmp(low, "motionblur")) e->keffectcode = 8;
                    else if (!strcmp(low, "particle")) e->keffectcode = 9;
                    else if (!strcmp(low, "setcamcoord")) e->keffectcode = 10;
                    else if (!strcmp(low, "setcamtarget")) e->keffectcode = 11;
                    else if (!strcmp(low, "evesound")) e->keffectcode = 12;
                    free(nm);
                }
            }
            for (k = 1; k < n_pa; ++k) {
                jparam *p = (jparam *)calloc(1, sizeof(jparam));
                char *kw = field_str(pa[k].data, pa[k].len, 0, "|");
                char low[32];
                KsStr t = {(char *)(kw ? kw : (char *)""), kw ? (int)strlen(kw) : 0};
                lower_trim(&t, low, sizeof low);
                if (!strcmp(low, "ima")) {
                    p->code = PARAM_OBJ; p->obj_kind = 1;
                    { char *nm = field_str(pa[k].data, pa[k].len, 1, "|");
                      int idx = nm ? find_name(w->texture_names, w->texture_count,
                          &(KsStr){nm, (int)strlen(nm)}) : -1;
                      if (idx >= 0) p->obj = w->textures[idx];
                      free(nm); }
                } else if (!strcmp(low, "cos")) {
                    p->code = PARAM_COS;
                    { char *v = field_str(pa[k].data, pa[k].len, 1, "|"); p->a = v?atof(v):0; free(v);
                      v = field_str(pa[k].data, pa[k].len, 2, "|"); p->b = v?atof(v):0; free(v);
                      v = field_str(pa[k].data, pa[k].len, 3, "|"); p->c = v?atof(v):0; free(v); }
                } else if (!strcmp(low, "sin")) {
                    p->code = PARAM_SIN;
                    { char *v = field_str(pa[k].data, pa[k].len, 1, "|"); p->a = v?atof(v):0; free(v);
                      v = field_str(pa[k].data, pa[k].len, 2, "|"); p->b = v?atof(v):0; free(v);
                      v = field_str(pa[k].data, pa[k].len, 3, "|"); p->c = v?atof(v):0; free(v); }
                } else if (!strcmp(low, "cte")) {
                    p->code = PARAM_CTE;
                    { char *v = field_str(pa[k].data, pa[k].len, 1, "|"); p->a = v?atof(v):0; free(v); }
                } else if (!strcmp(low, "rnd")) {
                    p->code = PARAM_RND;
                    { char *v = field_str(pa[k].data, pa[k].len, 1, "|"); p->a = v?atof(v):0; free(v);
                      v = field_str(pa[k].data, pa[k].len, 2, "|"); p->b = v?atof(v):0; free(v); }
                } else if (!strcmp(low, "spl")) {
                    p->code = PARAM_SPL;
                    { char *nm = field_str(pa[k].data, pa[k].len, 1, "|");
                      int idx = nm ? find_name(w->table_names, w->table_count,
                          &(KsStr){nm, (int)strlen(nm)}) : -1;
                      if (idx >= 0) p->obj = w->tables[idx];
                      free(nm);
                      { char *v = field_str(pa[k].data, pa[k].len, 2, "|"); p->a = v?atof(v):0; free(v); } }
                } else if (!strcmp(low, "aff")) {
                    p->code = PARAM_AFF;
                    { char *v = field_str(pa[k].data, pa[k].len, 1, "|"); p->a = v?atof(v):0; free(v);
                      v = field_str(pa[k].data, pa[k].len, 2, "|"); p->b = v?atof(v):0; free(v); }
                } else if (!strcmp(low, "rec")) {
                    p->code = PARAM_REC; p->obj_kind = 4;
                    { char *nm = field_str(pa[k].data, pa[k].len, 1, "|");
                      int idx = nm ? find_name(w->rect_names, w->rect_count,
                          &(KsStr){nm, (int)strlen(nm)}) : -1;
                      if (idx >= 0) p->obj = w->rects[idx];
                      else p->obj = &w->root;
                      free(nm); }
                } else if (!strcmp(low, "par")) {
                    p->code = PARAM_OBJ; p->obj_kind = 3;
                    { char *nm = field_str(pa[k].data, pa[k].len, 1, "|");
                      int idx = nm ? find_name(w->particle_names, w->particle_count,
                          &(KsStr){nm, (int)strlen(nm)}) : -1;
                      if (idx >= 0) p->obj = w->particles[idx];
                      free(nm); }
                } else if (!strcmp(low, "cam")) {
                    p->code = PARAM_OBJ; p->obj_kind = 2;
                    { char *nm = field_str(pa[k].data, pa[k].len, 1, "|");
                      int idx = nm ? find_name(w->camera_names, w->camera_count,
                          &(KsStr){nm, (int)strlen(nm)}) : -1;
                      if (idx >= 0) p->obj = w->cameras[idx];
                      free(nm); }
                } else {
                    p->code = PARAM_CTE; p->a = 0.0;
                }
                free(kw);
                e->params[k - 1] = p;
            }
            ks_str_array_free(pa, n_pa);
            part->effects[j] = e;
        }
        ks_str_array_free(fx, n_fx);
        part->nb_sound_cmd = n_snd;
        part->sounds = (jsndcmd *)calloc((size_t)n_snd + 1, sizeof(jsndcmd));
        /* sound command parsing is deferred (sound is a later milestone) */
        ks_str_array_free(snd, n_snd);
        /* <D> */
        { int n_d = 0; KsStr *dd = ks_split_stag(d->data, d->len, "D", &n_d);
          if (n_d >= 1) { char *v = field_str(dd[0].data, dd[0].len, 0, ","); part->time_length = v?atoi(v):0; free(v);
                          v = field_str(dd[0].data, dd[0].len, 1, ","); part->time_start = v?atoi(v):0; free(v); }
          ks_str_array_free(dd, n_d); }
        w->parts[i] = part;
        amiga_screen_set_progress(75 + (i + 1) * 25 /
            (script->part_count ? script->part_count : 1));
    }
    /* The engine keeps a sentinel part at index NBPart whose TimeLength is the
     * total; kScript.part() compares against it, so it must exist. */
    w->parts[script->part_count] = (jpart *)calloc(1, sizeof(jpart));
    /* kScript converts each part's TimeLength into a cumulative end time. */
    { int n = 0;
      for (i = 0; i < script->part_count; ++i) {
          int dur = w->parts[i]->time_length;
          w->parts[i]->time_length = n;
          n += dur;
      }
      w->parts[script->part_count]->time_length = n;
      w->end_date = n; }
    w->end_pref = script->end_pref;
    printf("graph stage: done\n");
    return w;
}

jpart *jworld_part(jworld *w, jint n) {
    int n3 = 0, n2 = 0;
    while (n3 < w->part_count - 1) {
        if (w->parts[n3 + 1]->time_length >= n && w->parts[n3]->time_length < n) n2 = n3;
        ++n3;
    }
    return w->parts[n2];
}

void jworld_rendah(jworld *w, jpart *part, jint n, int *last_part_played) {
    jint n2 = (n - part->time_length) + part->time_start;
    int i;
    for (i = 0; i < part->nb_effect; ++i) effect_rendah(w, part->effects[i], n2);
    (void)last_part_played;
}

void jworld_frame(jworld *w, jint tick) {
    jpart *part;
    jr_seed(&w->random, 100000u + (uint64_t)tick);
    part = jworld_part(w, tick);
    jworld_rendah(w, part, tick, NULL);
    rcr_create_image(&w->root);
}

uint64_t jworld_hash(jworld *w) {
    uint64_t h = 0xcbf29ce484222325ULL;
    int i, count = w->root.Modulo * w->root.Height;
    for (i = 0; i < count; ++i) {
        h ^= (uint64_t)((uint32_t)w->root.Chunk2[i] | 0xff000000u);
        h *= 0x100000001b3ULL;
    }
    return h;
}

static void free_particle(jparticle *p) {
    if (!p) return;
    free(p->posx); free(p->posy); free(p->posz);
    free(p->Vx); free(p->Vy); free(p->Vz);
    free(p->Ax); free(p->Ay); free(p->Az); free(p->force);
    free(p);
}

void jworld_free(jworld *w) {
    int i, j, k;
    if (!w) return;
    for (i = 0; i < w->texture_count; ++i) { if (w->textures[i]) { free(w->textures[i]->pixels); free(w->textures[i]); } free(w->texture_names[i]); }
    for (i = 0; i < w->table_count; ++i) { if (w->tables[i]) { free(w->tables[i]->datetable); free(w->tables[i]->floattable); free(w->tables[i]); } free(w->table_names[i]); }
    for (i = 0; i < w->camera_count; ++i) { free(w->cameras[i]); free(w->camera_names[i]); }
    for (i = 0; i < w->particle_count; ++i) { free_particle(w->particles[i]); free(w->particle_names[i]); }
    for (i = 0; i < w->rect_count; ++i) { if (w->rects[i]) rcr_free(w->rects[i]); free(w->rect_names[i]); }
    for (i = 0; i < w->part_count - 1; ++i) {
        jpart *part = w->parts[i];
        if (!part) continue;
        for (j = 0; j < part->nb_effect; ++j) {
            jeffect *e = part->effects[j];
            for (k = 0; k < e->nbparam; ++k) free(e->params[k]);
            free(e->params); free(e);
        }
        free(part->effects); free(part->sounds); free(part);
    }
    /* the sentinel part */
    if (w->parts[w->part_count - 1]) free(w->parts[w->part_count - 1]);
    free(w->parts);
    free(w->textures); free(w->texture_names);
    free(w->tables); free(w->table_names);
    free(w->cameras); free(w->camera_names);
    free(w->particles); free(w->particle_names);
    free(w->rects); free(w->rect_names);
    free(w->rcs.OneRec);
    rcr_free(&w->root);
    free(w);
}
