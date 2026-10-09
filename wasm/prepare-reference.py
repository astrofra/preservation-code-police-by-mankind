#!/usr/bin/env python3
"""Condense the original-bytecode export; never generate expectations from WASM."""
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
source = ROOT / 'native-sdl2/build/reference.bin'
if not source.exists():
    raise SystemExit('Run ./native-sdl2/verify.sh once to export the original Java reference.')
frames = []
with source.open('rb') as stream:
    def integer(n):
        value = stream.read(n)
        if len(value) != n:
            raise ValueError('Truncated reference')
        return int.from_bytes(value, 'big')

    def string():
        return stream.read(integer(2)).decode('utf-8')

    assert integer(4) == 0x43505231
    for _ in range(integer(4)):
        tick, pixel_hash, size = integer(4), integer(8), integer(4)
        state = stream.read(8 * size)
        events = [[string(), string()] for _ in range(integer(4))]
        item = dict(tick=tick, hash=f'{pixel_hash:016x}', events=events)
        if integer(1):
            pixels = stream.read(520 * 300 * 4)  # Original opaque ARGB big endian.
            rgba = bytearray(len(pixels))
            rgba[0::4], rgba[1::4], rgba[2::4], rgba[3::4] = pixels[1::4], pixels[2::4], pixels[3::4], pixels[0::4]
            item['rgba_sha256'] = hashlib.sha256(rgba).hexdigest()
            item['state'] = list(struct.unpack(f'>{size}d', state))
        frames.append(item)
pcm = {}
for p in sorted((ROOT / 'native-sdl2/build/pcm').glob('*.s16be')):
    raw = p.read_bytes()
    little = bytearray(len(raw))
    little[0::2], little[1::2] = raw[1::2], raw[0::2]
    pcm[p.name.removesuffix('.s16be')] = hashlib.sha256(little).hexdigest()
dest = ROOT / 'wasm/build/reference.json'
dest.parent.mkdir(parents=True, exist_ok=True)
dest.write_text(json.dumps(dict(source='Original Java bytecode, seed 888; frame seed 100000+tick', frames=frames, pcm=pcm), separators=(',', ':')))
print(f'Original reference: {len(frames)} frame hashes, {sum("state" in f for f in frames)} full states/pixels, {len(pcm)} PCM decodes')
