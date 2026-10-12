#!/usr/bin/env python3
"""Build the native indexed assets for the Amiga runtime (milestone N1).

documentation/amiga-native-runtime.md §4 and §9.N1. This tool is the only place
that ever touches a PNG or a 32-bit pixel. It produces one CPIX file
(amiga/src/agart.h): a single global 16-colour AGA palette plus one indexed
4-bit texture per name the script uses.

Inputs are all offline and already in the repository:
  - the verified engine's headless frames, native-sdl2/tools/dump_frames.cpp
    output (tick-NNNNN-part-NN.ppm), used only to derive the palette;
  - the script, native-sdl2/assets/scene.txt, for the texture list and the role
    (opaque or additive) of each use;
  - the source textures, native-sdl2/assets/Images/*.png.

Nothing here is imported by the Amiga runtime. The runtime reads CPIX only.

Design decisions this tool implements (measured, not assumed):
  - one palette for the whole demo (the user's choice): it costs some fidelity
    against per-scene palettes, but removes the per-part texture variants, and a
    single palette lets every texture be stored once.
  - the transparency key in the engine is the exact ARGB 0xFFFFFFFF; the opaque
    operators skip it. It maps to index 15.
  - index 0 is black, the clear colour and the neutral element of the additive
    pair tables.
  - a texture's role is fixed across the visible parts (part 0 is the black load
    window and is ignored); the roles come out consistent, which a single global
    texture cannot otherwise assume. Additive textures (flare3d, particle,
    twirl, tunnel, ground) are not keyed and may use index 15 as white.

Usage:
  build_assets.py FRAMES ASSETS OUTPUT [--max WxH] [--report FILE]
"""
import argparse
import json
import re
import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "native-sdl2" / "tools"))
from amiga_palette import (  # noqa: E402  (reuse the study's derived machinery)
    histogram, median_cut, kmeans, nearest_lut, saturation_weights, snap_aga,
)

USEFUL = 14            # indices 1..14
BLACK_INDEX = 0
KEY_INDEX = 15
FRAME_RE = re.compile(r"tick-(\d+)-part-(\d+)\.ppm$")
# Operators, mapped to the role of the texture they draw. The opaque ones key on
# 0xFFFFFFFF (drawrecUVS); the additive ones add and clamp (drawrecUVRGB/UVM).
OPAQUE_OPS = {"sprit", "sprit3d"}
ADDITIVE_OPS = {"flare3d", "particle", "twirl", "tunnel", "ground"}


def read_ppm(path):
    data = path.read_bytes()
    end = data.index(b"\n", data.index(b"\n", data.index(b"\n") + 1) + 1) + 1
    header = data[:end].split()
    width, height = int(header[1]), int(header[2])
    return np.frombuffer(data, np.uint8, width * height * 3, offset=end).reshape(height, width, 3)


def parse_script(text):
    """Part list with the (texture, role) of every IMA| used, in script order.

    Also returns the set of (part, texture) pairs used in more than one role:
    such a pair cannot share one packed texture, so the tool must refuse rather
    than silently pick the first role seen.
    """
    images = {n.strip(): f.strip().split("/")[-1]
              for n, f in re.findall(r"<KIMAGES>([^|<\s]+)\s*\|([^<]+)</KIMAGES>", text)}
    parts = []
    conflicts = []
    for i, body in enumerate(re.findall(r"<KPART>(.*?)</KPART>", text, re.S)):
        used = {}
        for effect in re.findall(r"<Fx>(.*?)</Fx>", body, re.S):
            pas = re.findall(r"<Pa>([^<]*)</Pa>", effect)
            if not pas:
                continue
            op = pas[0].strip().lower()
            role = 0 if op in OPAQUE_OPS else (1 if op in ADDITIVE_OPS else None)
            if role is None:
                continue
            for p in pas[1:]:
                if p.strip().lower().startswith("ima|"):
                    name = p.split("|", 1)[1].strip()
                    if name in used and used[name] != role:
                        conflicts.append((i, name, used[name], role))
                    used[name] = role
        parts.append(used)
    return images, parts, conflicts


def derive_palette(frames):
    """14 useful colours, then the reserved black and key white, snapped to AGA."""
    colours, counts = histogram(frames)
    weights = saturation_weights(colours, counts, 0.5)
    raw = kmeans(colours, weights, median_cut(colours, weights, USEFUL), 24)
    useful = snap_aga(raw)
    if len(useful) < USEFUL:                      # a nearly flat scene
        useful = np.vstack([useful, np.zeros((USEFUL - len(useful), 3), np.uint8)])
    palette = np.vstack([np.zeros((1, 3), np.uint8), useful,
                         np.full((1, 3), 255, np.uint8)])
    return palette


def visible_roles(parts, images):
    """One role per texture, from the visible parts only (part 0 is invisible).

    Part 0 is the black load window; what it "draws" is never seen, so its role
    assignment is noise. Restricting to parts 1.. makes the role of every texture
    unique, which is what a single global texture set needs.
    """
    roles = {}
    for i, used in enumerate(parts):
        if i == 0:
            continue
        for name, role in used.items():
            if name in roles and roles[name] != role:
                raise SystemExit(f"role conflict for {name!r} in visible part {i}: "
                                 f"{roles[name]} vs {role}")
            roles[name] = role
    missing = [n for n in roles if n not in images]
    if missing:
        raise SystemExit(f"script names without a file: {missing}")
    return roles


def quantise_texture(rgb, alpha, role, lut14, palette):
    """One packed 4bpp indexed texture. Role picks the key mapping (see agart.h).

    Returns the packed bytes, the row stride, the key mask and the mean absolute
    per-channel error introduced by quantising this texture to the palette.
    """
    height, width = rgb.shape[:2]
    f = rgb.astype(np.int32)
    cell = ((f[:, :, 0] >> 3) << 10) | ((f[:, :, 1] >> 3) << 5) | (f[:, :, 2] >> 3)
    idx = lut14[cell].astype(np.uint8)     # nearest of the 14 useful colours
    key = np.zeros_like(idx, bool)
    if role == 0:
        key = (alpha == 255) & (rgb[:, :, 0] == 255) & (rgb[:, :, 1] == 255) & (rgb[:, :, 2] == 255)
        idx = np.where(key, KEY_INDEX, idx).astype(np.uint8)
    restored = palette.reshape(16, 3)[idx].astype(np.int32)
    err = float(np.abs(restored - rgb.astype(np.int32)).mean())
    stride = (width + 1) // 2                     # packed bytes per row
    if width & 1:                                  # pad the row to a whole byte
        idx = np.pad(idx, ((0, 0), (0, 1)))
    packed = (idx[:, 0::2] << 4) | idx[:, 1::2]
    assert packed.shape == (height, stride), (packed.shape, stride)
    return packed.tobytes(), stride, key, err


def write_pack(path, palette, entries):
    """entries: list of (name, role, flags, width, height, stride, pixels_bytes).

    One file holds the single global palette and one texture per name. Layout
    must match amiga/src/agart.h exactly: 56-byte header, 32-byte entries.
    """
    head = 56 + len(entries) * 32
    offsets, blob, out = [], bytearray(), bytearray()
    for name, role, flags, w, h, stride, pixels in entries:
        offsets.append((head + len(blob), len(pixels)))
        blob += pixels
    out += b"CPIX" + struct.pack(">BBH", 1, 0, len(entries))
    out += bytes(palette.reshape(-1).astype(np.uint8))
    for (name, role, flags, w, h, stride, _), (off, size) in zip(entries, offsets):
        out += name.encode()[:15].ljust(16, b"\0")
        out += struct.pack(">BBHHHII", role, flags, w, h, stride, off, size)
    out += blob
    path.write_bytes(bytes(out))
    return len(out)


def frame_error(frames, palette):
    """Mean per-channel error and PSNR of quantising the sampled frames."""
    lut = nearest_lut(palette)
    diffs, psnr = [], []
    for frame in frames:
        f = frame.astype(np.int32)
        cell = ((f[:, :, 0] >> 3) << 10) | ((f[:, :, 1] >> 3) << 5) | (f[:, :, 2] >> 3)
        restored = palette[lut[cell]].astype(np.int32)
        d = np.abs(restored - frame.astype(np.int32))
        diffs.append(d)
        mse = float((d.astype(np.float64) ** 2).mean())
        psnr.append(float("inf") if mse == 0 else 10.0 * np.log10(255.0 ** 2 / mse))
    all_diffs = np.concatenate([d.reshape(-1, 3) for d in diffs], axis=0)
    return dict(mean_abs_channel_error=round(float(all_diffs.mean()), 4),
                worst_pixel_channel_error=int(all_diffs.max()),
                mean_psnr_db=round(float(np.mean([p for p in psnr if np.isfinite(p)])), 2))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("frames")
    ap.add_argument("assets")
    ap.add_argument("output")
    ap.add_argument("--max", default="320x200", help="cap texture size, or 'none'")
    ap.add_argument("--report")
    args = ap.parse_args()

    frames_dir = Path(args.frames)
    assets = Path(args.assets)
    out_dir = Path(args.output)
    out_dir.mkdir(parents=True, exist_ok=True)

    script = (assets / "scene.txt").read_text("latin-1")
    images, parts, conflicts = parse_script(script)
    if conflicts:
        print("texture role conflict within a part, refusing:", conflicts, file=sys.stderr)
        return 1
    roles = visible_roles(parts, images)

    cap = None
    if args.max != "none":
        w, h = (int(v) for v in args.max.split("x"))
        cap = (w, h)

    # One global palette, derived from every visible part's sampled frames.
    frames = []
    for p in sorted(frames_dir.glob("*.ppm")):
        m = FRAME_RE.search(p.name)
        if m and int(m.group(2)) != 0:
            frames.append(read_ppm(p))
    if not frames:
        print(f"no frame dumps in {frames_dir}; run native-sdl2/tools/dump_frames.cpp first",
              file=sys.stderr)
        return 1
    palette = derive_palette(frames)
    lut = nearest_lut(palette)

    entries = []
    errors = []
    total = 0
    for name in sorted(roles):
        role = roles[name]
        src = Image.open(assets / "Images" / (images[name] + ".png")).convert("RGBA")
        if cap and (src.width > cap[0] or src.height > cap[1]):
            src.thumbnail(cap, Image.LANCZOS)
        arr = np.asarray(src).astype(np.uint32)
        rgb = arr[:, :, :3].astype(np.uint8)
        alpha = arr[:, :, 3].astype(np.uint8)
        pixels, stride, key, err = quantise_texture(rgb, alpha, role, lut, palette)
        errors.append(err)
        total += len(pixels)
        entries.append((name, role, 1 if (role == 0 and key.any()) else 0,
                        src.width, src.height, stride, pixels))
        print(f"  {name:10s} role={'opaque ' if role == 0 else 'additive'} "
              f"{src.width:4d}x{src.height:<4d} {len(pixels):7d} B  err {err:5.2f}/255")

    path = out_dir / "assets.cpix"
    size = write_pack(path, palette, entries)
    fe = frame_error(frames, palette)
    print(f"--- palette (AGA 12-bit):")
    for i in range(16):
        c = palette[i]
        print(f"    {i:2d}  #{c[0]:02x}{c[1]:02x}{c[2]:02x}")
    print(f"--- {len(entries)} textures, {size} B = {size/1024:.0f} KiB in {path}")
    print(f"--- global frame error: mean {fe['mean_abs_channel_error']}/255, "
          f"worst pixel {fe['worst_pixel_channel_error']}, PSNR {fe['mean_psnr_db']} dB")
    if args.report:
        report = {"palette_rgb": [[int(v) for v in c] for c in palette],
                  "textures": len(entries),
                  "texture_error_mean": round(float(np.mean(errors)), 3),
                  "bytes": size, "frame_error": fe,
                  "parts": [len(p) for p in parts]}
        Path(args.report).write_text(json.dumps(report, indent=1))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
