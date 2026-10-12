/* Code Police — AmigaOS AGA screen host (m68k only).
 *
 * The real display path: open a 320x200 four-bitplane AGA screen, render the
 * engine's frames, downscale them to the screen, quantise to the part's
 * 16-colour palette, convert to bitplanes with c2p4 and present on vblank. This
 * is the port's §7 host; it compiles only for m68k-amigaos, so the host build
 * of the differential checks is unaffected.
 */
#include "objects.h"

#if defined(__m68k__) && defined(AGA_SCREEN)

#include "aga_quant.h"
#include "aga_palette.h"
#include "c2p4.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/displayinfo.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#define SW 320
#define SH 200
#define SBPL (SW / 8)

static struct Screen *g_screen;
static UBYTE *g_planes[4];
static UBYTE *g_chunky;
static UBYTE *g_indices;
static uint8_t g_lut[AGA_QUANT_LUT_SIZE];

static int open_screen(void) {
    int p;
    g_screen = OpenScreenTags(NULL,
        SA_Left, 0, SA_Top, 0,
        SA_Width, SW, SA_Height, SH,
        SA_Depth, 4,
        SA_Title, (ULONG) "Code Police",
        SA_ShowTitle, FALSE,
        SA_DisplayID, PAL_MONITOR_ID | LORES_KEY,
        TAG_DONE);
    if (!g_screen) {
        /* Fall back to whatever lowres mode Intuition chooses. */
        g_screen = OpenScreenTags(NULL,
            SA_Left, 0, SA_Top, 0, SA_Width, SW, SA_Height, SH, SA_Depth, 4,
            SA_Title, (ULONG) "Code Police", TAG_DONE);
    }
    if (!g_screen) return 0;
    for (p = 0; p < 4; ++p) {
        g_planes[p] = (UBYTE *)AllocMem((ULONG)SBPL * SH, MEMF_CHIP | MEMF_CLEAR);
        if (!g_planes[p]) return 0;
    }
    g_chunky = (UBYTE *)AllocMem((ULONG)SW * SH, MEMF_CHIP | MEMF_CLEAR);
    g_indices = (UBYTE *)AllocMem((ULONG)SW * SH, MEMF_ANY | MEMF_CLEAR);
    return g_chunky && g_indices;
}

static void set_palette(int part) {
    UWORD colours[16];
    int i;
    for (i = 0; i < 16; ++i) {
        const uint8_t *c = AGA_PALETTES[part][i];
        colours[i] = (UWORD)(((c[0] >> 4) << 8) | ((c[1] >> 4) << 4) | (c[2] >> 4));
    }
    LoadRGB4(&g_screen->ViewPort, colours, 16);
}

/* Nearest-neighbour 520x300 -> 320x200 into the chunky buffer. */
static void downscale(const uint32_t *argb, int part) {
    int x, y;
    aga_quant_build_lut(AGA_PALETTES[part], g_lut);
    for (y = 0; y < SH; ++y) {
        int sy = y * 300 / SH;
        for (x = 0; x < SW; ++x) {
            int sx = x * 520 / SW;
            uint32_t p = argb[sy * 520 + sx];
            unsigned cell = (((p >> 19) & 31u) << 10) | (((p >> 11) & 31u) << 5) |
                            ((p >> 3) & 31u);
            g_chunky[y * SW + x] = (UBYTE)(g_lut[cell] & 0x0Fu);
        }
    }
}

static void present(void) {
    c2p4_byte4(g_chunky, SW, SH, SW, (uint8_t **)g_planes, SBPL);
    WaitTOF();
    LoadView(&g_screen->ViewPort);
}

static void close_screen(void) {
    int p;
    for (p = 0; p < 4; ++p) if (g_planes[p]) FreeMem(g_planes[p], (ULONG)SBPL * SH);
    if (g_chunky) FreeMem(g_chunky, (ULONG)SW * SH);
    if (g_indices) FreeMem(g_indices, (ULONG)SW * SH);
    if (g_screen) CloseScreen(g_screen);
}

int amiga_screen_run(jworld *w) {
    int part_index, prev_part = -1, tick;
    if (!open_screen()) { close_screen(); return 1; }
    for (tick = 0; ; tick += 2) {
        jpart *part;
        int i;
        if (SetSignal(0L, SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) break;
        jworld_frame(w, (jint)tick);
        part = jworld_part(w, (jint)tick);
        part_index = 0;
        for (i = 0; i < w->part_count - 1; ++i)
            if (w->parts[i] == part) part_index = i;
        /* The palette is rebuilt only when the scene changes. */
        if (part_index != prev_part) { set_palette(part_index); prev_part = part_index; }
        downscale((const uint32_t *)w->root.Chunk2, part_index);
        present();
    }
    close_screen();
    return 0;
}

#else
/* Host build: the screen host does not exist. */
int amiga_screen_run(jworld *w) { (void)w; return 0; }
#endif
