# Diagnostic log

## 2026-10-08 — Initial request and scope

The user requested a quick diagnosis of the Java demo supplied in `original/`,
using the legacy Java preservation skill. They subsequently pointed out that
the Komplex/Forward Java workflow is also relevant. The adopted approach is
static archive, HTML, media and bytecode inspection, applying the skill's
Komplex bytecode cross-checks where relevant. No application port is requested.

The repository initially contained an English README and an untracked
`original/` directory; there were no staged changes. Original bytes are retained.

Initial findings: the supplied Pouët snapshot identifies Code Police by Mankind
(February 2001, second in the PC demo competition at Synthesis Party 2001).
The release text credits Krabob for code. The ZIP passes its CRC check.
`page.html` embeds a substantial scene script in the applet's `SCRIPT` parameter;
the applet canvas is 520 × 300 inside a 640 × 480 HTML layout.

The macOS Java registration lookup found no registered runtime. The shell does
resolve Homebrew `java` and `javap`; their availability is checked separately.
No live playback has been attempted.

## 2026-10-08 — Static diagnosis verified

- The user clarified that Mankind did not obfuscate its code. Readable class and
  method names support this; no obfuscation-removal work was performed.
- Homebrew Java and javap 25.0.2 work despite the empty macOS runtime registry.
  All 39 classes pass the skill's outer-structure inventory and javap disassembly.
- The initial unresolved-class warning was investigated: all five missing names
  occur outside the entry point's static closure. All 23 reachable classes exist.
- The ZIP's 107 files match the supplied extraction byte for byte. All 53 images
  and 10 sounds decode. All 41 script image paths and 10 sound paths exist with
  exact case. Media originals and the supplied catalog snapshot remain unchanged.
- Literal tag parsing, checked against `SplitSTag`, finds 12 active parts and a
  222-second nominal loop. Disabled-looking tags with embedded spaces must not
  be silently enabled by a replacement parser. Timer units are 10 ms, while the
  worker sleeps 20 ms after rendering; particle movement also includes per-frame
  state. These distinctions matter to faithful timing.
- The Komplex/Forward adjacent float/double-to-long-to-int conversion pattern is
  absent from the complete disassembly. This is a targeted check, not proof that
  an unperformed decompilation will be correct.
- The first inspection-script run exposed a Pillow API difference: JPEG images
  lack `n_frames`. Using a default of one for those images fixed the inspector;
  subsequent complete runs passed. Original files were never changed.
- The Java team's published explanation confirms browser applet obsolescence and
  removal of `java.applet` in Java 26. The JEP page itself returned HTTP 403; the
  successful official source is linked in the report.
- Agent decision: stop at the requested diagnosis, retain repeatable static
  evidence, and create one verified local milestone including the supplied
  preservation artifacts. No full decompilation, executable adaptation or push.

See [quick-diagnosis.md](quick-diagnosis.md) for evidence links, reproduction
commands, restoration priorities and the explicit limits of this inspection.

## 2026-10-09 — Java reactivation requested

The user requested the next stage: make the demo run again in Java. The repository
starts this stage clean at `717343a`. The implementation will preserve the
original engine structure, scene script and external media, retain independent
decompilations, establish an original-bytecode reference host, and isolate the
desktop platform replacements. Java playback, audio, lifecycle and packaging
will be checked on the available macOS host. No native or browser port is in scope.

### Independent reconstruction milestone

CFR 0.152 and Procyon 0.6.0 were already installed locally. Both were run against
a generated JAR containing all 39 unchanged class files. CFR's main applet and
active top-level sound class compile. Five unreferenced nested classes omitted
by CFR's JAR-mode output were also decompiled individually; Procyon includes
them in its main file. Eleven unreferenced top-level classes remain documented
outside the active build, including their missing alternate dependencies.

Procyon reconstructs `SplitSTag` incorrectly: substring assignment runs after
the terminal search result. The original bytecode condition and CFR agree.
The active CFR reconstruction was therefore selected without modifying either
decompiler's output. `reverse/README.md` describes coverage and reproducibility.

### Desktop and validation milestone

The active CFR-derived engine now runs through a small Canvas/resource adapter
and Java Sound clip wrapper. The original script, rendering arithmetic, timing
reads, 20 ms post-render sleep, class/method names and external assets are retained.
The four original cinema-frame images and close button are presented in a fixed
640 × 480 window; the artwork remains 520 × 300. The close button, Escape and
window close stop the worker and audio cleanly. The explicit platform deviations
and all 39 class dispositions are recorded in `java-reactivation.md` and
`evidence/java-reactivation/`.

An initial Java image check exposed asynchronous Toolkit dimensions after a
PixelGrabber-only preload. AWT MediaTracker now finishes preparation before the
engine uses the image; all 53 images then pass the actual Java decoder check.
All 10 original AU files decode through Java Sound. No original bytes changed.

The full deterministic comparison executes the unchanged archived classes through
an isolated test Applet stub. All 11,101 sequential frames from ticks 0–22,200
match the desktop output exactly, along with camera and particle state and all
38 audio commands. Test-only random seeding is not part of normal playback.
Method inventory confirms every non-synthetic original method in the 23 active
classes; all 16 inactive classes remain archived and independently decompiled.

The first real-time test reached every scene, including the final scene at
123 seconds, then observed a stopped engine at 175 seconds without a recorded
worker exception. Its interruption cause was not established, and it is retained
as an incomplete run rather than reported as a full-cycle pass. The follow-up
live test starts at a test-only timeline offset of 210 seconds and successfully
crosses the loop boundary, exercises pointer events and minimize/restore, observes
active audio playback and verifies clean window/thread/audio shutdown after
22 seconds. No historical acoustic equivalence or human listening is claimed.

The complete Gradle 9.4.0 Wrapper builds Java 17 bytecode with JDK 25. A macOS
ARM64 application was packaged with Homebrew OpenJDK 25.0.2 and original assets
outside its JAR. All 64 distributed script/media files match the archive.
The application was copied to a temporary path containing spaces and an accented
character, launched from that directory with JAVA_HOME unset and a PATH without
the development JDK, and run under a macOS sandbox denying network access.
Its eight-second GUI/animation/audio/close smoke test passed. The bundled app
also passed `codesign --verify --deep --strict`; this is local signing, not
notarization. `jdeps -s` reports only java.base and java.desktop dependencies.

Retained screen captures were visually inspected. Reference images and provenance
are in `reference/`; originals still match all 130 manifest hashes. Final Gradle
checks pass. macOS ARM64 is the tested target; Windows, Linux and Intel macOS
remain untested. No repository push or other port was performed.

## 2026-10-09 — Direct Java launch requested

The user asked for command-line playback without the HTML/cinema frame, then
clarified that they want to invoke `java`, rather than use the desktop package.
The existing original-bytecode reference host already provides a bare 520 × 300
window. Its direct invocation is being checked before choosing the final launcher.
No desktop presentation change has been made in this stage.

Direct invocation of the existing `OriginalHost` passed on Java 25.0.2. An
eight-second probe checked the original `kraycasting` component's 520 × 300
dimensions, captured its visible output and closed the window; the process
exited normally. The captured image was visually inspected and contains only
the applet. No new launcher or desktop option was needed. README and the Java
guide now give the direct command and its one-time host compilation prerequisite.

## 2026-10-09 — Distributable original-bytecode Java package requested

The user requested a distributable package, specifically a `.sh` that prepares
the package and a second `.sh` inside it that launches the demo. This continues
the direct `java`, original-bytecode, no-cinema-frame workflow. The packaging
script will compile only the existing host/support classes into a small launcher
JAR and copy the unchanged original release alongside it. The runtime package
will require an installed compatible Java, with no Gradle or compiler needed
by recipients. The existing desktop application is not the deliverable here.

The builder is `java-original/package.sh`; it uses `javac --release 17`, `jar`
and `zip` directly. It creates `dist/code-police-java-original.zip` plus its
unpacked directory, containing `run-code-police.sh`, a small host-only JAR,
instructions and all 107 original release files. No JVM, Gradle or reconstructed
engine is bundled. The launcher locates resources relative to its own path,
honors JAVA_HOME, checks the Java 17–25 requirement and rejects Java 26 explicitly.

The existing OriginalHost gained argument validation, a fixed 520 × 300 window
and an opt-in eight-second smoke-test mode that verifies the worker and clean
shutdown. Original class files are unchanged. The extracted archive passed its
CRC check, retained launcher executable permissions and matched every original
release file byte for byte. Launch/close smoke checks passed with Homebrew Java
21 and 25 from `/tmp`, in an extraction path with spaces and an accented character,
while macOS sandbox-exec denied network access. A simulated Java 26 command was
correctly rejected before launch. Results are under `evidence/java-original-package/`.
Linux, Windows and Java 17 playback remain untested. The finished archive is
approximately 1.2 MB; no upload or push was requested or performed.

## 2026-10-09 — C++11/SDL desktop port requested

The user requested a C++11/SDL desktop port without the cinema frame, with a
future WebAssembly port as a major design constraint. The repository starts
this stage clean at `34d43c4`. SDL2 is selected according to the preservation
skill. The current task implements and validates the native port; the browser
build remains a later stage, as requested.

The planned boundaries are an SDL-independent translation of the recovered
classes and methods, external original/faithfully converted assets, a platform
clock/resource/audio interface, and a thin SDL presentation/event loop. The
validated Java reconstruction and archived bytecode remain comparison references.
No renderer redesign, frame-based-to-fixed-step rewrite or embedded media arrays
are planned. The available host is Apple Silicon macOS with SDL2 2.32.70.

## 2026-10-09 — Native engine and numerical correspondence verified

Implemented the 23 active classes in strict C++11, retaining 79 explicit
method/constructor declarations and bodies plus the Java default constructors.
The SDL-independent engine keeps original parsing, rendering, buffer swaps,
camera/particle calculations and audio command logic. Explicit integer and
random-number semantics and ordered effect arguments avoid C++11 differences.
The translation tool and method map are retained as evidence; ordinary builds
use checked-in C++ and require no Java or translator.

All 53 images were converted through Java Toolkit/PixelGrabber to external PNGs;
all decoded ARGB arrays round-trip exactly, with a per-file hash manifest. The
script was extracted byte-for-byte (UTF-8 representation, original line endings)
and all ten AU files were copied unchanged, preserving filename case. A direct
hash audit confirms all 130 original files remain unchanged.

During implementation the user identified KScript as Krabob's Karate language,
and supplied a local KarateScript 1.2 tutorial and its Pouët entry. Inspection
confirmed that the actual launcher references `.txt` scripts, with related
`KCAM`/`KSCRIPT`/`KPART` tags. Adopted `scene.txt`, documented the relationship,
and kept the exact earlier Code Police parser rather than importing later
Amiga-language extensions. No tutorial files were modified or bundled.

The original bytecode and C++ engine agree on all 11,101 full-frame RGB hashes
across controlled ticks 0..22200 at step 2, and on every pixel of 16 selected
captures. Every camera/particle primitive and array was checked per frame;
the largest relative numerical difference is 5.4706239538404589e-14. All 38
sound commands agree at their issue frames, and all ten AU PCM decodes equal
Java's sample for sample. The full comparison also passes ASan/UBSan. Reports
are in `evidence/native/`; paired captures are in `reference/native/`.

The native host opens only the 520x300 demo, with nearest-neighbour resizing,
audio and clean shutdown. A four-second smoke run exercises minimize, restore,
resize, pointer movement and Escape. A longer run requested 226 seconds and
closed after 6,039 frames, but its final capture still shows the last scene and
the earlier host log did not record elapsed time or the shutdown cause. It is
therefore not evidence of an uninterrupted full loop. Added elapsed-time,
loop-count and quit-reason logging. A subsequent offset-210-second test ran
for 22.0386 seconds / 877 frames, explicitly recorded one timeline reset, and
exited because the requested duration elapsed. This validates the live loop
boundary separately from the exhaustive controlled-state run.

Packaging inspection revealed the installed SDL2 2.32.70 is actually
`sdl2-compat`, which dynamically loads SDL3. For a self-contained native package,
the builder instead pins upstream SDL2 2.32.10, checks its archive hash and links
it statically. `otool` confirms the result depends only on macOS system libraries
and frameworks. The native archive and `.app` are now being validated outside
the repository; browser compilation remains future work.

## 2026-10-09 — Native distribution verified

Completed the reproducible `.sh` package builder and distributable shell
launcher. The approximately 2.6 MB macOS ARM64 ZIP includes the application,
external script/images/audio, original credits, dependency licenses and a
resource/executable hash manifest. SDL2 is statically linked; only macOS system
libraries/frameworks remain. The local `.app` signature verifies successfully.

`tools/check_package.py` extracted the actual ZIP into a temporary path with
spaces and accents, then ran its shell launcher from `/tmp` with a minimal
system PATH and denied network access. Live audio, rendering, minimize/restore,
resize, pointer and Escape passed, ending at the requested 4.03262 seconds.
LaunchServices also opened the extracted `.app`, wrote a framebuffer capture,
and exited after the requested duration. All 65 packaged external resources and
the final signed executable matched their hashes. Saved the report, loop-boundary
log, sanitizer smoke log and system dependency audit under `evidence/native/`.
No original Java source, bytecode or release file was changed. No remote upload
or push was performed. macOS 14.1 ARM64 is the tested runtime; other desktop
targets and WASM remain explicitly untested.

## 2026-10-09 — Display resolution and fullscreen options

The user requested equivalent display-resolution/fullscreen CLI options for the
Java and C++ versions, using nearest-neighbour interpolation. They clarified
that fullscreen must keep the monitor's existing resolution and enlarge the
520x300 framebuffer as far as the aspect ratio permits. Their subsequent
suggestion to use the smaller dimension was adopted as the smaller *scale
factor*: fit inside the requested window bounds, rounding to the nearest pixel,
with no windowed bars. Examples: 1280x720 -> 1248x720; 800x600 -> 800x462.
Fullscreen retains the full screen surface and centers the maximal image with
black letterbox/pillarbox bands.

Added shared Java presentation classes and corresponding C++ geometry helpers;
new options are `--resolution WIDTHxHEIGHT`, `--fullscreen` and `--scale 1..4`.
F11 restores the last corrected window size; Escape exits. Both render engines
remain fixed at 520x300. The original Java host calls the unchanged applet's
`update` through scaled graphics, with presentation polling on the AWT thread;
its original worker and timing remain independent. The restored Java launcher
also accepts these options for an undecorated-artwork presentation, while its
existing no-option cinema presentation remains available. No original class or
engine arithmetic was edited.

Java live checks already pass for seven reference geometries, malformed options,
exact 2x nearest-neighbour pixels, fullscreen bars, unchanged monitor mode,
F11 entry/exit and window-size restoration, constrained manual resizing, and
scaled playback of the original bytecode with its fixed 520x300 buffer. C++
geometry checks pass the same examples plus fitting/idempotence across a grid
of requested dimensions. Native readback and package checks are in progress.

Native readback validation found two platform-specific test issues before
completion: macOS fullscreen Space transitions could delay processing of the
second F11 event past the smoke-test timeout, and the notch/menu safe area means
the drawable need not equal the raw monitor bounds. Disabled SDL fullscreen
Spaces, made the smoke test wait for both actual toggles, and fit to the actual
drawable. The test also verifies the real fullscreen flag and unchanged display
mode. These changes do not switch the monitor resolution.

Both the installed SDL2-compatible development build and the pinned SDL2 release
passed actual presentation readback checks. Window 1280x720 becomes 1248x720
(2496x1440 HiDPI output); window 800x600 becomes 800x462 (1600x924 output).
Fullscreen readback confirmed a centered aspect-preserving image, black bands,
no newly interpolated colours, two F11 transitions and unchanged monitor mode.
The native distribution was rebuilt, extracted and checked offline, including
the assertion that the original window size is restored after F11.

Rebuilt the original Java ZIP and the Java desktop application. The extracted
Java ZIP passes with live audio in fullscreen on Java 25 and windowed 800x462 on
Java 21, from `/tmp` with network denied and an accented/spaced package path.
All 107 packaged original files and all 130 archived repository files remain
byte-identical. Reports are in `evidence/native/display-checks.json`,
`evidence/native/package-check.json` and
`evidence/java-original-package/display-checks.json`. Only the presentation/host
layer changed; the previously validated renderer arithmetic was not modified.

The rebuilt bundled-runtime Java desktop app also passed its 800x462 display
smoke check with animation, live audio and clean shutdown. The final native ZIP
passed its offline extraction/LaunchServices check, including actual F11 window
size restoration. Documentation and all three generated deliverables are updated.

## 2026-10-09 — WebAssembly and optional original cinema presentation

The user requested the browser port, an optional reconstruction of the original
HTML cinema surround, and selectable display resolution. They then specified a
landing page with a Start button, credits from the release TXT, and links to
Pouët and the GitHub repository. The established 520x300 logical buffer,
nearest-neighbour scaling, fit-inside sizing and fullscreen without a monitor
mode change remain the presentation requirements.

Implementation decisions: reuse the validated C++11 engine and external PNG/AU
resources unchanged; isolate browser scheduling and presentation in `wasm/`.
The Start gesture will unlock audio. Generate the cinema markup from the
archived `page.html`, retaining its 640x480 table and original border images;
replace the applet with a canvas. Preserve all three historical HTML files in
the package. Replace popup/frame navigation and the broken close link with an
in-page player and return-to-home action, and document those adaptations.
Build a self-contained static site and ZIP with root build/preview scripts;
validate the extracted archive over HTTP before delivery. No upload is requested.

The first browser build now runs with Emscripten 4.0.23 / SDL2 2.32.8. The native
engine, Java host and existing assets remain unmodified. The package contains
the full unchanged original release, standalone runtime resources, readable JS,
WASM names, shared source copies, dependency notices and hashes. The landing
page parses credits from the release TXT and uses Code Police's archived Pouët
ID 1788 plus the repository's existing GitHub origin.

Initial Chrome comparison completed all 11,101 controlled reference frames,
16 actual-canvas captures/numeric states and 10 decoded PCM buffers without
differences outside the native numerical tolerance. Live playback and an active
Web Audio context also work. The missing-WASM test found that a later rejected
promise overwrote the filename-bearing error; the loader now preserves the first
error. Validation of the corrected extracted ZIP and additional browser engines
is continuing. The visible canvas uses physical device dimensions and disabled
smoothing to retain nearest-neighbour output at fractional sizes; the cinema
table has explicit original column widths to avoid modern fixed-table inference.

Chrome and Firefox completed the extracted-package comparison. Web Audio output
is also sampled after the intentionally silent four-second opening; actual
nonzero output is confirmed. The test now compares audio commands on every frame,
including frames expected to contain none. Resource packaging additionally checks
all AU files and the extracted script directly against the original bytes.

A screenshot-level check found fractional CSS compositor filtering that was not
visible in Canvas2D readback alone. Replaced the centering transform with absolute
pixel-aligned layout, and made the close button's larger touch target transparent.
Added screenshot-versus-canvas comparisons and black-band checks to prevent this
regression. This changes only browser presentation. The native geometry CTest
still passes and the shared renderer/Java sources have no differences.

The installed Playwright WebKit build 2251 crashes with a bus error before loading
a page on macOS 14.1 ARM64. Its launch failure is retained in
`evidence/wasm/webkit-launch.txt`; no Safari/WebKit compatibility is claimed from
this attempt. Final checks target actual Chrome 154.0.8037.98 and Playwright
Firefox 155.0, with physical mobile devices explicitly untested.

Final extracted-ZIP checks pass on both browsers: 11,101 original-Java frame
hashes each, 16 numeric-state/actual-source-canvas captures, all 38 sound commands
with no unexpected commands on other frames, ten PCM hashes, live nonzero
Web Audio output, landing credits/links, mouse-release Start, touch Start once,
HiDPI 2x, custom-size validation, optional cinema, fullscreen/restoration,
hidden/resume, and missing/corrupt resource errors. Actual screenshots equal the
visible canvas, 2x cinema pixels equal the Java reference and fullscreen bands
are black, with no interpolated colours. All 219 final package files, 107 included
original release files and 130 archived repository files pass hash verification.

The separate live run observed the full 222-second script loop after 9,407
rendered frames, with audio and clean stop. It used the same engine binary as
the final package; CSS centering was corrected during that long run and verified
separately in the final ZIP tests. Reports/captures are under `evidence/wasm/` and
`reference/wasm/`. The root builder also ran successfully from `/tmp`. The static
ZIP and ready-to-upload directory are under `dist/wasm/`. No upload or push was
performed. This completes the requested browser port and launch page milestone.

## 2026-10-09 — Browser loading repair and English navy interface

The user reports `assets/scene.txt : taille incorrecte`, and requests removing
the two introductory sentences, switching the complete HTML GUI to English,
using “CODE POLICE by Mankind”, and replacing the green UI palette with navy blue.
The local source, packaged file and HTTP preview all contain the expected 44,195
bytes and matching SHA-256. The script contains 1,488 original CRLF line endings;
conversion to LF during text-mode transfer would produce 42,707 bytes and trigger
the reported check. This is a reproducible hypothesis, not yet a confirmed cause
on the user's host; its URL has been requested. Also address stale cached files
without weakening the media integrity checks. Preserve original resource bytes
and the existing engine while adapting the browser loader and new GUI only.

The user provided https://demoports.emucamp.com/code-police/ . Direct HTTP reads
confirm the hypothesis: the hosted manifest requires 44,195 bytes, while its
script has 42,707 bytes, zero CRLFs and 1,488 LFs. Restoring CRLF yields exactly
the original file and its SHA-256. Recorded the byte counts and hashes in
`evidence/wasm/hosted-loading-diagnosis.json`; the original loader also reproduces
the reported error with that same LF transformation. The repaired loader inserts
only missing CR bytes and then retains the original length/checksum checks.
It also requests fresh manifests and versioned resources; packaging versions the
GUI JS/CSS links so an updated page does not reuse the older interface.

The new HTML interface is entirely English, reads “CODE POLICE” / “by Mankind”,
omits both introductory sentences and uses navy-blue surfaces with blue accents.
Credits remain sourced from the TXT, with English role labels. Original cinema
artwork, archived release, Java/native engine and runtime resource files are
unchanged. New screenshots are kept in `reference/wasm/navy-en/`, preserving the
earlier GUI captures as a separate version.

A read-only browser check against the host also encountered HTTP 429
responses during parallel downloads; individual file requests succeeded. Reduced
media concurrency from six to two and added at most two retries, respecting the
server's `Retry-After` delay. Versioned assets can use the normal HTTP cache;
only the manifest is forcibly revalidated. The regression check simulates both
a recoverable 429 followed by LF script bytes and a persistent 429, as well as
truncated and genuinely changed scripts. The fullscreen test now waits for the
actual restored canvas dimensions after the asynchronous browser transition.

The hosted browser check identifies the 429 page as the host's Tiger Protect
security response. It blocks this automated live-page check, including cinema
images, despite successful individual HTTP reads; it is not evidence of successful
remote deployment. The server has not been modified. The full extracted-ZIP tests
pass on Chrome and Firefox: both recover from an LF script and an initial 429,
restore exactly the original bytes in MEMFS, launch the demo, reject truncated
or altered scripts, and still match all 11,101 reference frames. English controls,
new captures, source hashes and the rebuilt ZIP are verified. Earlier GUI captures
remain intact. The local preview is updated; the user must replace the hosted
site contents with the new package to apply the changes online.

## 2026-10-12 — Amiga AGA feasibility, step 1 executed

The Amiga AGA 68060 study (`documentation/amiga-aga-feasibility.md`) made its
palette study the decisive first step. It was executed today on this machine,
with no Amiga and no emulator, using only the verified engine and the existing
Python stack.

A new headless exporter, `native-sdl2/tools/dump_frames.cpp`, reuses the verified
engine exactly as `verify.cpp` does (seed 888 at init, 100,000 + tick before each
frame) and writes part-labelled PPMs. 555 frames at every 40th tick cover the
whole 222 s cycle. `native-sdl2/tools/amiga_palette.py` derives the palettes:
pixel-frequency histogram with a saturation bonus, median-cut to 16 colours,
k-means refinement, then a snap to the AGA 12-bit register space.
`native-sdl2/tools/amiga-aga-palette.sh` drives it end to end. The palettes and
source-vs-quantised captures are in `reference/amiga-aga/`; the curated summary is
`evidence/amiga-aga/palettes.json`.

Result: the artistic target is acceptable, so the port proceeds. One palette per
part cuts the mean per-channel error roughly in half against a single palette for
the whole demo (for example part 3, 9.65 vs 17.31, and part 7, 10.85 vs 18.44), and
temporal segmentation (2–8 palettes inside a part) changes the error by under
1.5 %, so one palette per part is the right granularity. The twelve palettes total
384 bytes.

Two corrections to the study's estimates came out of the measurement. Part 0
(0–4 s) is a black load/setup window — it contains about 250 non-black pixels, and
the scene starts drawing in part 1 — so its legitimate palette is all black. And
parts 3–11 are 85–100 % covered, not the "~60 % mixed" the performance budget
assumed, so the honest planning figure is nearer 60 fps on the cycle model than
100+, which is recorded in the document's new §14.

Nothing was installed for this step and no toolchain was built. The next steps in
the plan (the ASM raster core, then C2P) still require vasm and an AmigaOS C++
cross-compiler, which remain absent.
