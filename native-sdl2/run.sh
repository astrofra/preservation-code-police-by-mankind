#!/bin/sh
set -eu
project=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ ! -x "$project/build/code-police" ]; then
    "$project/build.sh"
fi
exec "$project/build/code-police" "$@"
