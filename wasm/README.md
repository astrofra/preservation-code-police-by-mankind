# Code Police in the browser

This port compiles the existing C++11 engine to WebAssembly. The landing page
has a **Start** button, credits read from the original TXT, Pouët/GitHub links,
resolution selection, optional original cinema borders, sound and fullscreen.
The English interface uses a navy-blue palette and “CODE POLICE by Mankind”.
The cinema is on by default. Start is the user gesture that enables browser audio.

From any working directory, run the repository's scripts:

```sh
./build-wasm.sh
./preview-wasm.sh
# Open http://localhost:8000/
```

`preview-wasm.sh 8765` selects a different port. The build needs Git, Python 3,
CMake and a make backend. It installs **Emscripten 4.0.23** locally under
`wasm/build/emsdk` if that exact version is not already active. This version pins
the **SDL2 2.32.8** port. To reuse an SDK without modifying shell startup files:

```sh
CODEPOLICE_EMSDK=/path/to/emsdk ./build-wasm.sh
```

The complete site is `dist/wasm/code-police/`, with an upload archive at
`dist/wasm/code-police-web.zip`. Upload the directory/ZIP **contents** to a static
HTTP(S) host. Paths may be nested and contain spaces or Unicode. Serve `.wasm`
as `application/wasm` and `.js` as `text/javascript`. No Java, CDN, PHP, remote
media or cross-origin isolation headers are required. Opening `index.html` via
`file://` is not supported. The extracted ZIP also works with Python's HTTP server.

The selected resolution describes the artwork. A 1280×720 request fits to
1248×720; 800×600 fits to 800×462. The image and optional cinema shrink together
if the browser is smaller. Fullscreen maximizes the selected presentation:
the full cinema when enabled, otherwise the 520:300 image. Black fills the
remaining area; no monitor mode change occurs. Return to the landing page to
change resolution/sound and restart. Cinema can also be toggled during playback.
The original close image or **Home** returns to the landing page; Escape
first leaves browser fullscreen, then returns to the landing page. Hidden tabs
pause both animation time and audio and resume without skipping scenes.

## Preservation and resources

`native-sdl2/src/engine.cpp`, its headers, arithmetic helpers, STB decoder and
AU mixer are compiled directly, without edits for the browser. The 520×300
artwork is rendered through SDL2's software renderer to `#canvas`. A separate
visible canvas uses device-pixel dimensions and Canvas2D with smoothing disabled.
This avoids relying on CSS `image-rendering` for arbitrary fractional scaling.
No WebGL context, worker, pthread or Asyncify is used.

All 65 native resource files remain external under `assets/`. The loader fetches
them relative to the module, checks length and (when Web Crypto is available)
SHA-256, and writes their unchanged bytes into MEMFS before `main`. The PNGs are
the already verified Java-decoded GIF/JPEG derivatives; all ten AU files and the
KarateScript are unchanged. The independent original cinema JPG/GIF images are
copied to `Images/`. Errors identify the missing or corrupt resource.

The loader restores the original CRLF line endings if a text-mode upload has
changed `scene.txt` to LF, then applies the same size/checksum checks. It does
not accept a truncated or otherwise altered script. Resources use hash-versioned
URLs, the manifest is revalidated, and GUI scripts/styles have versioned links
to avoid mixing cached files between packages. Use binary transfer mode when
uploading the site to preserve every original file exactly.
Downloads run two at a time, with bounded retries that respect the server's
`Retry-After` header when it responds with HTTP 429.

`package.py` generates the cinema table from a copy of original `page.html`.
The new page replaces popup/frameset navigation, the applet and the historically
broken close action. Explicit column widths and block images retain the original
640×480 geometry. The original empty title is replaced with an accessible title.
The complete 107-file original release is included unchanged under `original/`;
its outdated Java instructions describe that historical release only.

`provenance.json`, `build-info.json`, `resources.json` and `SHA256SUMS` record
the adaptations, pinned dependencies, source identity and package hashes. JS
is unminified, WASM retains names, and the shared engine/adapter sources are
included under `sources/`. Library licenses are included; no new license is
asserted over Mankind's original artwork.

## Verification

```sh
# Once, if the original-bytecode reference has not been exported:
./native-sdl2/verify.sh
cd wasm
npm ci
npx playwright install firefox
# Screenshot checks also require Pillow: python3 -m pip install Pillow
npm test
```

The test uses installed Google Chrome plus Playwright Firefox; select a
subset with `CODEPOLICE_BROWSERS=chromium npm test`. WebKit can also be requested
explicitly, but the installed Playwright WebKit binary crashes at launch on this
macOS 14.1 machine, before loading the page. It extracts the
actual ZIP outside the repository, serves it at a nested accented URL, verifies
all hashes and rejects external runtime requests. It compares all 11,101
sequential pixel hashes against the original Java bytecode, selected numeric
states and actual canvas pixels, all ten decoded PCM buffers and the sound
commands. It also exercises Start/release, touch, HiDPI, cinema, resolution,
fullscreen, hidden/resume and missing/corrupt resources. Explicit browser
timeouts bound loading and UI waits.

`node wasm/live-smoke.mjs http://localhost:8000/` (from the repository root,
while preview is running) separately plays the complete 222-second script in
real time, checks its loop, samples actual Web Audio output and returns home.

Reports: [`evidence/wasm/`](../evidence/wasm/).
Captures: [`reference/wasm/`](../reference/wasm/).
See [technical notes](../documentation/wasm-reactivation.md) for the actual
tested browser versions, timing adaptation and remaining platform limits.
