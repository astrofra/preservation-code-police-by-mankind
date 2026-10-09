# Java reactivation

Date: 2026-10-09. The requested Java desktop restoration is based on the unchanged
original bytecode and script. CFR/Procyon output is reconstructed source, not
the author's original Java files. No C++, browser or WebAssembly port is included.

## Running and building

On the tested Apple Silicon Mac, open `dist/java/macos-arm64/Code Police.app`.
It includes its own Java runtime and the original asset files. Close the window,
click the original close image, or press Escape to quit.

From the repository root, for development with a JDK 25 installation on PATH:

```sh
./java-desktop/gradlew -p java-desktop run
./java-desktop/gradlew -p java-desktop run --args='--mute'
./java-desktop/gradlew -p java-desktop check installDist
java-desktop/build/install/code-police/bin/code-police
python3 java-desktop/tools/package_macos.py
```

For direct `java` playback of the original bytecode **without the cinema frame**:

```sh
./java-desktop/gradlew -p java-desktop referenceClasses
java -cp 'java-desktop/build/classes/java/main:java-desktop/build/classes/java/reference' OriginalHost "$PWD"
```

Run these from the repository root. The first command builds the existing host
and only needs repeating after source changes. `OriginalHost` opens a plain
520 × 300 applet surface and loads `original/mkd_codepolice/*.class` through an
isolated classloader; it does not run the reconstructed desktop engine or the
macOS package. It uses the original applet audio API. `page.html` is read only
to recover the `SCRIPT` parameter; no HTML decoration is displayed. This route
is tested on Java 25 and requires the legacy Applet API. Close its window to quit.

Gradle Wrapper 9.4.0 is complete and pins the distribution checksum. The compiler
toolchain is Java 25; application bytecode targets Java 17. The macOS application
bundles Homebrew OpenJDK 25.0.2 (ARM64), linked from `java.base` and
`java.desktop` plus their required modules. The reference harness needs an Applet-capable JDK (tested on 25);
the desktop application has no dependency on `java.applet`.

`prepareAssets` copies `page.html`, all 53 images and all 10 AU files unchanged
to `java-desktop/assets/`. Gradle distributions place that folder alongside
`lib/`; the macOS package places it alongside the code JAR. Lookup is relative
to the installation location, not the current directory. `-Dcodepolice.assets`
is an explicit development/test override. Packaging requires Python/Pillow and
macOS `iconutil`; the icon is a documented scaling of the original `badge2.gif`.
No media is embedded as arrays or hidden inside the application JAR.

## Structural fidelity and documented changes

The 23 classes in the applet's static dependency closure retain every original
non-synthetic method. The recovered main and nested class names, fields,
parameter tables, formula order, script parsing, rendering loops and unused
methods within those classes remain in the CFR-derived engine. Compiler-generated
accessors may differ when rebuilt by javac. The other 16 original classes are
preserved in the archives and independent reconstructions, with explicit
exclusion from the active build because they are outside this dependency graph.

The complete [class/method mapping](../evidence/java-reactivation/class-method-correspondence.json)
and [source diff against CFR](../evidence/java-reactivation/desktop-platform.patch)
make the boundaries reviewable. The following are the substantive adaptations:

| Original location | Desktop replacement | Reason and observable effect |
| --- | --- | --- |
| `kraycasting extends Applet`, `kRenderContext.app` | `DesktopSurface extends Canvas` | Removes the obsolete applet host while keeping engine entry points and the logical 520 × 300 surface. |
| `getParameter("SCRIPT")` | `SceneAssets` reads the original `page.html` attribute | Supplies the same original script; literal `SplitSTag` parsing is retained, including disabled-looking tags. |
| `getImage(getDocumentBase(), path)` | Local file URL, AWT decoder and synchronous `MediaTracker` preload | Preserves AWT-decoded pixels and catches missing files; resources are ready before the original timer starts. |
| `createImage(ImageProducer)` | ARGB snapshot of the published buffer | Keeps original buffer swapping but prevents repaint from observing a buffer being overwritten by the worker. The original pixels, including alpha, remain intact. |
| `applet_image`, `boolend` | Volatile publication | Makes frame and stop visibility explicit between AWT and the render worker. |
| `start()` | Guard against duplicate workers; clear stop flag; name thread | Prevents multiple render threads from a repeated lifecycle call. Normal playback retains its original start point. |
| `stop()` | Set end flag, join worker, then stop sounds | Prevents a final render from restarting audio after shutdown. No deprecated `Thread.stop()` is introduced. |
| AWT painting | Added `paint()` forwarding to original `update()` | Restores exposed windows without advancing the simulation on repaint. |
| Top-level `kSound.OurAudio` and constructor | `DemoAudio` replaces the `AudioClip` type | Preserves all recovered `kSound` command/state methods; Java Sound decodes μ-law to signed PCM and plays independent looping clips. |
| Browser/HTML framing | Fixed 640 × 480 desktop content with the four original border images | Retains the cinema framing. The original close image now works; Escape and window close also stop playback. |

The original `run()` clock reads, per-frame effect order and `sleep(20)` remain
unchanged. No fixed-step scheduler was imposed. Particle movement and motion
blur are frame dependent, so faster rendering on modern hardware can differ
from historical machine cadence even when the code and elapsed-time sequencing
match. Minimized windows continue the timeline and audio. There is no pause or
seek control in normal playback and no artwork mouse interaction was added.

Java Sound performs independent clip mixing through the host device. Event order
is checked against the original, including the approximately 10 ms `vib1`/`vib2`
loops. Exact acoustic output of the historical Microsoft/Sun applet audio engine
has not been established. The original files are unchanged; no resampled audio
is written back to the archive.

## Verification

```sh
./java-desktop/gradlew -p java-desktop check
./java-desktop/gradlew -p java-desktop compareOriginal
python3 java-desktop/tools/source_correspondence.py
./java-desktop/gradlew -p java-desktop verifyWindow
./java-desktop/gradlew -p java-desktop verifyWindow -Pduration=22 -Poffset=210
```

`check` decodes all images using AWT and all audio using Java Sound, and verifies
the script's literal tag behavior and the 12-part, 222-second nominal cycle.

`compareOriginal` loads the original classes in an isolated classloader and
provides an applet stub only in test code. It compares 11,101 consecutive frames
from ticks 0 through 22,200 at a controlled 20 ms step: every output pixel,
camera/particle primitive field and array, and the sequence of 38 sound commands
matches. Both renderers receive the same test-only `Math.random()` seed for
initialization and each frame. Normal playback does not alter the random seed.
This is a controlled numerical/visual equivalence test, not a claim about
historical frame rate or acoustic equivalence.

The first real-time window test reached the final scene (at 123 seconds) and
ended with a stopped engine at 175 seconds, without a recorded worker exception.
That run does not establish a complete uninterrupted real-time cycle. A separate
22-second live test uses a test-only initial timer offset of 210 seconds to
exercise the loop boundary, then normal playback, minimization/restoration,
pointer events, active audio output and clean shutdown. The offset exists only
in the external test harness and does not affect the application.

Real screen captures and deterministic reference/restored captures are described
in [the reference index](../reference/index.md). These are new JDK 25 captures,
not historical screenshots. Listening by a person, Windows/Linux playback and
Intel macOS have not been validated.

The application supports `--smoke-test` for package validation: it opens normally,
checks that rendering and audio remain active, and closes after eight seconds.
This opt-in test flag has no effect on ordinary launches. See the retained
validation records under `evidence/java-reactivation/` for the final results.

The direct original-bytecode host was also launched with `java`, observed at
eight seconds, checked for its 520 × 300 applet-only content, captured and
closed through its window listener. This was a short launch/close check; the
longer reconstruction comparisons are described above.

The packaged application passed this smoke test from a separate temporary
directory containing spaces and an accented character, with `JAVA_HOME` unset,
`PATH=/usr/bin:/bin` and network access denied by macOS `sandbox-exec`.
The test therefore used its bundled runtime and installation-relative assets.
All 64 packaged script/media files match the original archive. Local code-signature
verification also passed; the application has not been notarized.

## Resolution and fullscreen presentation (2026-10-09)

The original-bytecode host and the reconstructed Java launcher now accept
`--resolution WIDTHxHEIGHT`, `--scale 1..4` and `--fullscreen`. The native port
uses the same options and geometry. Windowed mode fits the 520:300 artwork
inside the requested dimensions, rounding to pixels (1280x720 -> 1248x720;
800x600 -> 800x462), with no bands. Fullscreen keeps the current monitor mode,
maximizes the image and centers black letterbox/pillarbox bands. F11 toggles and
restores the prior corrected window size; Escape closes. Nearest neighbour is
explicitly selected in Java2D. Window sizes are OS logical content dimensions.

`DisplayOptions` contains CLI/geometry, and `DemoWindow` owns only presentation.
The original applet remains an offscreen 520x300 component with its original
worker. An independent AWT repaint timer presents its existing `update()` output
through scaled graphics. The reconstructed engine uses the same presentation
adapter when display options are supplied. Its existing no-option cinema host
remains available; invoke display options to use the bare artwork view:

```sh
./java-desktop/gradlew -p java-desktop run --args='--resolution 1280x720 --fullscreen'
```

No `GraphicsDevice.setDisplayMode` call is made. The fullscreen window uses the
current graphics device, and the tests check that its resolution, bit depth and
refresh rate are unchanged across entry/exit. The engine size, double buffers,
script, rendering expressions and sound/timing logic are unchanged.

Run `./java-desktop/gradlew -p java-desktop verifyDisplay` for live geometry,
nearest-neighbour pattern, black-band, F11/window-size restoration, manual resize,
and original-bytecode presentation checks. The CLI Java distribution includes
the two presentation classes without including the reconstructed demo engine.
