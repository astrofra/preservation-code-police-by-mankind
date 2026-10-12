#!/usr/bin/env python3
"""Check the CPIX part files and render a visual contact sheet (milestone N1).

Independent of the C loader: this walks the format from amiga/src/agart.h and
fails on any inconsistency, then reconstructs every indexed texture to RGB from
its part palette and writes a contact sheet so the 16-colour result can be eyeballed
against the AGA references in reference/amiga-aga/.

It also reports the reserved indices actually used, because index 0 (black) and
index 15 (transparency key) are load-bearing: a texture that never uses 15 in an
opaque role has no transparency, and additive textures may use 15 as white.
"""
import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image

MAGIC = b"CPIX"
HEADER = 56
ENTRY = 32


def load(path):
    data = path.read_bytes()
    assert data[:4] == MAGIC, f"{path}: bad magic"
    assert data[4] == 1, f"{path}: bad version {data[4]}"
    count = struct.unpack_from(">H", data, 6)[0]
    palette = np.frombuffer(data, np.uint8, 48, 8).reshape(16, 3).copy()
    end = HEADER + count * ENTRY
    assert end <= len(data), f"{path}: entries overrun the file"
    entries = []
    for i in range(count):
        o = HEADER + i * ENTRY
        name = data[o:o + 16].split(b"\0", 1)[0].decode()
        role, flags = data[o + 16], data[o + 17]
        w, h, stride = struct.unpack_from(">HHH", data, o + 18)
        off, size = struct.unpack_from(">II", data, o + 24)
        assert off >= end and off + size <= len(data), f"{path}: {name} out of bounds"
        assert size == stride * h, f"{path}: {name} size {size} != stride*height"
        assert stride >= (w + 1) // 2, f"{path}: {name} stride too small"
        entries.append(dict(name=name, role=role, flags=flags, w=w, h=h,
                            stride=stride, off=off, size=size))
    return palette, entries, data


def decode(data, t):
    raw = np.frombuffer(data, np.uint8, t["size"], t["off"]).reshape(t["h"], t["stride"])
    idx = np.empty((t["h"], t["stride"] * 2), np.uint8)
    idx[:, 0::2] = raw >> 4
    idx[:, 1::2] = raw & 0x0F
    return idx[:, :t["w"]]


def main():
    directory = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("amiga/build/native-assets")
    output = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("amiga/build/native-assets/sheet.png")
    files = sorted(directory.glob("*.cpix"))
    assert files, f"no CPIX files in {directory}"
    tiles, problems = [], []
    for f in files:
        palette, entries, data = load(f)
        print(f"{f.name}: {len(entries)} textures, {len(data)} B")
        for t in entries:
            idx = decode(data, t)
            rgb = palette[idx]
            if t["role"] == 0:
                key = idx == 15
                rgb = np.where(key[:, :, None], 200, rgb)   # show the key as grey 200
            tiles.append((t["name"], rgb))
            used = np.unique(idx)
            if 0 not in used and t["role"] == 0:
                problems.append(f"{f.name}/{t['name']}: opaque texture uses no index 0 (black)")
    # Contact sheet: scale each tile to 96 px wide, pad, stack in a grid.
    cell = 96
    cols = 6
    rows = (len(tiles) + cols - 1) // cols
    sheet = np.full((rows * (cell + 18), cols * cell, 3), 20, np.uint8)
    for k, (name, rgb) in enumerate(tiles):
        im = Image.fromarray(rgb).convert("RGB")
        scale = cell / im.width
        im = im.resize((cell, max(1, int(im.height * scale))), Image.NEAREST)
        a = np.asarray(im)[:cell]
        r, c = divmod(k, cols)
        sheet[r * (cell + 18):r * (cell + 18) + a.shape[0], c * cell:c * cell + a.shape[1]] = a
    Image.fromarray(sheet).save(output)
    print(f"--- contact sheet: {output} ({len(tiles)} textures)")
    for p in problems:
        print("WARN", p)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
