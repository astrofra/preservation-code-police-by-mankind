# WebAssembly reactivation

The requested browser version shares the validated C++11 engine with the native
port. See [native correspondence and reference comparisons](native-reactivation.md)
for the original classes, preserved method decomposition, Java arithmetic,
random call order, image conversions and AU decoding. The browser task changes
none of those shared source files. `wasm/src/main.cpp` is an additional platform
entry point; `wasm/web/` handles the launch page and presentation.

## Page and presentation

The original cinema is a 640×480 HTML table: a 640×13 top border, a central row
of 65×300 / 520×300 / 55×300, and a 640×167 bottom border. Packaging copies that
table from `original/mkd_codepolice/page.html` and substitutes a canvas for the
applet. All four border JPEGs and the 15×15 close GIF are original bytes. The
full original release remains in the package, including the script-bearing
HTML. The runtime reads the previously extracted external `assets/scene.txt`.

The original popup/frameset behavior is replaced by an in-page player. The close
image returns to the new landing page, whose credits are parsed from the original
ISO-8859-1 TXT. The Pouët link uses Code Police's archived canonical ID **1788**;
the previously discussed ID 12562 identifies Karate, not this production.
GitHub points to this repository's existing origin. No external fonts, images
or API requests are used. Links are ordinary user navigation.

The requested resolution is an upper bound on artwork dimensions. Both dimensions
use the smaller scale factor and round to the nearest pixel, as in the desktop
hosts. The whole composition also fits within the available viewport. Fullscreen
fits the chosen presentation (cinema or bare artwork) to the browser's screen
surface, centered on black. It never requests a display mode switch.

The fixed 520×300 SDL software canvas is separate from the visible canvas. The
visible canvas backing size follows physical device pixels; both its size and
placement are aligned to that grid. `drawImage` with `imageSmoothingEnabled=false`
performs nearest-neighbour enlargement, including noninteger factors. This
avoids browser-dependent fractional CSS pixelated filtering. The four decorative
JPEGs retain ordinary browser image rendering; they are not the demo buffer.

Screenshot readback exposed an additional issue: a fractional CSS transform used
to align a centered canvas caused the browser compositor to filter it, despite
an exact Canvas2D backing store and integer reported bounds. The final page uses
absolute layout coordinates on the physical-pixel grid, with no transform on the
visible canvas or its container. Tests compare the actual screenshot crop to
the visible canvas, and 2x cinema output to the original Java capture. Fullscreen
bands are checked for black pixels; the small original close control is accounted
for separately. These checks go beyond reading the unscaled source buffer.

## Timing and audio

The original loop renders, then sleeps for 20 ms. Script interpolation reads
elapsed wall-clock milliseconds divided by 10; particles also advance per render.
An `emscripten_async_call` schedules the next callback **20 ms after** the prior
frame completes. A 50 Hz fixed-step rewrite, or a 60 Hz animation-frame gate,
would change that policy. The browser host therefore uses neither catch-up
updates nor a new physics timestep. Timing remains subject to browser scheduling
and CPU load, just as the Java worker is subject to scheduling and render cost.
Deterministic comparison at imposed ticks is not a historical playback-rate claim.

Browser adaptation: when the tab becomes hidden, audio and the clock pause;
resuming subtracts that wall-time interval from the platform clock. Foreground
stalls retain elapsed time and do not cause a catch-up burst. The engine has no
local civil-date-dependent behavior, so the same imposed timeline is checked
under different browser time zones.

SDL2's Emscripten audio backend uses Web Audio ScriptProcessorNode and resamples
the original 8000 Hz mono PCM through SDL. It is opened/resumed inside the Start
gesture; all original play/loop/stop commands and PCM samples remain in the
shared mixer. ScriptProcessorNode is deprecated but supported by the tested
browsers; an AudioWorklet migration would be a separate host change. No audio
files are rewritten for the browser. Pausing/returning home silences the device.

## Build and validation provenance

Emscripten **4.0.23**, its SDL2 **2.32.8** port and the existing vendored STB
**2.30** are pinned. C++11, disabled FP contraction, Java wrap/shift helpers and
explicit original evaluation order are retained. JavaScript is readable and
WASM retains function names. Static assets are fetched first, then copied into
MEMFS through `preRun`; no `.data` resource bundle or media arrays are generated.

The build records its base Git commit plus hashes of all compilation/packaging
sources, including changes made after that commit. Library notices and the exact
original release accompany the package. The root build/preview scripts resolve
their own location and work from another current directory.

`wasm/prepare-reference.py` condenses the existing original-bytecode binary
reference. `wasm/validate.mjs` tests the extracted ZIP over HTTP at a nested URL
with spaces and accents. Its machine-readable report and identified screenshots
are retained under `evidence/wasm/` and `reference/wasm/`.

Chrome **154.0.8037.98** and Playwright Firefox **155.0**, on macOS 14.1 ARM64,
passed all 11,101 sequential frame hashes, 16 actual software-canvas captures
and numeric states, all 38 audio commands and ten decoded PCM hashes. Maximum
relative numerical-state error against Java was **4.7808978997920804e-14**.
The controlled timeline ran under Pacific/Honolulu; touch/HiDPI checks used
Asia/Tokyo. This validates timezone independence, not a civil-date feature.
The browser output contained no extra interpolated colours at the tested scales.

The installed Playwright WebKit mac14 ARM64 build exits with a **bus error before
opening any page**. This is a test-runner limitation, not a passing WebKit test.
Physical Safari, iOS/Android devices and their audio/fullscreen policies remain
untested. Desktop Chrome/Firefox and emulated touch/HiDPI are the verified scope.

A separate real-time Chrome run completed the full script and observed its wrap
after approximately **222 seconds**, with **9,407 rendered frames**, nonzero
Web Audio output and clean return to the landing page. The imposed reference
schedule and this real scheduler run are separate evidence. Its report is
`evidence/wasm/live-playback.json`. The engine binary was identical to the final
package; the CSS centering correction was validated separately in final-package
screenshots. The observed frame rate is a measurement on this host while other
checks ran, not a claim about historical playback cadence.

Implementation references: [Emscripten runtime callbacks](https://emscripten.org/docs/porting/emscripten-runtime-environment.html),
[Module/preRun and resource location](https://emscripten.org/docs/api_reference/module.html),
[static deployment](https://emscripten.org/docs/compiling/Deploying-Pages.html).
