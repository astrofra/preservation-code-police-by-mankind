#!/bin/sh
# Step 1 of the Amiga AGA port plan (documentation/amiga-aga-feasibility.md):
# derive 16-colour palettes from the verified engine's own headless output.
# No Amiga, no emulator, no ASM. Reuses the engine exactly as verify.sh does.
set -eu
project=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
root=$(CDPATH= cd -- "$project/.." && pwd)
stride=${STRIDE:-40}
segments=${SEGMENTS:-1}

cmake -S "$project" -B "$project/build" >/dev/null
cmake --build "$project/build" --target dump-frames --parallel >/dev/null

frames="$project/build/amiga-aga-frames"
mkdir -p "$frames"
"$project/build/dump-frames" "$project/assets" "$frames" "$stride"

python3 "$project/tools/amiga_palette.py" "$frames" \
    --output "$root/reference/amiga-aga" --segments "$segments" \
    --evidence "$root/evidence/amiga-aga/palettes.json"

echo
echo "Palettes:   reference/amiga-aga/palettes.json"
echo "Comparisons: reference/amiga-aga/compare/*.png (source over 16-colour, plus palette strip)"
echo "Evidence:    evidence/amiga-aga/palettes.json"
echo "Set SEGMENTS=N to try N temporal palettes per part; STRIDE sets the tick sampling."
