# Code Police — C++11 / SDL2

Native 520 × 300 demo, audio included, no cinema frame and no Java at runtime.
The original Karate script is external in `assets/scene.txt`.

## Build and run

Install a C++11 compiler, CMake and SDL2 development files. On macOS, the usual
development build can use `brew install cmake sdl2`. From the repository root:

```sh
./native-sdl2/build.sh
./native-sdl2/run.sh
```

The executable is `native-sdl2/build/code-police`. It locates `assets` next to
it, independently of the current working directory. `--assets DIRECTORY`
overrides that location. CMake copies the checked-in resources during the build.

Escape/window close quits; F11 toggles fullscreen. `--scale 1` gives the original
520 × 300 window, `--scale 2` is the default. Use `--mute` without an audio device.
`--resolution WIDTHxHEIGHT` requests a window fitting within those bounds at the
original 520:300 aspect ratio, with nearest-neighbour scaling. Pixel dimensions
are rounded: `1280x720` becomes **1248x720**, `800x600` becomes **800x462**.
Manual resizing applies the same constraint. Dimensions refer to the window's
content area in OS logical units; HiDPI rendering uses the drawable's actual pixels.

```sh
./native-sdl2/run.sh --resolution 1280x720
./native-sdl2/run.sh --resolution 1280x720 --fullscreen
```

`--fullscreen` uses the current monitor resolution, maximizes the displayed
image while preserving its ratio, and centers it between black letterbox or
pillarbox bands. It **never changes the monitor's display mode**. The requested
resolution determines the window size restored by F11. `--scale` is shorthand
for an integer-sized window; if both size options are supplied, the last wins.

## macOS distribution

```sh
./native-sdl2/package.sh
```

Requires Python 3.9+, CMake, the macOS developer tools and curl. The first build
downloads upstream SDL2 2.32.10, verifies its pinned SHA-256 and builds it
statically. Subsequent builds reuse the local source/cache. The result is
`dist/native/macos-arm64/Code Police.app` on Apple Silicon and
`dist/native/code-police-macos-arm64.zip`, with a `run-code-police.sh` launcher.
No Homebrew libraries, SDL3, Java or network access are needed at playback time.
The app is locally ad-hoc signed, not notarized. The deployment target is macOS
11; only the actual build host has been tested. Linux/Windows packages are not
provided or claimed tested.

To test the generated ZIP outside the repository with network denied, a minimal
system PATH, live audio and a separate LaunchServices launch:

```sh
python3 native-sdl2/tools/check_package.py
```

## Preservation verification

```sh
./native-sdl2/verify.sh
```

This developer-only check needs JDK 25 for the original Applet classes (normal
native builds do not). It recreates PNG derivatives with Java's image decoder,
executes the original bytecode, exports 11,101 frames' hashes and complete camera/
particle states, then compares the C++ renderer and audio commands. Its binary
reference is about 560 MB under ignored `build/`, not part of the distribution.
Sixteen full pixel captures supplement the whole-timeline hashes. Audio PCM
is also compared sample by sample. For sanitizers:

```sh
cmake -S native-sdl2 -B native-sdl2/build-asan -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCODEPOLICE_SANITIZERS=ON
cmake --build native-sdl2/build-asan --parallel
native-sdl2/build-asan/verify-native native-sdl2/assets native-sdl2/build/reference.bin native-sdl2/build/pcm
```

`--smoke-test` exercises the actual window/audio, minimize/restore, resize,
pointer, F11 twice and Escape for at least four seconds (allowing OS fullscreen
transitions to finish), and checks the unchanged monitor mode and nearest filter.
`--duration 226` tests a full live cycle;
`--offset SECONDS` is a diagnostic timeline offset, not a particle-state seek.
`--capture FILE.bmp` saves the last 520 × 300 framebuffer on exit.
`--capture-display FILE.bmp` captures the actual SDL output, including scaling
and fullscreen bands. `ctest --test-dir native-sdl2/build --output-on-failure`
checks the geometry; `python3 native-sdl2/tools/check_display.py` additionally
checks real display readbacks (requires Pillow).

The engine and `Platform` boundary are prepared for later WebAssembly reuse.
No browser port has been made yet. See the detailed
[preservation notes](../documentation/native-reactivation.md) for class/method
correspondence, arithmetic, resource conversion, timing and validation limits.
