# Code Police — C++11/SDL2 restoration

The native port retains the original 520 × 300 software renderer and plays the
demo in a plain SDL window, without the HTML cinema surround or a JVM. Window
scaling is nearest-neighbour (2× by default); resizing preserves the aspect ratio.
Escape/window close quits; F11 toggles fullscreen. AU audio remains active.

## Source correspondence

The translation starts from the CFR-based Java reconstruction already verified
against all 39 archived classes. It includes the 23 reachable classes: the
outer `kraycasting`, its 21 active nested classes, and the top-level `kSound`.
Their original names, fields, method boundaries, expressions, redundant locals,
branches and calls are retained. Nested classes become C++ structs with an
explicit `owner` pointer; visibility is relaxed to replace Java's synthetic
accessors. The 16 inactive classes remain preserved and accounted for in
`reverse/README.md` and the Java correspondence report; they are not introduced
into the active native engine.

[`method-correspondence.json`](../native-sdl2/tools/method-correspondence.json)
maps 79 translated method/constructor bodies and declarations to reconstructed
Java lines. Default constructors of `kPixel`, `helpfade` and `kTimer` explicitly
initialize Java's zero-valued fields. AWT `paint/update`, the render thread and
its lifecycle are platform boundaries: `frame()/frameAt()` perform one original
`run()` iteration, `main.cpp` presents the returned buffer, and idempotent
shutdown calls the retained `kSound.StopAll()`. `createImage()` retains the
double-buffer swap while removing the two `MemoryImageSource` wrappers.
`kTexture` receives decoded pixels instead of an AWT image. `kSound`'s linked
list is scoped to one engine instance; its command methods retain their logic.

[`translate.py`](../native-sdl2/tools/translate.py) is an auditable migration aid,
not a runtime or build requirement. It parses the verified Java with
`javalang==0.13.0`, makes the stated platform substitutions, and emits checked-in
C++11. The ordinary build requires no Java or Python parser. The source is not
obfuscated. Source expressions are parenthesized explicitly by the translator.

`java_semantics.hpp` defines Java's 32-bit wraparound, masked arithmetic shifts,
integer division edge cases and double-to-int saturation/NaN conversion without
C++ undefined behaviour. Array assignment shares storage as in Java; grids and
objects are reference-counted with non-owning parent links. Replaced grid pixels
are freed, including those rebuilt by animated `SetRect()` calls. Floating-point
contraction is disabled, and no fast-math option is used. Effect arguments with
`doubleout()` are evaluated through sequential temporaries, preserving Java's
left-to-right consumption of random numbers in C++11.

## Script and external media

The HTML `SCRIPT` parameter is essential: it supplies resources, sequences,
durations, effect parameters, camera animation and sound commands. It is extracted
verbatim to `native-sdl2/assets/scene.txt` (including its original line endings).
The parser retains its case-sensitive tag scanning, including the distinction
between active `<KPART>`/`<Fx>` and inactive misspelled tags. This is not replaced
with an XML parser or a rewritten timeline.

The user identified this language as **Karate Engine / KarateScript by Krabob**.
The supplied local KarateScript 1.2 `Tutorial_3D` provides primary corroboration:
`3D01_MinimalExemple` executes
`Karate s=Tutorial_3D/3D01_MinimalExemple.txt w=320 h=240`, and its script contains
`KCAM`, `KSCRIPT` and `KPART`. The tutorial therefore supports retaining `.txt`.
That later Amiga dialect also has `MAIN`, `PLAY` and 3D constructors absent from
Code Police; this port implements the recovered Code Police dialect only.
Krabob describes the later tool in his comments on
[the supplied Pouët page](https://www.pouet.net/prod.php?which=12562).
The tutorial is contextual evidence, not copied into this project's runtime.

All 53 original images (including unused cinema images) have external PNG
derivatives, named with an appended `.png` so original script filenames remain
unchanged. Java Toolkit/PixelGrabber supplies the reference ARGB pixels; PNG
round trips were checked exactly. This avoids changing the artwork through a
different JPEG or GIF decoder. `image-manifest.tsv` records source, derivative
and decoded ARGB hashes. `stb_image` 2.30 reads the PNGs. The renderer's white
pixel transparency sentinel and integer blends remain original; SDL displays
the final buffer as opaque ARGB8888 with texture blending disabled.

All ten AU files are unchanged. Their 8 kHz mono mu-law samples are decoded to
signed 16-bit PCM and mixed at 8 kHz, then SDL converts to the device format.
Play restarts, stop silences, and loop wraps at the exact original sample count,
including the 85-sample vibration loops. Overlapping voices are summed and
saturated; historical JVM audio-device mixing and resampling are not claimed
to be sample-identical to modern SDL output.

## Timing and future WebAssembly boundary

The original script clock is elapsed wall time divided by 10 ms. The run loop
renders once then sleeps 20 ms. Particles also evolve once per render call.
This port keeps that policy, with no fixed-step rewrite, catch-up frames, or
vsync constraint. The clock continues during minimize/restore; a long stall
advances the timeline but does not invent missed particle updates. The faster
native renderer can perform more particle updates than a historical JVM; the
original timing is machine-dependent and this remains a preservation limit.

`engine.cpp` has no SDL, window, event, filesystem or thread calls. `Platform`
supplies resources, time and audio commands; `frame()` returns a software buffer.
`main.cpp` owns native polling/presentation/sleep, and `assets.cpp` owns external
file decoding and mixing. This prepares reuse with a browser frame callback,
virtual resource filesystem and user-activated audio. No WASM build or browser
runtime is implemented or claimed tested at this stage.

## Verification so far

- Strict `-std=c++11`, Apple Clang 15 on macOS ARM64.
- Original `.class` files executed independently via `ReferenceSupport`; seed
  888 at initialization and 100000 + tick for each controlled comparison frame.
  Normal playback uses a fresh seed and the live clock.
- 11,101 sequential frames, tick 0 through 22200 at step 2: all complete RGB
  frame hashes equal; all 16 selected captures also compared pixel by pixel.
  This 50 Hz test cadence is a controlled experiment, not a historical FPS claim.
- All camera and particle primitive fields/arrays compared at every frame;
  maximum relative floating-point difference `5.4706239538404589e-14`, no visible
  pixel difference. The tolerance is `1e-10`.
- All 38 sound commands match the original, including their frame of issue;
  all ten decoded PCM streams match Java's decoder sample for sample.
- Full comparison also passed AddressSanitizer and UndefinedBehaviorSanitizer.
- A live audio/window smoke check exercised minimize, restore, resize, pointer
  movement and Escape, with clean shutdown.

The longer run requested 226 seconds and ended after 6,039 frames, but its final
capture remains in the last scene and that first host log lacked elapsed time
and quit cause. It is not claimed as an uninterrupted full-cycle check. After
adding explicit instrumentation, a 210-second initial offset plus 22.0386 seconds
of real playback produced 877 frames, one logged timeline loop and an automatic
duration-triggered exit. Offset testing exercises the live loop boundary, not
the particle history preceding that offset.
