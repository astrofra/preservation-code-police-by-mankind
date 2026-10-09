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
