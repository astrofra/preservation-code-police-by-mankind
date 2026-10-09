#!/bin/sh
set -eu
CODEPOLICE_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
CODEPOLICE_SITE="$CODEPOLICE_ROOT/dist/wasm/code-police"
if [ ! -f "$CODEPOLICE_SITE/index.html" ]; then
    printf 'Build the site first: %s/build-wasm.sh\n' "$CODEPOLICE_ROOT" >&2
    exit 1
fi
exec python3 -m http.server "${1:-8000}" --bind 127.0.0.1 --directory "$CODEPOLICE_SITE"
