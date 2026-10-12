# Code Police — Amiga port (C)

An AmigaOS port of the verified Code Police engine. **The target is C**, not
C++: the Amiga ancestor engine (Karate) is plain C, and it keeps the toolchain,
the runtime and the ABI simple. The C++ engine under `native-sdl2/` is not the
port; it is the **verification oracle** (§7 of the feasibility study) that the
C port must match frame for frame.

## Why C

- The preserved Karate source (`original/karate_fighter/`) is 12 kLOC of C.
- The AmigaPorts C++ runtime needs a `std::mutex` shim and a libc stub just to
  link; C avoids all of that (§16.3).
- The inner loops are assembly anyway; C is the glue, and C is what the existing
  host module (`ChunkyStartup.asm`) expects.

## Layout

- `src/amiga_platform.h/.c` — the host boundary: file loading, PNG decode via
  `stb_image`, time, and (later) audio and events. Mirrors the C++ `Platform`
  in `native-sdl2/src/platform.hpp`, deliberately without `std::mutex`.
- `src/main.c` — the port's entry point. Milestone 1 loads and decodes an asset.
- `build.sh` — compiles for `m68k-amigaos` and runs the result under `vamos`.

## Toolchain

The AmigaPorts cross-toolchain is installed off `PATH` (see the study's §16.3):

```sh
export PATH="$HOME/.local/m68k-amigaos/bin:$HOME/.local/bin:$PATH"
```

`build.sh` uses `-mcrt=nix20` (libnix), the same runtime the sibling OS-MAGI-80
project links against, and executes the result with `vamos` at `-C 20` (68020,
which `vamos` supports; it does not emulate 68060 instructions, so use FS-UAE
for the 68060 target). `vamos` runs the process with a tiny default stack,
which `stb_image`'s zlib overflows, so `build.sh` passes `-s 256` (KiB).

## Milestones

1. **Asset path — done.** `./amiga/build.sh` compiles the C host for
   `m68k-amigaos`, links an AmigaOS hunk executable, and under `vamos` decodes a
   PNG and hashes it. The hash matches the host exactly
   (`6b2dfccf12eaa45a` for `Images/CARE.gif.png`), so the file path, the PNG
   decode and the `0xFF000000 | rgb` pixel convention are confirmed on the
   target before any engine code is written.
2. **Script parser — done.** `amiga/src/kscript.{h,c}` ports the tokeniser.
   `python3 amiga/validate.py` confirms it matches `evidence/diagnostic.json`
   on every field.
3. **Renderer core — done.** `amiga/src/renderer.{h,c}` ports the context, the
   rasterisation primitives and the field generators.
   `python3 amiga/compare_render.py` shows 7/7 paths bit-exact against the
   verified C++ engine.
4. **Object graph and whole-script frames — done.** `amiga/src/objects.{h,c}`
   builds textures, tables, cameras, particles, rectangles and the effect tree,
   and renders real frames of the whole 222 s script. `python3
   amiga/compare_frames.py` compares all 11,101 frames' hashes against the
   oracle in `native-sdl2/build/reference.bin`: **11,099 of 11,101 match**
   (99.982 %), including camera maths, particle simulation and every effect.
5. **AGA host** — a real screen, Copper palette, the four-plane Kalms C2P
   (`c2p4_kalms.S`), and input/quit.

### The two known divergences

Ticks 11050 and 11070 (part 10, the particle passage) differ. The state dump in
`native-sdl2/tools/frame_state.cpp` and the port's own dump show the particle
systems' `ave` sums and `pos[0]` are identical; the divergence is sub-ULP in
`popu3`'s positions and reaches the pixel hash on exactly those two frames. It
does not propagate: every frame before and after matches. It is recorded rather
than hidden, and is the next precision bug to chase.

### How the port is verified

The same C sources build for the host (fast: the whole 11,101-frame cycle runs
in a few seconds) and for m68k. The differential checks run the host build, and
`compare_render.py --m68k` and `compare_frames.py --m68k` spot-check the real
m68k binary under `vamos`. This keeps the object graph, effect dispatch, camera
maths and particle simulation identical across both, since they share one source.
