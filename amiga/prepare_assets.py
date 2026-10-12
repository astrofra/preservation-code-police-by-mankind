#!/usr/bin/env python3
"""Convert source PNGs to the small, target-friendly Code Police LZ4 format."""

from pathlib import Path
import struct
import sys

from PIL import Image


def lz4_block(data: bytes) -> bytes:
    """A small greedy LZ4 block encoder; no Python extension is required."""
    output = bytearray()
    positions: dict[bytes, int] = {}
    anchor = 0
    index = 0
    size = len(data)

    def length_bytes(length: int) -> bytes:
        extra = bytearray()
        while length >= 255:
            extra.append(255)
            length -= 255
        extra.append(length)
        return bytes(extra)

    while index + 4 <= size:
        key = data[index:index + 4]
        previous = positions.get(key)
        positions[key] = index
        if previous is None or index - previous > 0xFFFF:
            index += 1
            continue
        match = 4
        while index + match < size and data[previous + match] == data[index + match]:
            match += 1
        literal_length = index - anchor
        match_length = match - 4
        token = min(literal_length, 15) << 4 | min(match_length, 15)
        output.append(token)
        if literal_length >= 15:
            output.extend(length_bytes(literal_length - 15))
        output.extend(data[anchor:index])
        output.extend(struct.pack("<H", index - previous))
        if match_length >= 15:
            output.extend(length_bytes(match_length - 15))
        index += match
        anchor = index
        for position in range(index - match + 1, index):
            if position + 4 <= size:
                positions[data[position:position + 4]] = position

    literal_length = size - anchor
    output.append(min(literal_length, 15) << 4)
    if literal_length >= 15:
        output.extend(length_bytes(literal_length - 15))
    output.extend(data[anchor:])
    return bytes(output)


def convert(source: Path, destination: Path) -> tuple[int, int]:
    count = 0
    total = 0
    for png in sorted((source / "Images").glob("*.png")):
        with Image.open(png) as image:
            rgba = image.convert("RGBA")
            width, height = rgba.size
            if width > 0xFFFF or height > 0xFFFF:
                raise ValueError(f"image too large for CPIM: {png}")
            pixels = bytearray()
            for red, green, blue, alpha in list(rgba.getdata()):
                pixels.extend((alpha, red, green, blue))
        raw = bytes(pixels)
        compressed = lz4_block(raw)
        flags = 1 if len(compressed) < len(raw) else 0
        payload = compressed if flags else raw
        output = destination / "Images" / (png.stem + ".cpil")
        output.parent.mkdir(parents=True, exist_ok=True)
        data = struct.pack(">4sBBHHII", b"CPIL", 1, flags, width, height,
                           len(raw), len(payload)) + payload
        if not output.exists() or output.read_bytes() != data:
            output.write_bytes(data)
        count += 1
        total += len(data)
    return count, total


def main() -> int:
    here = Path(__file__).resolve().parent
    source = Path(sys.argv[1]) if len(sys.argv) > 1 else here.parent / "native-sdl2" / "assets"
    destination = Path(sys.argv[2]) if len(sys.argv) > 2 else here / "build" / "hd" / "assets"
    count, total = convert(source, destination)
    print(f"prepared {count} CPIL/LZ4 images ({total} bytes) in {destination / 'Images'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
