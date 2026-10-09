# Code Police — Mankind (2001)

This package runs the original Java demo with a **520 × 300 render buffer**, with sound
and without the HTML/cinema frame. The original `.class` files are unchanged.

## Launch

Install **Java 17 through 25**. Java 21 or 25 is recommended. A full JDK is not
required by the package; no compiler, Gradle, Python or browser is needed.
Java 26 and later do not include the Applet API required by the original classes.

On macOS or Linux, extract the complete ZIP, then run:

```sh
./run-code-police.sh
./run-code-police.sh --resolution 1280x720
./run-code-police.sh --resolution 1280x720 --fullscreen
```

If your extractor does not preserve executable permissions:

```sh
sh run-code-police.sh
```

Escape or close the window to quit. F11 toggles fullscreen and restores the
previous window size. `--resolution WIDTHxHEIGHT` fits the artwork inside the
requested bounds, rounding to pixels: **1280x720 → 1248x720**, **800x600 → 800x462**.
`--scale 1..4` is a shortcut for an integer-sized window (default: 1).
The last size option wins. Interpolation is always **nearest neighbour**.
Sizes describe the content area in OS logical units, excluding the title bar.

Fullscreen fills the current monitor as far as the artwork's aspect ratio
permits, with centered black letterbox/pillarbox bands. It never changes the
monitor resolution. The requested size applies to windowed mode only.
`--mute` disables sound.

The script can be invoked from another working
directory; all resources are resolved from the package. To select a particular
Java installation, set `JAVA_HOME` to its directory.

Direct Java invocation is also possible from the extracted package directory:

```sh
java -jar lib/code-police-launcher.jar .
java -jar lib/code-police-launcher.jar . --resolution 1280x720 --fullscreen
```

`./run-code-police.sh --smoke-test` performs an optional eight-second launch
and shutdown check. Normal playback loops until you close the window.

## Contents and credits

- `run-code-police.sh`: portable POSIX shell launcher.
- `lib/code-police-launcher.jar`: small applet host, not a reconstructed demo engine.
- `original/mkd_codepolice/`: original classes, script, images, sounds and release notes.

The original `page.html` supplies the scene script only; its decoration is not
displayed. Keep the complete directory together when moving or distributing it.
Java itself is not bundled, so this archive is not tied to a CPU architecture.
Playback and shell launching have been checked on Apple Silicon macOS; Linux
and Windows playback have not been tested.

Original credits from `MKD_CodePOlice.txt`: code by Krabob; graphics by Zaac,
Alexx, Myke and Krabob; sounds by Leviathan and Tex. The original release notes
remain in `original/mkd_codepolice/`. Packaging does not change the original
work's authorship or licensing.
