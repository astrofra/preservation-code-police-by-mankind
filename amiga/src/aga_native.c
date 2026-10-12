/* Code Police — native AGA presenter (milestone N2, m68k only).
 *
 * The N2 milestone of documentation/amiga-native-runtime.md §9: show a static
 * frame held as 4-bit indices on a real AGA screen, with no PNG, no 32-bit pixel
 * and no quantisation in the path. It loads the CPIX pack, builds the pair tables
 * once, draws a card (the 16 palette entries plus a few real game textures), C2Ps
 * the index buffer straight to the four bitplanes and presents on vblank.
 *
 * This is deliberately not the final host: it proves the chain index buffer ->
 * C2P -> AGA before the object graph (N4) drives the indexed renderer. It compiles
 * only for m68k-amigaos with -DAGA_SCREEN; on the host it is a stub.
 */
#include "agart.h"
#include "agart_ctx.h"

#include <stdio.h>

#if defined(__m68k__) && defined(AGA_SCREEN)

#include "c2p4.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/displayinfo.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#define SW 320
#define SH 200

static struct Screen *g_screen;
static UBYTE *g_planes[4];
static size_t g_plane_stride;
static UBYTE *g_chunky;

static int open_screen(void) {
    int p;
    g_screen = OpenScreenTags(NULL,
        SA_Left, 0, SA_Top, 0, SA_Width, SW, SA_Height, SH, SA_Depth, 4,
        SA_Title, (ULONG)"Code Police (indexed)",
        SA_ShowTitle, FALSE,
        SA_DisplayID, PAL_MONITOR_ID | LORES_KEY,
        TAG_DONE);
    if (!g_screen)
        g_screen = OpenScreenTags(NULL,
            SA_Left, 0, SA_Top, 0, SA_Width, SW, SA_Height, SH, SA_Depth, 4,
            SA_Title, (ULONG)"Code Police (indexed)", TAG_DONE);
    if (!g_screen) return 0;
    for (p = 0; p < 4; ++p) {
        g_planes[p] = (UBYTE *)g_screen->BitMap.Planes[p];
        if (!g_planes[p]) return 0;
    }
    g_plane_stride = g_screen->BitMap.BytesPerRow;
    g_chunky = (UBYTE *)AllocMem((ULONG)SW * SH, MEMF_CHIP | MEMF_CLEAR);
    return g_chunky != NULL;
}

static void close_screen(void) {
    if (g_chunky) FreeMem(g_chunky, (ULONG)SW * SH);
    if (g_screen) CloseScreen(g_screen);
    g_screen = NULL;
    g_chunky = NULL;
}

/* Draws a recognisable card into the index buffer: the 16 palette entries as
 * bars, then a few real textures through their reachable operators. */
static void draw_card(AgartContext *ctx, const AgartPart *pack, const AgartTables *tables) {
    static const char *names[] = { "titre", "badge2", "space1", "vecball", "care" };
    uint8_t blend[16][16];
    int i;
    agart_blend_build(blend, pack, 128);
    agart_clear(ctx, AGART_BLACK_INDEX);
    for (i = 0; i < 16; ++i) {
        AgartRect bar;
        bar.x1 = i * 20;
        bar.x2 = bar.x1 + 18;
        bar.y1 = 0;
        bar.y2 = 40;
        agart_clear_rect(ctx, &bar, (uint8_t)i);
    }
    for (i = 0; i < 5; ++i) {
        const AgartTexture *t = agart_find(pack, names[i]);
        AgartUV uv[4];
        int x = 8 + (i % 3) * 104;
        int y = 48 + (i / 3) * 76;
        if (!t) continue;
        uv[0].u = 0; uv[0].v = 0;
        uv[1].u = t->width << 16; uv[1].v = 0;
        uv[2].u = 0; uv[2].v = t->height << 16;
        uv[3].u = t->width << 16; uv[3].v = t->height << 16;
        ctx->rect.x1 = x; ctx->rect.y1 = y;
        ctx->rect.x2 = x + 96 > SW ? SW : x + 96;
        ctx->rect.y2 = y + 72 > SH ? SH : y + 72;
        if (t->role == AGART_ROLE_ADDITIVE) agart_blit_add(ctx, t, uv, tables);
        else agart_blit_alpha(ctx, t, uv, blend);
    }
}

int aga_native_run(const char *pack_path) {
    AgartPart *pack;
    AgartTables tables;
    uint8_t *buffer;
    AgartContext ctx;
    int frames;
    FILE *m;
    /* Prefer the pack linked into the image (N1b): then no asset is opened at
     * start-up. Fall back to the file when it was built without EMBED=1. */
    pack = agart_load_embedded();
    if (!pack) pack = agart_load(pack_path);
    if (!pack) { printf("native: cannot load %s\n", pack_path); return 1; }
    buffer = (uint8_t *)AllocMem((ULONG)SW * SH, MEMF_ANY | MEMF_CLEAR);
    if (!buffer) { agart_free(pack); return 1; }
    agart_tables_build(&tables, pack);
    ctx.pixels = buffer;
    ctx.previous = NULL;
    ctx.width = SW;
    ctx.height = SH;
    ctx.clip.x1 = ctx.clip.y1 = 0;
    ctx.clip.x2 = SW;
    ctx.clip.y2 = SH;
    ctx.rect = ctx.clip;
    draw_card(&ctx, pack, &tables);

    if (!open_screen()) { FreeMem(buffer, (ULONG)SW * SH); agart_free(pack); return 1; }
    LoadRGB4(&g_screen->ViewPort, (UWORD *)pack->palette, 16);
    /* The card is static: C2P once, then hold it on screen and let the user quit
     * with Ctrl-C. A short bounded loop keeps the screen refreshing under the
     * emulator and proves the presentation path is stable. */
    m = fopen("CP:ran.txt", "w");
    if (m) { fprintf(m, "aga native: pack=%s textures=%d\n", pack_path, pack->count); fclose(m); }
    for (frames = 0; frames < 250; ++frames) {
        if (SetSignal(0L, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) break;
        c2p4_byte4(buffer, SW, SH, SW, (uint8_t **)g_planes, g_plane_stride);
        WaitTOF();
    }
    m = fopen("CP:ran.txt", "w");
    if (m) { fprintf(m, "aga native: presented %d frames\n", frames); fclose(m); }
    close_screen();
    FreeMem(buffer, (ULONG)SW * SH);
    agart_free(pack);
    return 0;
}

#else
/* Host build: there is no AGA screen. */
int aga_native_run(const char *pack_path) { (void)pack_path; return 0; }
#endif
