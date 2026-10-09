#!/bin/sh
set -eu
CODEPOLICE_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
exec "$CODEPOLICE_ROOT/wasm/build.sh" "$@"
