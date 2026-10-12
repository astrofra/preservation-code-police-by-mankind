#!/bin/sh
# Build and launch the Code Police AGA host in FS-UAE.
set -eu

project=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
root=$(CDPATH= cd -- "$project/.." && pwd)
config=${FSUAE_CONFIG:-"$project/build/code-police-aga.fs-uae"}
fsuae=${FSUAE_BIN:-fs-uae}

if ! command -v "$fsuae" >/dev/null 2>&1; then
    echo "error: cannot find FS-UAE ('$fsuae')" >&2
    exit 1
fi
if [ ! -f "$config" ]; then
    echo "error: FS-UAE config not found: $config" >&2
    exit 1
fi

if [ "${BUILD:-1}" = "1" ]; then
    python3 "$project/prepare_assets.py" "$root/native-sdl2/assets" "$project/build/hd/assets"
    RUN=0 CPU="${CPU:-68020}" "$project/build.sh"
fi

exec "$fsuae" "$config"
