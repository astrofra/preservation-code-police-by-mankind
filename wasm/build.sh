#!/bin/sh
set -eu
CODEPOLICE_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
CODEPOLICE_EMSDK=${CODEPOLICE_EMSDK:-"$CODEPOLICE_ROOT/wasm/build/emsdk"}
if ! command -v emcc >/dev/null 2>&1 || ! emcc --version | head -1 | grep -q ' 4.0.23 '; then
    if [ ! -f "$CODEPOLICE_EMSDK/emsdk_env.sh" ]; then
        mkdir -p "$(dirname -- "$CODEPOLICE_EMSDK")"
        git clone --depth 1 --branch 4.0.23 https://github.com/emscripten-core/emsdk.git "$CODEPOLICE_EMSDK"
    fi
    if [ ! -f "$CODEPOLICE_EMSDK/.emscripten" ]; then
        "$CODEPOLICE_EMSDK/emsdk" install 4.0.23
        "$CODEPOLICE_EMSDK/emsdk" activate 4.0.23
    fi
    set +u
    . "$CODEPOLICE_EMSDK/emsdk_env.sh" >/dev/null
    set -u
fi
if ! emcc --version | head -1 | grep -q ' 4.0.23 '; then
    printf 'This build requires Emscripten 4.0.23 (CODEPOLICE_EMSDK).\n' >&2
    exit 1
fi
emcmake cmake -S "$CODEPOLICE_ROOT/wasm" -B "$CODEPOLICE_ROOT/wasm/build/cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build "$CODEPOLICE_ROOT/wasm/build/cmake" --parallel
python3 "$CODEPOLICE_ROOT/wasm/package.py"
