# Feasibility study — Code Police on Amiga AGA (68060)

Date: 2026-10-11. Scope: an analytic feasibility study for re-hosting the
preserved Code Police renderer on an Amiga with the AGA chipset and a 68060.
Target as agreed: **320×200 maximum, 16 colours, a fixed palette per scene
derived from the original C++ output, and the bulk of the runtime written in
assembly.** No Amiga hardware or emulator was used, and nothing was compiled
for m68k.

**Method and disclaimer.** Every number below is derived from the repository's
own evidence — the transcribed engine, the asset inventory and the browser
measurements — plus published Amiga hardware figures. The performance figures
are *estimates from a cycle model*, not measurements. They exist to show the
order of magnitude and to identify which decisions matter.

## 1. Verdict

**Feasible with margin, and this target is the right one.** The earlier concern
was per-pixel rasterisation time; the reduced target removes it. Halving the
linear resolution cuts the pixel count by 2.44×, and dropping to 16 colours
halves the bitplane conversion and — more importantly — lets the per-pixel
colour maths collapse into table lookups. The result is a port that should run
at or above the original's measured ~42 fps, not below it.

Four decisions carry the port:

1. **320×200, 4 bitplanes.** 64,000 pixels per frame instead of 156,000, and a
   C2P that converts four planes instead of eight.
2. **One fixed 16-colour palette per scene, chosen offline** by quantising the
   raw ARGB frames the C++ engine already produces headlessly. Palette *entry
   animation* then covers fades and tints, which is how 16-colour modes have
   always handled them.
3. **Indexed rendering.** The renderer must work directly in palette indices, not
   convert ARGB at present time. This is both the fast path and the only way the
   colour blends stay coherent at 16 colours.
4. **Assembly for the hot core, C++ for everything else.** The ASM rewrite is
   bounded by the existing deterministic reference, which can validate it
   headlessly — see §7.

## 2. What is being ported

Facts taken from this repository.

| Property | Value | Source |
| --- | --- | --- |
| Source framebuffer | 520 × 300 = 156,000 px | `native-sdl2/src/engine.hpp` |
| Source pixel format | 32-bit ARGB, `0xFF000000 \| rgb` | `native-sdl2/src/engine.cpp` |
| Framebuffer model | double buffer, swapped by pointer | `kRenderContext::createImage` |
| Rasterisation unit | axis-aligned 16×16 blocks, affine UV | `raycasting::draw` |
| Mapping arithmetic | 16.16 fixed point, shift/mask texel lookup | `drawrecUVRGB`, `drawrecUVSA` |
| Per-block 3D maths | `sqrt`/`atan2`/`sin`/`cos` per 16×16 block | `findfloor`, `findtunnel`, `findtwirl` |
| Textures | 53 images, 1.89 Mpix total | `evidence/diagnostic.json` |
| Particles | three systems of 300, 300 and 32 | `assets/scene.txt` (`KTICLE`) |
| Script | 44,195 bytes, 12 parts, 222 s loop | `assets/scene.txt` |
| Part boundaries | 0, 4, 14, 24, 34, 39, 42, 62, 78, 92, 108, 122 s | `evidence/diagnostic.json` |
| Audio | 10 AU files, mono 8 kHz μ-law, 556 KiB | `assets/Sounds` |
| Measured source rate | 9,407 frames / 222 s ≈ 42 fps | `evidence/wasm/live-playback.json` |

Two facts matter most. First, **the resolution is a single value**: `520×300`
appears only in `engine.hpp` as `getSize()` and in the SDL hosts. The scene
script carries no resolution, so a 320×200 build needs no script change —
confirmed by searching the whole tree. Second, **the renderer writes a linear
chunky buffer in 16×16 blocks**, which is exactly the input shape a
chunky-to-planar routine consumes.

## 3. Target definition

**Display.** 320 × 200, 4 bitplanes (16 colours). This is a standard AGA lowres
mode; the chip bus load is negligible compared with the 640×480 8-plane ceiling.

**Aspect ratio, explicitly.** The source is 1.733:1. Amiga lowres pixels are not
square — on PAL a 320×200 screen presents at roughly 1.6:1. Three options:

- accept 320 × 200 with a slight horizontal stretch;
- use **320 × 184** (58,880 px, 1.739:1) to match the source and let the display's
  vertical centring provide the rest;
- keep 320 × 200 and letterbox a 320 × 185 image area.

The difference is a few thousand pixels and no structural change. The
recommendation of "320×200 maximum" is satisfied by all three; the aspect
decision is artistic and can be made from a side-by-side capture.

**Borders.** The original 640×480 cinema frame of four static JPEG borders does
not survive the resolution change. The natural replacement is a static AGA
border in chip RAM: either pre-converted bitplane strips, or the demo rendered
into a 320×200 window on a 640×256 screen. Either way the borders are written
once, not per frame, so they cost no bus time.

**Chip RAM budget.** 320 × 200 × 4 bits = 25,600 bytes per screen. Double
buffering plus borders is well under 100 KiB — a rounding error on any A1200.

## 4. Colour: one palette per scene

This is the part that makes or breaks the port artistically, and it has a clean
answer.

### 4.1 Where quantisation happens

The engine computes true-colour ARGB per pixel and blends in RGB. Converting
ARGB to an index at present time would cost a lookup — and a nearest-colour
search — for every one of the 64,000 pixels, which is the expensive option.

The port should still **make the renderer indexed**: textures are converted to
palette indices once at load, and the common opaque path becomes `dst[i] =
tex[uv]`. But an earlier revision of this section listed the wrong operators.
The table below was rebuilt by enumerating every function that writes the
framebuffer and every site that calls one, **which the §9 risk list made a
prerequisite for the port**. Two methods the first draft relied on,
`drawrecUV` and `drawrecUVL`, are declared but **never called anywhere** in the
engine; the paths that actually run are `drawrecUVRGB`, `drawrecUVSA` (which
falls through to `drawrecUVS`) and `drawrecUVM`.

| Engine operation | Called by | Current ARGB code | Indexed equivalent |
| --- | --- | --- | --- |
| `drawrecUVS` | `drawrecUVSA` (alpha 255) | opaque texel copy, index −1 transparent | `dst[i] = tex[uv]`, one compare |
| `drawrecUVSA` | `Setzoom3D`, `SetzoomP` | alpha `n`/255 then `drawrecUVS`: `(dst*(255−n) + src*n) >> 8` per channel | `dst[i] = blend[src][dst]` per call |
| `drawrecUVRGB` | `draw()` (Sprit, Sprit3D) | `clamp(tex_ch + dst_ch)` per channel, clamped 0–255 | `dst[i] = add[src][dst]`, palette-dependent |
| `drawrecUVM` | `flare3D()` (particles, flares) | `clamp(tex_ch + dst_ch)`, clamped 255 only | `dst[i] = add255[src][dst]`, palette-dependent |
| `findtwirl` colour | `findtwirl()` (twirl scenes) | per block `r = n*26`, `g = n*24`, `b = n*23` into `kPixel` | quantise once per 16×16 block |
| `MotionBlur` | `MotionBlur` effect | `((a & 0xFCFCFF) + (b & 0xFCFCFF)) >> 1` | `dst[i] = mix[a][b]`, palette-dependent |
| `drawrecUV` | — **never called** | texel copy | not required |
| `drawrecUVL` | — **never called** | add scalar to R,G,B, clamp | not required |

`findtwirl` is the odd one out: it does not touch the framebuffer at all. It
writes a shading colour into each `kPixel` of a render context, and that colour
is later consumed by `drawrecUVRGB` as the additive source. In the indexed model
its three multiplications become one quantisation per 16×16 block, and the
result is a palette index feeding the pair table below.

**What actually changed from the first draft.** The first draft's headline win
was `drawrecUVL`'s "ramp" table, replacing a scalar add and three clamps with one
byte lookup. That method is **dead code**, so that specific win does not exist.
What replaces it is better in coverage and the same in kind: every reachable
per-pixel colour operator reduces to a small pair table over palette indices,
because in the indexed model **both operands are 16-valued palette indices** —
the framebuffer holds indices, and textures are quantised to the same palette.
`drawrecUVSA`'s blend is such a table for the call's constant alpha; `MotionBlur`,
`drawrecUVRGB` and `drawrecUVM` are pair tables of `clamp(a + b)`, 256 bytes each.
`drawrecUVS` needs no table at all, just a copy and a transparent-index compare.

So the indexed design holds for all four loops. The correction is not that it
fails, but that the three pair tables are **palette-dependent**: they must be
rebuilt whenever a scene's palette changes, not merely when a fade level changes
as the first draft said. Twelve rebuilds per cycle of 256-byte tables is nothing,
and each fits inside one vblank. `drawrecUVSA`'s alpha `n` is constant across a
call, so one 256-byte table per call suffices; there is no need for a
256 × 16 × 16 table across all alpha levels.

The residual caution is fidelity, not performance. The pair tables quantise
`clamp(palette[i] + palette[j])` back to the nearest palette entry, so an additive
sprite can land on a different index than a true-colour render would choose. That
is inherent to 16 colours and is what the §13 error measurement already covers.

### 4.2 Deriving the palettes

The raw material already exists. `native-sdl2/tools/verify.cpp` runs the engine
headlessly and dumps frames as PPM at chosen ticks; the reference captures under
`reference/native/` are the same output. The procedure:

1. Dump every 4th tick across each of the 12 parts (part boundaries are exact,
   from `evidence/diagnostic.json`), giving a few hundred frames per scene.
2. Accumulate a colour histogram per part, **weighted by pixel frequency**, with
   a small bonus for rare saturated colours so sprite accents and title glows are
   not dropped.
3. Run median-cut (or k-means seeded from median-cut) down to 16 colours.
4. Store one 16-entry palette per part — 12 palettes, 384 bytes total (16 × 12 ×
   2 bytes, since AGA colour registers are 12-bit words).
5. Verify by re-rendering the sampled frames, quantising to the palette, and
   measuring mean and maximum per-channel error against the ARGB source. The
   repository already depends on Pillow for screenshot comparison
   (`wasm/check-presentation.py`), so the tooling for this check is in place.

Twelve palettes is a deliberate choice over one global palette: the script has
genuinely different looks (title, tunnels, particles, the 100-second finale) and
per-scene palettes cost nothing to switch.

**This procedure was executed; see [§13](#13-step-1-results--palette-study-executed).**

### 4.3 Fades and tints

A fixed palette cannot darken a colour that is not in it. The classic solution
applies here: during a fade, keep the *indices* fixed and animate the *palette
RGB values*. `helpfade` and `colorclip` already compute a scalar fade amount, so
the port can scale all 16 RGB triples toward black or a tint — 48 multiply-adds
in the vertical-blank window, which is free. The Copper writes all 16 colour
registers as a single list, so there is no tearing and no per-pixel cost.

If a scene needs a fade that is not a simple global scale, the alternative is to
bake N intermediate palettes (for example 32 levels) and switch between them.
That is 32 × 32 bytes per scene and is still negligible.

## 5. Chunky-to-planar at 4 bitplanes

The conversion now produces four bitplane words per 16 pixels instead of eight,
which roughly halves both the table lookups and the register traffic of the
8-plane case. The budget, at 320 × 200 = 64,000 pixels:

| C2P cost | Cycles per frame | Time at 50 MHz |
| --- | --- | --- |
| 1.5 cycles/pixel | 96,000 | 1.9 ms |
| 3 cycles/pixel | 192,000 | 3.8 ms |
| 6 cycles/pixel | 384,000 | 7.7 ms |

Kalms' 68060-tuned routine sits at the top of that table; the 4-plane case is
the variant these routines are best optimised for, since 320×200/16 is the
canonical AGA demoscene target. A simple routine such as McGavin's is adequate
for bring-up at ~6 cycles/pixel.

**A 4-plane Kalms adaptation already exists.** The upstream Kalms source ships
`c2p1x1_4_c5_bm.s` (4 planes, Public Domain) under `bitmap/` in
[Kalmalyzer/kalms-c2p](https://github.com/Kalmalyzer/kalms-c2p), so the 4-plane
case does not have to be written from scratch. A sibling Amiga project,
OS-MAGI-80, carries a ready adaptation: `src/graphics/c2p4_kalms.S` converts a
byte-per-pixel chunky buffer to four separate bitplanes, keeps the upstream
non-modulo transpose and pipelined stores, and changes only the assembler
syntax (GNU), the calling convention (C ABI), and the plane/stride pointers. Its
upstream original and provenance are vendored unchanged next to it. Its contract
is exactly this port's target: **source bytes must be colours 0..15 and width
must be a non-zero multiple of 32**. It is exercised by byte-exact host vectors
and by real 68020 assembly loops under the Musashi emulator, and it is the
default converter in that project. Two further backends accompany it (a pair-LUT
and a table-free five-stage merge) if Kalms ever needs replacing.

**Options, briefly.** Blitter-only is still not viable: the blitter cannot
transpose bitplanes, so it would need four masked passes and it steals the chip
bus. Akiko is CD32-only and not present on an A1200/A4000. Kalms remains the
recommended choice, with the 68060 variant.

## 6. Performance budget

With indexed rendering the inner loop is a load, a table lookup and a store.
Reasonable 68060 figures, caches warm and the texture resident:

| Inner loop | Cycles/pixel | Notes |
| --- | --- | --- |
| `drawrecUVS` opaque | 4–6 | shift, mask, compare, load, store |
| `drawrecUVSA` alpha blend | 6–8 | one per-call 256-byte table |
| `drawrecUVRGB` / `drawrecUVM` additive | 4–6 | one pair-table lookup, same cost as an opaque copy |
| `MotionBlur` via mix LUT | 8–10 | two loads plus one lookup |
| Worst observed mix | ~12 | |

The additive paths cost no more than the opaque copy in the indexed model,
because they too are a single pair-table lookup (§4.1); the pair tables'
palette dependence is a rebuild cost at scene changes, not a per-pixel cost.

Full-frame, fully-covered estimates:

| Scene character | Raster | C2P | Total | Estimate |
| --- | --- | --- | --- | --- |
| Title / sprites, ~25 % fill | 1–2 ms | 2–4 ms | 3–6 ms | 150+ fps |
| Mixed, ~60 % fill | 3–5 ms | 2–4 ms | 5–9 ms | 100+ fps |
| Full-screen ground/tunnel, opaque | 5–8 ms | 3–4 ms | 8–12 ms | ~80 fps |
| Full-screen, heavy blend | 8–12 ms | 3–4 ms | 11–16 ms | ~60 fps |

Even allowing for the object/script CPU work (which is small: 627 blocks of 3D
maths, particles in the hundreds, a 44 KiB script walked once per frame), a
**stable 50 fps is realistic and 42 fps — the original rate — is comfortable**.

The practical limit becomes the display, not the CPU. The port should present on
a vertical blank and not run faster than that, which also keeps the 10 ms script
ticks and the 222-second cycle intact.

## 7. Assembly strategy

Writing "the bulk in assembly" is the right instinct for this target, but it
matters where the line is drawn.

**Assembly, without question:**

- **C2P.** Register allocation and `ror`/`and`/`or` chains are the whole point of
  the Kalms routines; this is ASM by definition.
- **Inner raster loops** — `drawrecUVS`/`drawrecUVSA`, `drawrecUVRGB`,
  `drawrecUVM` and `MotionBlur`, plus the particle and flare blits. These are
  load/lookup/store loops where a C compiler emits register spills that cost more
  than the loop body. The two additive loops need the per-scene table of §4.1.
- **The audio mixer.** Four-channel Paula output driven from an 8 kHz mono
  software mix is short and entirely register work.
- **Copper list and palette writes**, including the fade animation.
- **The blitter setup** for the static borders.

**C++ is the sane choice for:**

- **The script parser and object graph.** 44 KiB of tagged text, hashtables,
  parameters, spline tables, particle definitions. This is cold code executed
  once at load; rewriting it in ASM buys nothing and risks the correspondence
  with the verified original.
- **Camera and particle mathematics.** Doubles, `sqrt`, `atan2`, `sin`, `cos`
  per block or per particle. The 68060 FPU handles this well and the volume is
  small. Fixed-point conversion is available later if profiling justifies it.
- **The host and startup path.**

**The decisive enabler.** The repository already has a headless oracle: the
deterministic replay path (`cp_test_frame` in the browser host, `verify.cpp`
natively) compares 11,101 frame hashes, numeric states and PCM buffers against
the original Java bytecode. A hand-written ASM raster core can therefore be
validated **against the verified C++ engine, without a screen**, frame by frame,
on a workstation or emulator. That converts the usual ASM risk (subtle visual
bugs found late) into an exact diff at a known tick.

Two stages are needed because quantization is intentionally lossy:

1. **Raster core in ARGB.** ASM primitives must reproduce the C++ engine's ARGB
   hashes bit-for-bit. This validates the affine maths, clipping and block
   edge handling exactly.
2. **Indexed stage.** Separately validated by comparing quantised output to an
   offline quantisation of the same reference frames, using the tolerance-based
   checks the repository already performs.

Keeping the verified `engine.cpp` compilable and serving as the oracle for the
ASM core is the single most valuable process decision in this port.

## 8. What must change

**Presentation host (rewrite, small).** The SDL2 hosts (`wasm/src/main.cpp`,
`native-sdl2/src/main.cpp`) have no counterpart on AmigaOS classic. Replace with
`OpenScreen`/`OpenWindow` at 320×200, a Copper list for the palette, `IDCMP` for
input and quit, and a vertical-blank wait. A small framework such as ACE or a
hand-written `graphics.library` layer fits; the engine does not care which.

**This host layer already exists, in three-plane form.** The preserved Karate
source archive (`original/karate_fighter/`, see §16) contains
`karateSRC/objects/ChunkyStartup.asm`, a linkable 68020+ module that opens an
8-bit AGA (or Cybergraphics) screen, runs a triple buffer, provides
`Allocatechunkyforscreen()`, `ScreenRefresh(UBYTE*)`, `ListenEnd()` (mouse
buttons or Escape to quit) and a stable 50 Hz clock from `timer.device`, and is
documented as MuForce-clean and multitasking-legal. `ScreenRefresh` is where its
C2P call lives; the module is written to call a Kalms C2P by function pointer, so
the converter is a link-time choice, not a hard dependency. Its depth is
hard-coded to 8 planes, so a 4-plane port changes that constant and the C2P
entry, and reuses the rest. It is written for phxass syntax rather than vasm, and
its public interface is vbcc's `__reg()` style, both of which are mechanical
changes.

**Pixel format and the indexed conversion.** This is the substantive change.
The engine's `Array<jint>` pixel buffers, the ARGB constants (`0xFF000000 | …`)
and the per-pixel RGB maths must become 8-bit indices plus lookup tables, as in
§4.1. If the goal is to preserve `engine.cpp` exactly, the alternative is to
keep it as the oracle and write a parallel indexed renderer in ASM — which is
precisely the two-stage plan in §7 and is the recommended route.

**C++ runtime.** This was the largest hidden cost if any engine code is compiled.
It was measured directly (§16) by compiling the real `engine.cpp` for
`m68k-amigaos`, `-m68060 -std=c++11 -O2`:

| Feature used | Measured status on the m68k toolchain | Mitigation |
| --- | --- | --- |
| `std::shared_ptr` (`Ref<T>`) everywhere | Compiles and links | Non-atomic intrusive pointer behind the `Ref<T>` typedef if refcount traffic shows up |
| Exceptions (`throw std::runtime_error`) | Compiles with `-fexceptions` | Keep, or a fatal handler for the few sites |
| `std::function` in `Sound::event` | Compiles and links | Function pointer plus user data if it profiles hot |
| `std::mutex` in the mixer | **Does not compile** — the one hard failure | No threads in the host; compile the lock to a no-op (§16) |
| `std::sqrt`/`atan2`/`sin`/`cos` | Compiles (soft-float by default; FPU with the right flags) | Keep; move to fixed point only if profiled |
| `std::chrono` | Not the desktop API | `timer.device`; `clockOffset` already models suspend/resume |
| `<fstream>` | Compiles and links | Embedded resource blob or `dos.library` load |

The AmigaPorts `m68k-amigaos-gcc` (the maintained name for what this study
earlier called bebbo-amiga-gcc) is the established toolchain and is already
installed here; see §11.3.

**Assets.** 65 external files become an embedded blob or a disk load. At 16
colours the decoded texture set is about **1.8 MiB as 8-bit chunky** if every
texture is kept at full source resolution — but the target is 320×200, so
textures above the screen size can be reduced, and the working set is far
smaller. 1.8 MiB of Fast RAM is the worst case and remains the reason an
accelerator with at least 8 MiB is the realistic floor.

**Audio.** The engine mixes every active sound into one mono signed 16-bit
stream at 8 kHz and clamps it. Paula gives four 8-bit DMA channels; the faithful
mapping is a software mix to two 8-bit channels driving two Paula channels,
leaving two free. μ-law decoding already exists and is unchanged.

**Timing.** `kTimer` counts 10 ms ticks, and the script is 222 seconds long. A
`timer.device` or CIA time base is sufficient, and the existing `clockOffset`
mechanism already supports suspend/resume.

## 9. Risks and unknowns

| Risk | Severity | Note |
| --- | --- | --- |
| Indexed conversion breaks a per-pixel colour effect not listed in §4.1 | **Realised, resolved** | The enumeration was done (§15): the additive operators `drawrecUVRGB` and `drawrecUVM` were missing from §4.1. They do reduce to a palette-index pair table, but one that must be rebuilt on palette change, so §4.1 was corrected. No blocker. |
| Palette quality on photographic textures at 16 colours | Medium | Measured in §13; per-scene palettes keep the mean error near 10/255 |
| Fades that are not a global scale | Low | Bake intermediate palettes |
| Aspect ratio dissatisfaction | Low | Decide from a side-by-side capture; 320×184 is available |
| ASM maintenance and regression | Medium | Bounded by the headless oracle (§7) |
| No Amiga hardware for verification | Process | Emulator first, real machine before any published performance claim; AGA bus contention and 68060 timing are where emulation is approximate |

One caution on expectations: 16 colours will not look like the source. The
artwork is photographic and the source blends in true colour. The right
comparison is not "does it match the C++ output" but "is the chosen 16-colour
look a deliberate, good-looking 16-colour look" — which is a judgement to make
from captures, scene by scene, at the palette-selection stage.

## 10. Staged plan

1. **Palette study first.** Using the existing headless PPM export, generate the
   12 palettes and render side-by-side comparisons against the source frames.
   This is cheap, needs no Amiga, and decides whether the artistic target is
   acceptable before any port work begins. **If this step fails, the port should
   not proceed.** — **Executed; see [step 1 results](#13-step-1-results--palette-study-executed).**
2. **Colour-path enumeration.** Enumerate every framebuffer and `kPixel` writer
   and every call site, so the indexed design is closed before code is written.
   — **Executed; see [§15](#15-colour-path-enumeration-executed).** It corrected
   §4.1 and reduced the raster core to four loops.
3. **ASM raster core in ARGB**, validated bit-for-bit against `verify.cpp`'s
   hashes. No Amiga required. Implement only the four reachable writers of §15.
   The toolchain for this is installed and the engine already compiles for
   `m68k` (§11.3, §16.3).
4. **C2P bring-up** at 320×200 4 planes; get the picture on an AGA screen and
   confirm the display path end to end. Adopt the four-plane Kalms adaptation
   of §16.2 rather than writing a converter.
5. **Swap in the 68060 pipeline** and measure at a fixed scene.
6. **Indexed renderer and palette switching** per part, with the Copper fade
   animation and the per-scene additive table of §4.1.
7. **Host integration**: screen, copper, input, quit, assets, audio. Adapt the
   Karate presenter of §16.1 from 8 planes to 4.
8. **Measure on real hardware** and optimise only the slowest part, expected to
   be the metaball particle pass in part 11.

Steps 1 to 3 decide the project, and all three can be completed before any
Amiga-specific host code exists. Steps 1 and 2 are done and required no
toolchain; step 3 now has its toolchain present.

## 11. Host tooling: macOS Apple Silicon

The development machine is a Mac (Apple Silicon, `arm64`, macOS 14.1) with
Homebrew 6.0.21. This section records what is already present and what the port
would need on top of it. Everything below was checked on this machine; nothing
here is recalled from memory.

### 11.1 What is already in place

| Tool | State | Notes |
| --- | --- | --- |
| Homebrew | 6.0.21 | `arm64` prefix at `/opt/homebrew` |
| FS-UAE | **installed**, formula `fs-uae` 3.2.35, native `arm64` | CLI at `/opt/homebrew/bin/fs-uae` |
| Python | 3.14.7 with pip 26.2.1 | system/Homebrew |
| Pillow | 12.1.0 | already a repository dependency for screenshot checks |
| numpy | 2.3.5 | available for histogram and quantisation work |
| git | Apple Git 2.39.3 | for the toolchain sources below |
| `lha` | **installed**, Homebrew | unpacks the preserved Karate `.lha` archives |
| `amitools` 0.8.1 (`xdftool`, `rdbtool`), `vamos` | **installed** via pipx under `$HOME/.local/bin` | ADF/HDF editing and headless AmigaOS API execution |

FS-UAE being a **Homebrew formula rather than a cask** is convenient: it is a
native arm64 binary and it is fully scriptable from the command line, which is
what the automated verification path in §7 wants. There is no GUI launcher
installed and none is required. `~/Documents/FS-UAE/` already exists with the
standard tree (`Kickstarts`, `Hard Drives`, `Floppies`, `Configurations`,
`Save States`, ...), so the ROM path below is conventional, not invented.

`amitools` and `vamos` matter twice over: they create and edit ADF/HDF images
from Python and run AmigaOS CLI programs without booting a full emulator, which
is what a headless assemble-inject-boot-compare loop needs, and they are the
same tools the sibling OS-MAGI-80 project standardised on (§16).

**Kickstart ROMs.** FS-UAE ships only the free AROS ROM
(`aros-amiga-m68k-rom.bin`); no copyrighted Kickstart is bundled. The 3.0 and
3.1 ROMs the user holds must be placed in `~/Documents/FS-UAE/Kickstarts/`.
FS-UAE scans that directory and identifies ROMs both by conventional filename
and by checksum, so `kick31.rom` is the name to use for the A1200/AGA path; it
is also acceptable to let the emulator pick by checksum.

### 11.2 Emulator setup

FS-UAE 3.2.35 exposes exactly what this port needs. Its binary advertises the
models `A500, A500+, A600, A1000, A2000, A3000, A4000, A1200, CD32, CDTV`, the
CPU values `68000 ... 68060`, and — importantly — the bundled accelerator
presets **Blizzard 1230 IV, Blizzard 1240, Blizzard 1260 and CyberStorm PPC**,
alongside options named `accuracy`, `fast_memory`, `chip_memory`,
`zorro_iii_memory`, `fpu_model` and `floppy_drive_0`. A configuration along
these lines targets the study's hardware:

```ini
[fs-uae]
amiga_model = A1200
accelerator = Blizzard 1260   ; 68060 with FPU
accuracy = -1                 ; automatic; set 1 to force cycle-exact
kickstart_file = kick31.rom
chip_memory = 2
fast_memory = 8
floppy_drive_0 = CodePolice.adf
```

Two caveats, stated plainly:

- **`Blizzard 1260` gives the CPU and FPU, not the cycle budget.** An emulator
  does not reproduce a real 68060 pipeline or AGA bus contention. Fine for
  correctness, misleading for frame rate — which is why §9 still calls for a
  real machine before any performance claim.
- **There is no `68060`-aware build of FS-UAE on Homebrew.** 3.2.35 is what
  installs, and it is adequate. Building FS-UAE from source for macOS arm64 is
  possible if a specific WinUAE/UAE version is ever needed, but it is not a
  prerequisite.

### 11.3 Cross-toolchain: present and validated

The hot core is assembly and the rest is C++, so two toolchains are needed.
**Both are already installed on this machine, outside `PATH`**, and the C++
side has been validated by compiling the real engine. This section supersedes
the earlier "the real gap" wording; the gap is closed.

A maintained **AmigaPorts `m68k-amigaos-gcc`** suite (the same toolchain this
study earlier called bebbo-amiga-gcc, which is one of its ancestors) lives under
`$HOME/.local/m68k-amigaos`, built from a source tree at
`$HOME/dev/toolchains/m68k-amigaos-gcc`. Add its `bin` to `PATH` to use it. It
provides, all verified by running them:

| Component | Version | Use here |
| --- | --- | --- |
| `m68k-amigaos-gcc` / `-g++` | 16.2.0b | compiles the C++ engine and a future host |
| `m68k-amigaos-{ld,as,nm,objdump,size,strip}` (binutils) | 2.46 | links AmigaOS hunk executables; disassembly and size inspection |
| `vasmm68k_mot` | 2.0b | the `68060` assembler for the C2P and the raster cores |
| `vobjdump`, `gdb`, `lha`, `fd2pragma`, `sfdc` | bundled | object inspection, debugging, archives, headers |

The NDK (3.2) headers are under `$HOME/.local/m68k-amigaos/m68k-amigaos/`,
including `ndk-include/` (the system API prototypes) and the three C runtimes
(`libnix`, `clib2`, `ixemul`). No `m68k-elf-gcc` is needed; the AmigaOS-native
toolchain is the one to build on.

**The engine has been compiled for the target.** The §7 assumption — "compile
`engine.cpp` for m68k" — is no longer untested. With

```sh
export PATH="$HOME/.local/m68k-amigaos/bin:$PATH"
m68k-amigaos-g++ -std=c++11 -m68060 -O2 -fexceptions -ffp-contract=off \
    -I native-sdl2/src -I native-sdl2/vendor/stb -c native-sdl2/src/engine.cpp
```

the real, verified renderer compiles clean for the 68060: **166,824 bytes of
text, 4,892 bytes of data, no errors**. `std::shared_ptr`, `std::function`,
exceptions and `<fstream>` all compile and link, and a smoke object links into a
real `AmigaOS loadseg()ble executable`. The only hard failure in the whole
source is `std::mutex` in `assets.hpp`/`assets.cpp`, exactly the shim §8
already lists: this libstdc++ has no thread support, and the mixer's lock should
become a no-op because the host has no threads.

The remaining mechanical items are therefore small and known: the mutex shim,
`timer.device` in place of `std::chrono`, and the resource loader in place of
`<fstream>`. None of them is an unknown.

### 11.4 Host-side helpers

Beyond the emulator and the toolchain, the workflow is covered end to end, and
the step-1 tools were built and run for §13.

| Need | Tool | State |
| --- | --- | --- |
| Colour histograms, median-cut, k-means, AGA-snapped error | `native-sdl2/tools/amiga_palette.py` | **present and executed** |
| Palette derivation from headless frame dumps | `native-sdl2/tools/dump_frames.cpp` (reuses the verified engine; 12 parts, part-labelled PPM) | **present and executed** |
| End-to-end driver | `native-sdl2/tools/amiga-aga-palette.sh` | **present and executed** |
| Side-by-side palette captures | Pillow, in the same tool | **present**, mirroring `wasm/check-presentation.py` |
| Reading/writing ADF and HDF images | `amitools` (`xdftool`, `rdbtool`) via pipx | **present** (0.8.1) |
| Headless AmigaOS API execution | `vamos` via pipx | **present** |
| Assembling the ASM core | `vasmm68k_mot` + VLink (AmigaPorts) | **present** (vasm 2.0b) |

`amitools` is worth calling out because it is the piece that makes a scripted
workflow pleasant: it creates and edits ADF/HDF images, injects a built
executable, and reads the resulting filesystem back, all from Python. Together
with FS-UAE's command line this gives a headless loop — assemble, inject, boot,
capture the framebuffer, compare — without touching a GUI. That loop is exactly
what the staged plan in §10 needs for steps 4 to 7, and it reuses the comparison
discipline the WebAssembly host already established.

### 11.5 Legal and provenance note

The Kickstart 3.0/3.1 ROMs are copyrighted and are not redistributable. They
stay on the development machine, are never committed to the repository, and are
never bundled into a package. The same rule applies to the two external Amiga
references in §16: the Karate archive under `original/` and the sibling
OS-MAGI-80 project are used as references from their own locations and are not
copied into this repository. The Kalms C2P transpose itself is Public Domain;
the Karate sources are offered for reuse by their author; OS-MAGI-80 is MIT and
its adaptations would be attributed if any were adopted. This mirrors the
position the repository already takes for the original Java release: original
bytes are preserved and hash-pinned under `original/`, but third-party rights
are not asserted over them. If a distribution is ever contemplated, the
executable must be separated from the ROM requirement and the user must supply
their own Kickstart.

## 12. References

- `native-sdl2/src/engine.cpp`, `native-sdl2/src/engine.hpp`: the transcribed
  renderer, framebuffer model and per-pixel loops.
- `native-sdl2/src/assets.cpp`: AU decoding and the software mixer.
- `native-sdl2/tools/verify.cpp`: the headless frame/PPM exporter and the
  11,101-frame deterministic comparison, which is the oracle for §7 and the raw
  source for §4.2.
- `assets/scene.txt`: effects, particle populations and the 222-second timeline.
- `evidence/diagnostic.json`: asset inventory, part boundaries and active effects.
- `evidence/wasm/live-playback.json`: the measured ~42 fps source cadence.
- [WebAssembly reactivation](wasm-reactivation.md) and
  [native reactivation](native-reactivation.md): the existing hosts and the
  cadence policy this study reuses.
- [iOS audio unlock](ios-audio-unlock.md): an example of a host-only change that
  left the verified engine untouched, which is the model proposed here.
- `original/karate_fighter/KarateScript1SRC.lha` and `karatescript12.lha`: the
  Amiga Karate 1.0 source and the KarateScript 1.2 distribution, the ancestor of
  this engine (§16.1).
- OS-MAGI-80 (`$HOME/dev/OS-MAGI-80`): a sibling Amiga project whose
  `src/graphics/c2p4_kalms.S` is the four-plane Kalms adaptation of §16.2, with
  the upstream Kalms Public Domain source vendored in `third_party/kalms-c2p/`.
- AmigaPorts `m68k-amigaos-gcc` (toolchain at `$HOME/.local/m68k-amigaos`): the
  cross-compiler used for the §11.3 and §16.3 compilation of the engine.

## 13. Step 1 results — palette study (executed)

**Verdict: the artistic target is acceptable, so the port proceeds.** The
16-colour look was produced from the verified engine's own output and inspected
scene by scene. The result is a deliberate, readable 16-colour rendition; the
photographic sources band, as predicted, but never become unrecognisable.

**What was run.** The headless export was reproduced without any Amiga:

- `native-sdl2/tools/dump_frames.cpp` — a new exporter that reuses the verified
  engine exactly as `verify.cpp` does. It walks the 12 script parts, reseeds
  `JavaRandom` to 100,000 + tick before each frame, calls the same `frameAt()`,
  and writes PPMs named `tick-TTTTT-part-NN.ppm`. 555 frames were captured at
  every 40th tick (≈ 4 per second) over the 222 s cycle.
- `native-sdl2/tools/amiga_palette.py` — the derivation: a pixel-frequency
  histogram per part with a saturation bonus, median-cut to 16 colours, k-means
  refinement, then a snap to the AGA 12-bit register space (4 bits per channel).
  It also derives a single global palette for the whole demo and can split a part
  into temporal segments, and it measures the error of the AGA-snapped palette,
  which is the number the Copper list will actually show.
- `native-sdl2/tools/amiga-aga-palette.sh` — the end-to-end driver, mirroring
  `verify.sh`.

```sh
./native-sdl2/tools/amiga-aga-palette.sh          # one palette per part
SEGMENTS=4 ./native-sdl2/tools/amiga-aga-palette.sh   # four temporal palettes per part
```

Outputs: `reference/amiga-aga/palettes.json` (palettes, RGB and AGA register
words, statistics), `reference/amiga-aga/compare/*.png` (source over quantised,
plus a 16-swatch strip) and `evidence/amiga-aga/palettes.json` (curated summary).

**Result.** One palette per part is clearly worth it. Against a single global
palette, per-scene palettes cut the mean per-channel error on every part that has
content, in several cases by nearly half:

| Part | Mean abs. channel error, per-scene | … against one global palette | PSNR (per-scene) |
| --- | --- | --- | --- |
| 1 | 0.47 | 0.94 | 41.5 dB |
| 2 | 6.26 | 9.85 | 26.7 dB |
| 3 | 9.65 | 17.31 | 26.5 dB |
| 6 | 14.29 | 19.73 | 23.1 dB |
| 7 | 10.85 | 18.44 | 25.5 dB |
| 8 | 10.31 | 13.94 | 24.4 dB |
| 9 | 12.66 | 16.99 | 24.0 dB |
| 10 | 13.54 | 16.48 | 23.4 dB |
| 11 | 10.62 | 12.81 | 24.8 dB |

Parts 3–11 hold 13,000–70,000 distinct colours per frame and are 85–100 %
covered, so every one of them is a genuinely hard quantisation; the per-scene
palettes nevertheless keep the mean channel error near 10/255. The twelve AGA
palettes total 384 bytes (12 × 16 × 2).

**Temporal segmentation was evaluated and is not needed.** Splitting a part into
2, 4 or 8 contiguous palettes and adding their Copper-list switch cost changes the
mean error by less than 1.5 % (part 11, the 100-second finale, moves from 10.62 to
10.51). One palette per part is the right granularity; the §4.3 palette animation
covers the fades within a part.

**One factual correction to §6.** The assumed fill fractions were optimistic for
the opening and pessimistic elsewhere. Measured on the source frames:

- **Part 0 (0–4 s) is black.** It is the load/setup window: its first frames
  contain about 250 non-black pixels. The scene starts drawing in part 1. The
  quantiser legitimately returns an all-black palette with a 0.28/255 error; it
  costs nothing and needs no special case.
- Part 1 (the "touchload" logo) is only ~7 % covered, not 25 %.
- Parts 3–11 are the opposite extreme: 85–100 % covered, essentially full-screen
  raster work for most of the cycle, not the "~60 % mixed" the budget assumed.

The performance consequence is discussed in §14.

**Reproducibility.** Median-cut is deterministic and k-means is seeded from it
with no random restarts, so repeated runs are byte-identical. `SEGMENTS` and the
tick `STRIDE` are the only knobs.

## 14. Corrected screen statistics and performance consequence

The measured fill fractions replace the §6 estimates. The relevant figure for the
Amiga is not the *average* fill but the *worst* part, because the port must hold
frame rate everywhere.

| Part | Mean fill | Max fill | Distinct colours (mean) | Dominant work |
| --- | --- | --- | --- | --- |
| 0 | 0 % | 0 % | 2 | none (black) |
| 1 | 7 % | 8 % | 25 | sprites |
| 2 | 53 % | 57 % | 21,165 | ground + sprites |
| 3 | 100 % | 100 % | 70,184 | full-screen twirl / motion blur |
| 4 | 88 % | 100 % | 25,305 | ground |
| 5 | 83 % | 86 % | 13,427 | ground |
| 6 | 99 % | 100 % | 60,157 | full-screen twirl |
| 7 | 96 % | 100 % | 17,847 | full-screen tunnel |
| 8 | 85 % | 100 % | 43,525 | mixed ground/tunnel/twirl |
| 9 | 98 % | 100 % | 24,416 | tunnel |
| 10 | 96 % | 100 % | 22,482 | tunnel |
| 11 | 98 % | 100 % | 17,424 | metaball particles + tunnel |

Reading the §6 budget against this table, the port spends most of its time in the
"full-screen, heavy blend" row (11–16 ms total, ~60 fps by the cycle model) rather
than the "mixed, ~60 % fill" row, so the honest planning figure is **~60 fps on
the model, not 100+**, with a **stable 50 fps** as the target and the original
42 fps as the comfortable floor. The conclusion is unchanged — the CPU is not the
bottleneck, and the port presents on vblank — but the margin is smaller than §6
implied, and part 11's metaball pass remains the first candidate for optimisation,
exactly as the plan anticipated.

## 15. Colour-path enumeration (executed)

The §9 risk list made this a prerequisite for the port: enumerate every write to
`kPixel->r/g/b` and to the pixel buffer before starting, so the indexed
conversion is designed against the paths that actually run. It was carried out
statically against `native-sdl2/src/engine.cpp`, and it changed §4.1.

**Framebuffer writers.** Every assignment to the chunky buffer, by method:

| Line | Method | Operation |
| --- | --- | --- |
| 1246 | `MotionBlur` | average current and previous frame, `>> 1` |
| 1522 | `drawrecUV` | opaque copy — **unreachable** |
| 1603 | `drawrecUVL` | additive + clamp — **unreachable** |
| 1675 | `drawrecUVM` | additive + clamp 255 |
| 1770 | `drawrecUVRGB` | additive + clamp 0–255 |
| 1832 | `drawrecUVS` | opaque copy, index −1 transparent |
| 1907 | `drawrecUVSA` | alpha blend, then `drawrecUVS` |

**Reachability from the script effects.** Following the call graph from the
effect handlers in `kEffectX::rendah` to the raster primitives:

- `Sprit`, `Sprit3D` → `draw()` → `drawrecUVRGB`
- `Ground`, `Tunnel` → `SetzoomP`/`Setzoom3D` → `drawrecUVSA` → `drawrecUVS`
- `Flare3D`, `Particle` → `flare3D`/`drawflare` → `drawrecUVM`
- `MotionBlur` → `MotionBlur`
- `Twirl` → `findtwirl` (writes `kPixel` shading, consumed by `drawrecUVRGB`)

`drawrecUV` and `drawrecUVL` appear nowhere in this graph. They are the residual
of the original Java class and are preserved in the transcription, but the Amiga
port does not need them and the ASM raster core should not implement them.

**kPixel writers.** Only `colorclip` (per block, from `helpfade`) and `findtwirl`
(per block) write `kPixel->r/g/b`. Both run once per 16×16 block, i.e. 627 blocks
for a full 520×300 context, so an indexed port quantises there, not per pixel.

**Consequence.** Four reachable per-pixel loops must be carried into the indexed
design — the opaque copy (`drawrecUVS`), the alpha blend (`drawrecUVSA`), the
additive copy (`drawrecUVRGB`/`drawrecUVM`) and the frame average (`MotionBlur`).
All four reduce to a small pair table over palette indices, as §4.1 now shows, but
those tables are palette-dependent and must be rebuilt when the scene palette
changes. The first draft of §4.1 named the wrong two operators and missed the
additive one, which is the model used by the sprites and the particle passes;
§4.1 has been rebuilt accordingly. This enumeration is what makes the §7
assembly work well-defined, and it is why the step-3 ARGB core can be written
against a known, closed set of four loops rather than an open-ended one.

## 16. Reusable Amiga foundation and toolchain validation

Three findings change how the port should be built, and each was verified on
this machine rather than recalled. They are recorded here because they are the
starting point for the assembly and host work, not background.

### 16.1 Code Police is a descendant of the Amiga Karate engine

The preserved user-supplied archive `original/karate_fighter/` holds two LHa
containers: `KarateScript1SRC.lha`, the complete Amiga C and assembly source of
**Karate 1.0** (2004), and `karatescript12.lha`, the **KarateScript 1.2**
distribution (2005) with its plugins and tutorials. The distribution names the
engine "Karate Demo-Script Fighter" and credits **m4nkind**, the same group as
Code Police, with the author's address on the sources. The script format is the
same one this repository already preserves: `<KSCRIPT>`, `<KPART>`, `<KCAM>`,
`<KIMG>`. Code Police's browser/Java demo is therefore the descendant of this
Amiga engine, which means the archive is not merely similar prior art — it is
the same lineage, and the author states plainly that its code may be reused.

Relevant to this port:

- `karateSRC/objects/ChunkyStartup.asm` — the 8-bit AGA host and C2P presenter
  described in §8, with triple buffering, quit handling and a 50 Hz
  `timer.device` clock.
- `karateSRC/objects/c2p1x1_8_c5_bm.s` and `kalmscpu5blit0_{128,200,240,256}` —
  Kalms' routines, wired for 8 planes and specific heights.
- `karateSRC/objects/KMakeColorTable.i` — builds a palette pair table
  `pal[X][Y] = quantise(clamp(pal[X] + pal[Y]))` plus a 15-to-8-bit remap LUT.
  This is the exact §4.1 additive pair table, written in 2001, and it confirms
  that the indexed-design correction in §4.1 follows the engine's own design
  rather than inventing one.
- `karateSRC/objects/Kcaster.i` — `DrawCasterTable`, the raycasting ancestor of
  the ground/tunnel/twirl effects.

**The one mismatch to resolve is depth.** Karate is hard-wired to 8 bitplanes
and 256 colours (`BIDTAG_Depth,8`) and renders natively in 8-bit palette
indices; this port's target is 4 bitplanes and 16 colours. There is no 4-plane
C2P in the Karate archive.

### 16.2 A 4-plane Kalms C2P already exists

The depth mismatch above is closed by a sibling Amiga project, OS-MAGI-80 (MIT
licensed), whose `src/graphics/` directory carries a four-plane Kalms
adaptation. `c2p4_kalms.S` converts a byte-per-pixel chunky buffer to four
separate bitplanes, retaining the upstream non-modulo transpose and pipelined
store order and changing only assembler syntax, the C ABI, and the plane/stride
pointers. Its upstream original (`c2p1x1_4_c5_bm.s`, Public Domain) and its
provenance are vendored unchanged in that project's `third_party/kalms-c2p/`.
The routine's contract — source bytes must be colours 0..15, width a non-zero
multiple of 32 — is exactly this port's target. It is verified by byte-exact
host vectors and by real 68020 assembly loops under the Musashi emulator, and it
is that project's selected runtime converter. Two alternative backends (a pair
LUT and a five-stage table-free merge) ship alongside it.

So the two halves of the low-level foundation now have concrete sources: the
presentation host and the indexed pair-table idea from the Karate archive
(§16.1), and the four-plane Kalms converter from OS-MAGI-80. Neither is
committed here; both are external references to be adapted, and the Kalms
transpose is Public Domain while the host module is offered for reuse by its
author.

### 16.3 The toolchain is installed, and the engine compiles

The cross-toolchain this study once listed as "the real gap" is present on the
machine and has been exercised (§11.3). An AmigaPorts `m68k-amigaos` suite lives
at `$HOME/.local/m68k-amigaos`, off `PATH`: `m68k-amigaos-g++` 16.2.0b, binutils
2.46, `vasmm68k_mot` 2.0b, and the NDK 3.2 headers. `amitools` 0.8.1 and `vamos`
are installed through pipx for ADF/HDF work and headless AmigaOS execution.

The decisive check, which the study had deferred to "a tiny translation unit",
was run against the real engine. `native-sdl2/src/engine.cpp` compiles clean for
`-m68060 -std=c++11 -O2 -fexceptions` (166,824 bytes of text), and links into a
real AmigaOS hunk executable. The single compilation failure in the whole
source is `std::mutex` in the asset layer, which §8 already schedules for a
no-op shim. `shared_ptr`, `std::function`, exceptions and `<fstream>` are all
supported.

### 16.4 What this means for the plan

- Step 3 (the ASM raster core) is no longer gated on installing a toolchain.
  vasm 2.0b and an AmigaOS C++ compiler are usable today, and the ARGB core can
  be assembled and linked on this machine.
- Step 4 (C2P bring-up) has a ready 4-plane Kalms adaptation to adopt directly,
  rather than a routine to write.
- Step 7 (host integration) has a reusable triple-buffered AGA presenter to
  adapt from the same lineage, with only its plane count and C2P entry to
  change.
- The next verification milestone is executable, not analytical: build the
  engine for m68k with the mutex shim, place it on an ADF with `amitools`, run it
  under FS-UAE, and compare frames against the `verify.cpp` oracle. Everything
  needed for that loop is already installed.
