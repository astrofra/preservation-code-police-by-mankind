#!/bin/sh
set -eu
project=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cmake -S "$project" -B "$project/build" -DCMAKE_BUILD_TYPE=Release "$@"
cmake --build "$project/build" --parallel
