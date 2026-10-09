# Java reference captures

All captures below were generated during the 2026-10-09 restoration on Apple
Silicon macOS with Homebrew OpenJDK 25.0.2. They are **not historical captures**.
The historical user-supplied Pouët screenshot remains separately in `original/`.
The SHA-256 [manifest](manifest.json) identifies the retained PNG files.

`java-bytecode/tick-*.png` renders the unchanged original 39-class archive through
the test-only Applet stub. `java-desktop/tick-*.png` renders the corresponding
restored engine. Each image is the 520 × 300 logical framebuffer at 1× export
scale. Tick numbers use the original 10 ms time unit. These images come from a
sequential comparison, starting at tick 0 and rendering at every even tick up
to 22,200. The test controls `Math.random()` with seed 888 at initialization and
100,000 + tick before each frame. Audio is replaced with event recording in
this comparison. No pointer input is used. Both versions match pixel for pixel.

| Tick | Elapsed time | Reference / desktop files |
| --- | --- | --- |
| 1500 | 15 s | `java-bytecode/tick-01500.png`, `java-desktop/tick-01500.png` |
| 2500 | 25 s | `java-bytecode/tick-02500.png`, `java-desktop/tick-02500.png` |
| 4500 | 45 s | `java-bytecode/tick-04500.png`, `java-desktop/tick-04500.png` |
| 8000 | 80 s | `java-bytecode/tick-08000.png`, `java-desktop/tick-08000.png` |
| 11000 | 110 s | `java-bytecode/tick-11000.png`, `java-desktop/tick-11000.png` |
| 13000 | 130 s | `java-bytecode/tick-13000.png`, `java-desktop/tick-13000.png` |

`java-desktop/screen-27s.png`, `screen-47s.png`, `screen-117s.png` and
`screen-137s.png` are actual screen crops produced by `java.awt.Robot`, including
the 640 × 480 cinema frame. They use normal unseeded randomness, the recovered
worker cadence and real Java Sound output. Filename times are nominal test
elapsed seconds; a small foregrounding delay precedes each screenshot, so they
are not exact-time comparison frames. Exports are 640 × 480; the underlying
physical display scale was not recorded. Pointer events and minimize/restore
were exercised at seconds 18–19; no artwork mouse response is defined.

The image-producing source is the Java reactivation milestone following
`7a503ae`; the commit that first adds this index and these files records its
exact source version. Reproduction is documented in
[java-reactivation.md](../documentation/java-reactivation.md).
