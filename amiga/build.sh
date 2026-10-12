#!/bin/sh
# Build the Code Police Amiga port and run it.
#
# The AmigaPorts m68k toolchain and the vamos runner live off PATH; this script
# puts them on PATH for the build only. Use CPU=68060 for the real target (build
# only; vamos executes 68020, use FS-UAE for 68060).
set -eu
project=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
root=$(CDPATH= cd -- "$project/.." && pwd)
toolchain="${M68K_AMIGAOS_PREFIX:-$HOME/.local/m68k-amigaos}"
cpu="${CPU:-68020}"
runtime="${RUNTIME:--mcrt=nix20}"

PATH="$toolchain/bin:$HOME/.local/bin:$PATH"
export PATH

build="$project/build/$cpu"
mkdir -p "$build"

common="$project/src/main.c $project/src/amiga_platform.c $project/src/kscript.c
    $project/src/renderer.c $project/src/render_check.c $project/src/objects.c
    $project/src/c2p4.c $project/src/c2p_check.c $project/src/aga_quant.c
    $project/src/aga_preview.c $project/src/amiga_screen.c"

# The headless binary runs under vamos (no graphics.library) and is what the
# differential validators use.
printf '> m68k-amigaos-gcc -m%s (headless)%s\n' "$cpu" " $runtime"
m68k-amigaos-gcc -std=c99 -m"$cpu" -O2 -Wall -Wextra -Werror -fno-common \
    -ffp-contract=off -Wno-unused-function \
    -I "$root/native-sdl2/vendor/stb" -I "$project/src" \
    $common -o "$build/code-police" "$runtime" -lm

# The AGA binary opens a real 320x200 four-plane screen; run it under FS-UAE.
printf '> m68k-amigaos-gcc -m%s -DAGA_SCREEN%s\n' "$cpu" " $runtime"
m68k-amigaos-gcc -std=c99 -m"$cpu" -O2 -fno-common -ffp-contract=off \
    -Wno-unused-function -DAGA_SCREEN \
    -I "$root/native-sdl2/vendor/stb" -I "$project/src" \
    $common -o "$build/code-police-aga" "$runtime" -lm

# The checked-in FS-UAE configuration boots this folder directly. Keep its
# executable in sync with the freshly built one; otherwise FS-UAE silently
# runs a stale screen host from an earlier bring-up attempt.
if [ -d "$project/build/hd" ]; then
    cp "$build/code-police-aga" "$project/build/hd/code-police-aga"
fi

file "$build/code-police" "$build/code-police-aga"

if [ "${RUN:-1}" = "1" ]; then
    # Ki-stack: zlib inside stb_image needs far more than the AmigaOS default.
    # -m 8192: 8 MiB, since the 520x300 grid and buffers exceed the 1 MiB vamos
    # default (-m is in KiB, and above 8 MiB the memory map with hardware access
    # enabled refuses).
    printf '\n> vamos -C 20 -s 256 -m 8192 (cwd=%s)\n' "$root"
    ( cd "$root" && vamos -C 20 -s 256 -m 8192 "$build/code-police" native-sdl2/assets "${MODE:-}" )
fi
