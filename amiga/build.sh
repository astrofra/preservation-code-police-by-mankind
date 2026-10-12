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

set -- -std=c99 -m"$cpu" -O2 -Wall -Wextra -Werror -fno-common -ffp-contract=off \
    -Wno-unused-function \
    -I "$root/native-sdl2/vendor/stb" -I "$project/src" \
    "$project/src/main.c" "$project/src/amiga_platform.c" \
    -o "$build/code-police"

printf '> m68k-amigaos-gcc -m%s%s\n' "$cpu" " $runtime"
m68k-amigaos-gcc "$@" "$runtime"

file "$build/code-police"

if [ "${RUN:-1}" = "1" ]; then
    # Ki-stack: zlib inside stb_image needs far more than the AmigaOS default.
    printf '\n> vamos -C 20 -s 256 (cwd=%s)\n' "$root"
    ( cd "$root" && vamos -C 20 -s 256 "$build/code-police" native-sdl2/assets Images/CARE.gif.png )
fi
