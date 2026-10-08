# Code Police — quick static diagnosis

Date: 2026-10-08. Scope: inspect the supplied files, using the legacy Java
preservation workflow and its Komplex/Forward bytecode checks. No port, full
decompilation or live playback was requested or performed.

## Assessment

The supplied distribution is a strong starting point for faithful reactivation.
The ZIP passes CRC checks; its 107 file entries match the extracted release
byte for byte. The applet's static application dependency graph is complete,
and every media file declared by the scene script is present with exact filename
case. This supports feasibility, not a claim that the demo already runs correctly.

The archived Pouët page identifies **Code Police by Mankind**, released in
February 2001, second in the PC demo competition at Synthesis Party 2001.
The original `MKD_CodePOlice.txt` credits code to Krabob; graphics to Zaac, Alexx,
Myke and Krabob; sounds to Leviathan and Tex. These are supplied documentary
claims, not dates inferred from filesystem timestamps. The user's comment that
Mankind did not obfuscate its code is consistent with the readable class and
method names observed in this release.

## What is preserved

| Item | Finding |
| --- | --- |
| Java bytecode | 39 classes, all class-file version 45.3; no Java source files supplied |
| Entry point | `kraycasting extends java.applet.Applet implements Runnable` |
| Display | 520 × 300 applet canvas in a 640 × 480 HTML composition |
| Images | 53 files: 37 GIF and 16 JPEG; all decode; all single-frame |
| Audio | 10 AU files; all decode; all mono 8 kHz μ-law |
| Scene script | Embedded directly in the `SCRIPT` parameter of `page.html` |
| Declared media | 41 image references and 10 audio references; none missing |
| Timeline | 12 active parts; nominal 222-second cycle, configured to loop |
| Historical reference | Saved Pouët page and its screenshot, supplied locally |

The SHA-256 [manifest](../evidence/original-manifest.json) covers 130 supplied
files, excluding macOS `.DS_Store` metadata. Acquisition dates are unknown.
No asset was converted or modified.

## Engine and composition

The main class initializes a render context, reads `SCRIPT`, creates the script
and rendering objects, then starts a worker thread. The render context holds
two integer pixel buffers and presents them through AWT `MemoryImageSource`.
Textures are read through `PixelGrabber`. The inspected application references
use standard Java/AWT/applet facilities; no external graphics library or native
library-loading call was found.

The script provides asset names, cameras, rectangular viewports, spline tables,
particle definitions and ordered effects. Active effects include sprites,
perspective sprites, flares, raycast ground and tunnels, twirl, motion blur,
particles and timed sound events. Constant, affine, sine, cosine, random and
spline parameter classes make this a configurable demo engine. The readable
script is particularly valuable evidence of composition and sequencing.

The timer computes `(System.currentTimeMillis() - StartTime) / 10` and narrows
to an integer. Script durations therefore use 10 ms units. The worker sleeps
20 ms **after** computing a frame; this is not evidence of a steady 50 fps.
The 222-second nominal cycle includes a 4-second opening part and a 100-second
final part. Actual observed playback duration and loop behavior remain untested.

Particle motion contains per-call velocity and position updates in `move()`,
called by `mvtfloor()` and `mvtgalax()`. A restoration must account for this mix
of elapsed-time sequencing and frame-dependent state, as well as motion blur;
changing the frame scheduler can change the image even at matching timestamps.

The HTML is part of the presentation: `index.html` opens `frame.html` in a
fullscreen window, which loads `page.html` with its decorative image frame.
The release instructions specifically target Internet Explorer and say the
graphical close button is broken, recommending Alt+F4. A later restoration
should distinguish the 520 × 300 artwork from its surrounding HTML design.

## Dependencies and Komplex/Forward checks

The raw class inventory reports five unresolved application names: `helpfade`,
`kEffectX`, `kParameterXOB`, `kScript` and `kTable`. Following the actual entry
point instead reaches **23 supplied classes with no unresolved application
reference**. The unresolved names belong to classes outside this closure.
These appear to be leftover alternate classes; that origin is an inference.
They should be preserved, not used as evidence that the active demo is incomplete.
In particular, active audio uses the top-level `kSound`, while the similarly
named `kraycasting$kSound` is outside the main static closure.

The skill's Komplex/Forward guidance is applicable as a verification method,
without attributing Code Police to Komplex or assuming identical engine code.
`javap -p -c` successfully disassembles all 39 classes. No adjacent
`f2l; l2i` or `d2l; l2i` conversion pair was found anywhere in that disassembly.
The specific intermediate-narrowing pitfall documented for Forward/Godog is
therefore not demonstrated here. Rendering arithmetic, integer overflow,
array bounds and active counts still need checking during a future decompilation.

The script parser uses literal, case-sensitive tag delimiters. Spellings such
as `<K PART>` and `<F x>` leave some apparent content inactive; replacing the
parser with a tolerant XML/HTML parser would change which effects execute.
The reported timeline and effects follow the original delimiter behavior.

## Reactivation constraints and next step

The original launch route depends on browser applet support. Modern browsers no
longer support that route, and Java 26 removes the `java.applet` package.
See the Java team's [applet removal explanation](https://inside.java/2025/12/03/applet-removal/).
The local Homebrew JDK 25.0.2 successfully provides `javap`; the macOS registered
runtime lookup alone did not find it. No application launch was attempted.

For a future restoration, first obtain independent CFR and Procyon reconstructions
and cross-check ambiguous code against the saved bytecode. Preserve a minimal
Java reference host around the original classes for behavioral comparison where
possible, then replace applet resource loading, presentation and audio in an
explicitly separate desktop adaptation. Retain the original script and media.
Native SDL2 or browser ports can follow if requested.

Audio needs particular care: music is already supplied as sampled AU loops,
so no module-player dependency was found. `vib1.AU` and `vib2.AU` are only about
10.6 and 10.5 ms long, respectively, and are explicitly looped by the script.
Their headers and payload lengths agree; they must not be discarded as truncated
files. Loop gaps, concurrent clips and event timing must be checked audibly.

The applet's lifecycle also deserves validation: `stop()` stops audio and sets
an end flag; `start()` creates a new thread without visibly resetting that flag.
Restart, thread shutdown, painting, image fidelity and audio synchronization
remain untested. There is no evidence from this static diagnosis alone that
the original is broken during its intended one-shot launch.

## Checks and reproduction

Tools used: Homebrew OpenJDK/javap 25.0.2, Python 3.14.7, Pillow 12.1.0 and
FFmpeg/ffprobe 7.1.1. All 39 classes passed the skill inspector's outer-structure
checks and `javap` disassembly. All 53 images decoded through Pillow; all 10
sounds decoded completely through FFmpeg, with AU header length checks as well.
This does not validate Java's decoders, live rendering or mixing.

From the repository root, with the skill installed at the indicated local path:

```sh
python3 /Users/fra/.codex/skills/reactivate-maeda-java/scripts/inspect_classes.py original/mkd_codepolice > evidence/classes.json
javap -classpath original/mkd_codepolice -p -c original/mkd_codepolice/*.class > evidence/bytecode/all-classes.txt
python3 evidence/inspect_original.py
```

The inspection script requires Pillow, ffprobe and ffmpeg. Its outputs are
[diagnostic.json](../evidence/diagnostic.json) and the manifest; it never executes
the archived Java classes. The [bytecode listing](../evidence/bytecode/all-classes.txt)
is disassembly, not recovered original Java source. No decompiler comparison,
visual comparison, launch, interaction test or listening test has yet been done.
