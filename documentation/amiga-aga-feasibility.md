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
| Mapping arithmetic | 16.16 fixed point, shift/mask texel lookup | `drawrecUV`, `drawrecUVL` |
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

Instead the port should **make the renderer indexed**: textures are converted to
palette indices once at load, and the blend loops operate on indices through
small tables. Concretely, every per-pixel colour operation in the engine maps to
a precomputed table:

| Engine operation | Current ARGB code | Indexed equivalent |
| --- | --- | --- |
| `drawrecUV` | texel copy | `dst[i] = tex[uv]` |
| `drawrecUVL` | add scalar `n` to R, G, B, clamp each | `dst[i] = ramp[level][tex[uv]]`, 32 × 16 bytes |
| `MotionBlur` | `(a & 0xFCFCFF + b & 0xFCFCFF) >> 1` | `dst[i] = mix[a][b]`, 16 × 16 bytes |
| `findtwirl` colour | per-block `r = n*26` etc. | quantise per block (627 blocks/frame) |
| `colorclip` fade | interpolate RGB by distance | palette animation, §4.3 |

The `drawrecUVL` row is the important one: three channel adds, three clamps and
three conditional moves per pixel become **one byte lookup**. The shaded paths
get *faster* at 16 colours, not slower. This is the single largest performance
win in the whole study, and it is a direct consequence of the 16-colour target.

The two lookup tables are trivial in size (512 B and 256 B) and can be rebuilt
in vblank when a scene's fade level changes.

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
4. Store one 16-entry palette per part — 12 palettes, 576 bytes total.
5. Verify by re-rendering the sampled frames, quantising to the palette, and
   measuring mean and maximum per-channel error against the ARGB source. The
   repository already depends on Pillow for screenshot comparison
   (`wasm/check-presentation.py`), so the tooling for this check is in place.

Twelve palettes is a deliberate choice over one global palette: the script has
genuinely different looks (title, tunnels, particles, the 100-second finale) and
per-scene palettes cost nothing to switch.

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

**Options, briefly.** Blitter-only is still not viable: the blitter cannot
transpose bitplanes, so it would need four masked passes and it steals the chip
bus. Akiko is CD32-only and not present on an A1200/A4000. Kalms remains the
recommended choice, with the 68060 variant.

## 6. Performance budget

With indexed rendering the inner loop is a load, a table lookup and a store.
Reasonable 68060 figures, caches warm and the texture resident:

| Inner loop | Cycles/pixel | Notes |
| --- | --- | --- |
| `drawrecUV` opaque | 4–6 | shift, mask, or, load, store |
| `drawrecUVL` via ramp LUT | 6–8 | one lookup replaces the RGB maths |
| `MotionBlur` via mix LUT | 8–10 | two loads plus one lookup |
| Worst observed mix | ~12 | |

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
- **Inner raster loops** — `drawrecUV`, `drawrecUVL`, `MotionBlur`, the particle
  and flare blits. These are load/lookup/store loops where a C compiler emits
  register spills that cost more than the loop body.
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

**Pixel format and the indexed conversion.** This is the substantive change.
The engine's `Array<jint>` pixel buffers, the ARGB constants (`0xFF000000 | …`)
and the per-pixel RGB maths must become 8-bit indices plus lookup tables, as in
§4.1. If the goal is to preserve `engine.cpp` exactly, the alternative is to
keep it as the oracle and write a parallel indexed renderer in ASM — which is
precisely the two-stage plan in §7 and is the recommended route.

**C++ runtime.** Still the largest hidden cost if any engine code is compiled:

| Feature used | AmigaOS classic status | Mitigation |
| --- | --- | --- |
| `std::shared_ptr` (`Ref<T>`) everywhere | Available, heavy; refcount traffic | Non-atomic intrusive pointer behind the `Ref<T>` typedef |
| Exceptions (`throw std::runtime_error`) | Usually disabled on m68k | Error returns or a fatal handler for the few sites |
| `std::function` in `Sound::event` | Heavy | Function pointer plus user data |
| `std::mutex` in the mixer | No threads in the host | Compile the lock to a no-op |
| `std::sqrt`/`atan2`/`sin`/`cos` | Fine on a full 68060 FPU | Keep; move to fixed point only if profiled |
| `std::chrono`, `<fstream>` | Not the desktop API | `timer.device`; embedded resource blob |

bebbo-amiga-gcc is the established toolchain for this target.

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
| Indexed conversion breaks a per-pixel colour effect not listed in §4.1 | Medium | Enumerate every write to `kPixel->r/g/b` and the pixel buffer before starting |
| Palette quality on photographic textures at 16 colours | Medium | Per-scene palettes and the offline error check mitigate it; expect visible banding on gradients |
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
   not proceed.**
2. **ASM raster core in ARGB**, validated bit-for-bit against `verify.cpp`'s
   hashes. No Amiga required.
3. **C2P bring-up** with a simple routine at 320×200 4 planes; get the picture on
   an AGA screen and confirm the display path end to end.
4. **Swap in Kalms**, 68060 variant, and measure at a fixed scene.
5. **Indexed renderer and palette switching** per part, with the Copper fade
   animation.
6. **Host integration**: screen, copper, input, quit, assets, audio.
7. **Measure on real hardware** and optimise only the slowest part, expected to
   be the metaball particle pass in part 11.

Steps 1 and 2 decide the project, and both can be completed before any
Amiga-specific host code exists.

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

FS-UAE being a **Homebrew formula rather than a cask** is convenient: it is a
native arm64 binary and it is fully scriptable from the command line, which is
what the automated verification path in §7 wants. There is no GUI launcher
installed and none is required. `~/Documents/FS-UAE/` already exists with the
standard tree (`Kickstarts`, `Hard Drives`, `Floppies`, `Configurations`,
`Save States`, ...), so the ROM path below is conventional, not invented.

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

### 11.3 Cross-toolchain: the real gap

The hot core is assembly and the rest is C++, so two toolchains are needed and
**neither is installed yet**.

**68060 assembly — vasm + vlink.** Homebrew has no formula (`brew search vasm`,
`vbcc` and `vlink` return nothing relevant). The toolchain is built from source,
which is a few minutes of work with the C compiler already present:

```sh
# vasm is distributed by its author at http://sun.hasenbraten.de/vasm/
tar xf vasm.tar.gz && cd vasm
make CPU=68060
# vasm and vlink are then single static binaries, no runtime dependencies
```

vasm is the standard modern 68k assembler and its `68060` backend is exactly
what the C2P routines and the raster inner loops need. The Kalms routines ship
as vasm-compatible source in the demoscene convention, so no translation is
required.

**C++ — the choice is not obvious, and it should be prototyped early.**

| Route | Availability | Assessment |
| --- | --- | --- |
| `m68k-elf-gcc` (Homebrew, bottled, 16.2.0, GPL-3.0-or-later) | **not installed**, one `brew install` away | A bare-metal ELF toolchain from GCC upstream, with no AmigaOS libc, startup code or `amiga.lib`. Useful for assembler object tests and for checking what the compiler does with the engine's inner loops; **not** a way to produce an AmigaOS executable. |
| **bebbo-amiga-gcc** (source, gcc 6.5 / 13.4) | not installed | The standard AmigaOS m68k cross-compiler. Produces real AmigaOS hunk executables and links against `amiga.lib`, `proto/`, `graphics.library` stubs. **The realistic toolchain for this port.** |
| vbcc | not installed | Lighter, C-focused, weaker C++ support than gcc. A fallback if gcc's m68k code generation disappoints. |

The practical consequence is that the §7 assumption — "compile `engine.cpp`
for m68k" — must be validated on a tiny translation unit before any port work
is planned around it. The engine uses `std::shared_ptr` throughout, `std::function`,
`std::mutex`, exceptions and `<fstream>` (see §8); the shims in §8 exist precisely
because a stock AmigaOS toolchain does not offer all of that out of the box.
Concretely, the first spike is:

1. `brew install m68k-elf-gcc` for a fast, disposable check of what the compiler
   does with a `std::shared_ptr` and a table-driven inner loop.
2. Build bebbo-amiga-gcc and repeat, against the real target runtime.
3. Only then decide how much of the engine is compiled versus rewritten in ASM.

### 11.4 Host-side helpers

Beyond the emulator and the compilers, four small tools cover the workflow, and
three of them are already satisfied by the repository's Python stack.

| Need | Tool | State |
| --- | --- | --- |
| Colour histograms, median-cut, per-channel error | numpy + Pillow, or a small C tool | **present** (2.3.5 / 12.1.0) |
| Palette derivation from headless PPM dumps | `native-sdl2/tools/verify.cpp` | **present**, already writes PPM |
| Side-by-side palette captures | Pillow | **present**, used by `wasm/check-presentation.py` |
| Reading/writing ADF and HDF images | `amitools` (`xdftool`, `rdbtool`) via pip | **absent**, one `pip install amitools` away and macOS-friendly |
| Assembling the ASM core | vasm + vlink | absent, source build |

`amitools` is worth calling out because it is the piece that makes a scripted
workflow pleasant: it creates and edits ADF/HDF images, injects a built
executable, and reads the resulting filesystem back, all from Python. Together
with FS-UAE's command line this gives a headless loop — assemble, inject, boot,
capture the framebuffer, compare — without touching a GUI. That loop is exactly
what the staged plan in §10 needs for steps 3 to 7, and it reuses the comparison
discipline the WebAssembly host already established.

### 11.5 Legal and provenance note

The Kickstart 3.0/3.1 ROMs are copyrighted and are not redistributable. They
stay on the development machine, are never committed to the repository, and are
never bundled into a package. This mirrors the position the repository already
takes for the original Java release: original bytes are preserved and hash-pinned
under `original/`, but third-party rights are not asserted over them. If a
distribution is ever contemplated, the executable must be separated from the ROM
requirement and the user must supply their own Kickstart.

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
