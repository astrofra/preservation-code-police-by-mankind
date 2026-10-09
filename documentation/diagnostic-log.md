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
