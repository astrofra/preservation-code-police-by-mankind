#!/usr/bin/env python3
"""Derive fixed 16-colour AGA palettes and decide the artistic go/no-go.

Step 1 of the staged plan in documentation/amiga-aga-feasibility.md. The raw
material is the engine's own headless PPM output (native-sdl2/tools/dump_frames.cpp,
which reuses the verified engine exactly as verify.cpp does). This tool never
touches an Amiga and never changes the engine.

Method (§4.2 of the study):
  1. Accumulate a pixel-frequency histogram over the sampled frames.
  2. Add a small bonus for rare saturated colours so sprite accents survive.
  3. Median-cut down to 16 colours, then k-means refinement.
  4. Snap to the AGA 12-bit register space (4 bits per channel) and report the
     error that snapping introduces, because that is what the hardware shows.
  5. Write the palettes, error statistics and side-by-side captures.

The study proposes one palette per script part. This tool also answers the two
questions that decision depends on:
  - a single global palette for the whole demo, to show what per-scene palettes buy;
  - temporal segmentation (N palettes inside one part), because a long part whose
    look changes mid-way is exactly where one fixed palette breaks down. Part 11's
    100-second tunnel is the case in point.

Deterministic: median-cut is exact and k-means is seeded from it with no random
restarts, so repeated runs are byte-identical.
"""
import argparse
import json
import re
import sys
from pathlib import Path

import numpy as np
from PIL import Image

# AGA colour registers are 12-bit: 4 bits per channel for each of 16 entries.
AGA_LEVELS = 16
PALETTE_SIZE = 16
FRAME_RE = re.compile(r"tick-(\d+)-part-(\d+)\.ppm$")


def read_ppm(path):
    """Read a binary P6 PPM written by the exporter (maxval 255)."""
    data = path.read_bytes()
    if not data.startswith(b"P6\n"):
        raise ValueError(f"Not a binary PPM: {path}")
    # Header is exactly "P6\n520 300\n255\n" as written by dump_frames.cpp.
    end = data.index(b"\n", data.index(b"\n", data.index(b"\n") + 1) + 1) + 1
    header = data[:end].split()
    width, height = int(header[1]), int(header[2])
    return np.frombuffer(data, dtype=np.uint8, count=width * height * 3, offset=end).reshape(height, width, 3)


def histogram(frames):
    """5-bit-per-channel histogram with exact per-cell channel sums."""
    counts = np.zeros(1 << 15, dtype=np.int64)
    sums = np.zeros((1 << 15, 3), dtype=np.float64)
    for frame in frames:
        f = frame.reshape(-1, 3).astype(np.int64)
        cell = ((f[:, 0] >> 3) << 10) | ((f[:, 1] >> 3) << 5) | (f[:, 2] >> 3)
        counts += np.bincount(cell, minlength=1 << 15)
        for c in range(3):
            sums[:, c] += np.bincount(cell, weights=f[:, c], minlength=1 << 15)
    occupied = np.nonzero(counts)[0]
    means = sums[occupied] / counts[occupied, None]
    colours = np.clip(np.rint(means), 0, 255).astype(np.uint8)
    return colours, counts[occupied].astype(np.float64)


def saturation_weights(colours, counts, boost):
    """Pixel frequency, with a bonus favouring rare saturated accents."""
    hi = colours.max(axis=1).astype(np.float64)
    lo = colours.min(axis=1).astype(np.float64)
    saturation = (hi - lo) / 255.0
    return counts * (1.0 + boost * saturation)


def weighted_median_split(order, weights):
    """Index into `order` that splits the sorted values at half the weight."""
    cumulative = np.cumsum(weights[order])
    half = cumulative[-1] * 0.5
    cut = int(np.searchsorted(cumulative, half, side="left"))
    return min(max(cut, 1), len(order) - 1)


def median_cut(colours, weights, k):
    boxes = [np.arange(len(colours))]
    while len(boxes) < k:
        best, best_score = None, -1.0
        for i, box in enumerate(boxes):
            if len(box) < 2:
                continue
            cols = colours[box]
            extent = (cols.max(axis=0) - cols.min(axis=0)).astype(np.float64)
            score = weights[box].sum() * (extent.max() + 1.0)
            if score > best_score:
                best, best_score = i, score
        if best is None:
            break
        box = boxes.pop(best)
        cols = colours[box]
        axis = int(np.argmax((cols.max(axis=0) - cols.min(axis=0))))
        order = box[np.argsort(cols[:, axis], kind="stable")]
        cut = weighted_median_split(order, weights)
        boxes.append(order[:cut])
        boxes.append(order[cut:])
    return boxes


def kmeans(colours, weights, boxes, iterations):
    centres = np.array([np.average(colours[b], axis=0, weights=weights[b]) for b in boxes])
    for _ in range(iterations):
        distance = ((colours[:, None, :] - centres[None, :, :]) ** 2).sum(axis=2)
        assign = np.argmin(distance, axis=1)
        moved = False
        for i in range(len(centres)):
            member = assign == i
            if not member.any():
                continue
            updated = np.average(colours[member], axis=0, weights=weights[member])
            if not np.allclose(updated, centres[i]):
                moved = True
            centres[i] = updated
        if not moved:
            break
    return np.clip(np.rint(centres), 0, 255).astype(np.uint8)


def snap_aga(colours):
    """Snap a 24-bit palette to the AGA 12-bit register space."""
    step = 255.0 / (AGA_LEVELS - 1)
    return np.clip(np.rint(np.rint(colours / step) * step), 0, 255).astype(np.uint8)


def nearest_lut(palette):
    """Map every 5-bit cell to its nearest palette index (one 32768-entry LUT)."""
    cells = np.arange(1 << 15)
    r = ((cells >> 10) & 31).astype(np.int32) * 255 // 31
    g = ((cells >> 5) & 31).astype(np.int32) * 255 // 31
    b = (cells & 31).astype(np.int32) * 255 // 31
    rgb = np.stack([r, g, b], axis=1)
    distance = ((rgb[:, None, :].astype(np.int32) - palette[None, :, :].astype(np.int32)) ** 2).sum(axis=2)
    return np.argmin(distance, axis=1).astype(np.uint8)


def hex_colour(c):
    return f"#{int(c[0]):02x}{int(c[1]):02x}{int(c[2]):02x}"


def aga_word(c):
    return int((int(c[0]) >> 4) << 8 | (int(c[1]) >> 4) << 4 | (int(c[2]) >> 4))


def palette_json(palette, raw):
    return dict(
        palette_hex=[hex_colour(c) for c in palette],
        palette_rgb=[[int(v) for v in c] for c in palette],
        palette_aga_0rgb=[aga_word(c) for c in palette],
        palette_hex_24bit=[hex_colour(c) for c in raw],
    )


def derive(frames, boost, iterations):
    """Median-cut seeded k-means, snapped to the AGA 12-bit register space."""
    colours, counts = histogram(frames)
    weights = saturation_weights(colours, counts, boost)
    raw = kmeans(colours, weights, median_cut(colours, weights, PALETTE_SIZE), iterations)
    if len(raw) < PALETTE_SIZE:
        raw = np.vstack([raw, np.zeros((PALETTE_SIZE - len(raw), 3), np.uint8)])
    return raw, snap_aga(raw)


def quantise(frame, palette, lut):
    f = frame.astype(np.int32)
    cell = ((f[:, :, 0] >> 3) << 10) | ((f[:, :, 1] >> 3) << 5) | (f[:, :, 2] >> 3)
    return palette[lut[cell]]


def evaluate(frames, palette):
    """Per-frame and aggregate error of quantising `frames` to `palette`."""
    lut = nearest_lut(palette)
    frame_means, frame_psnr, all_diffs = [], [], []
    for frame in frames:
        diff = np.abs(quantise(frame, palette, lut).astype(np.int32) - frame.astype(np.int32))
        frame_means.append(float(diff.mean()))
        mse = float((diff.astype(np.float64) ** 2).mean())
        frame_psnr.append(float("inf") if mse == 0 else 10.0 * np.log10(255.0 ** 2 / mse))
        all_diffs.append(diff.reshape(-1, 3))
    diffs = np.concatenate(all_diffs, axis=0)
    return dict(
        mean_abs_channel_error=round(float(diffs.mean()), 4),
        worst_pixel_channel_error=int(diffs.max()),
        mean_frame_error=round(float(np.mean(frame_means)), 4),
        worst_frame_error=round(float(np.max(frame_means)), 4),
        mean_psnr_db=round(float(np.mean(frame_psnr)), 2),
    ), [round(v, 3) for v in frame_means]


def capture(path, frame, palette):
    restored = quantise(frame, palette, nearest_lut(palette))
    strip = np.repeat(np.repeat(palette[:, None, :], 12, axis=0), frame.shape[1], axis=1)
    stack = np.vstack([frame, np.zeros((4, frame.shape[1], 3), np.uint8), restored, strip])
    path.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(stack).save(path)


def segments_of(indexes, count):
    """Contiguous, near-equal chunks of a frame list, in tick order."""
    return [list(chunk) for chunk in np.array_split(np.arange(indexes), count)]


def screen_statistics(frames):
    """Rasteriser load and colour richness, measured on the source frames.

    Fill fraction is the share of pixels that are not near-black: it bounds the
    per-frame rasterisation work, so it is the honest input to the performance
    budget rather than an assumed fill. Distinct colours shows how far the
    source is from the 16-colour target, which is the quantisation difficulty.
    """
    fills, distinct = [], []
    for frame in frames:
        f = frame.reshape(-1, 3)
        fills.append(float((f.max(axis=1) > 8).mean()))
        distinct.append(int(np.unique(f, axis=0).shape[0]))
    return dict(
        mean_fill_fraction=round(float(np.mean(fills)), 4),
        max_fill_fraction=round(float(np.max(fills)), 4),
        distinct_colours_mean=round(float(np.mean(distinct)), 1),
        distinct_colours_max=int(np.max(distinct)),
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("frames", type=Path, help="directory of tick-*-part-*.ppm frames")
    parser.add_argument("--output", type=Path, default=Path("reference/amiga-aga"))
    parser.add_argument("--saturation-boost", type=float, default=0.5)
    parser.add_argument("--kmeans-iterations", type=int, default=24)
    parser.add_argument("--segments", type=int, default=1,
                        help="temporal palettes inside each part (1 = the study's one per part)")
    parser.add_argument("--evidence", type=Path,
                        help="write a curated machine-readable summary here (evidence/)")
    parser.add_argument("--no-captures", action="store_true")
    args = parser.parse_args()

    by_part = {}
    for path in sorted(args.frames.glob("tick-*-part-*.ppm")):
        match = FRAME_RE.search(path.name)
        if match:
            by_part.setdefault(int(match.group(2)), []).append((int(match.group(1)), path))
    if not by_part:
        raise SystemExit(f"No tick-*-part-*.ppm frames in {args.frames}")

    loaded = {part: [(tick, read_ppm(path)) for tick, path in sorted(entries)]
              for part, entries in by_part.items()}
    every_frame = [frame for entries in loaded.values() for _, frame in entries]

    # One palette for the whole demo, for comparison against the per-scene choice.
    global_raw, global_palette = derive(every_frame, args.saturation_boost, args.kmeans_iterations)

    compare_dir = args.output / "compare"
    parts, summary = {}, []
    for part in sorted(loaded):
        entries = loaded[part]
        ticks = [tick for tick, _ in entries]
        frames = [frame for _, frame in entries]
        whole_raw, whole = derive(frames, args.saturation_boost, args.kmeans_iterations)
        whole_stats, whole_frames = evaluate(frames, whole)
        global_stats, _ = evaluate(frames, global_palette)

        segment_records, weights = [], []
        for index, members in enumerate(segments_of(len(frames), args.segments)):
            member_frames = [frames[i] for i in members]
            raw, palette = derive(member_frames, args.saturation_boost, args.kmeans_iterations)
            segment_stats, _ = evaluate(member_frames, palette)
            segment_records.append(dict(
                index=index, ticks=[ticks[members[0]], ticks[members[-1]]],
                **palette_json(palette, raw), statistics=segment_stats))
            weights.append(len(members))
        segmented_error = round(float(np.average(
            [s["statistics"]["mean_abs_channel_error"] for s in segment_records], weights=weights)), 4)

        parts[part] = dict(
            **palette_json(whole, whole_raw),
            statistics=whole_stats,
            screen=screen_statistics(frames),
            segments=segment_records,
            global_comparison=global_stats,
            improvement=dict(
                whole_mean_error=whole_stats["mean_abs_channel_error"],
                segmented_mean_error=segmented_error,
                global_mean_error=global_stats["mean_abs_channel_error"],
                segments=args.segments,
            ),
        )
        summary.append(dict(part=part, frames=len(frames),
                            whole=whole_stats["mean_abs_channel_error"],
                            global_=global_stats["mean_abs_channel_error"],
                            segmented=segmented_error,
                            psnr=whole_stats["mean_psnr_db"],
                            worst_pixel=whole_stats["worst_pixel_channel_error"]))

        if not args.no_captures:
            # A representative frame and the worst frame, both against the whole-part palette.
            worst = int(np.argmax(whole_frames))
            for label, idx in (("sample", len(frames) // 2), ("worst", worst)):
                capture(compare_dir / f"part-{part:02d}-{label}-tick-{ticks[idx]:05d}.png",
                        frames[idx], whole)

    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "palettes.json").write_text(json.dumps(dict(
        palette_size=PALETTE_SIZE, aga_levels_per_channel=AGA_LEVELS,
        source="native-sdl2/tools/dump_frames.cpp (verified engine, seed 888)",
        settings=dict(saturation_boost=args.saturation_boost,
                      kmeans_iterations=args.kmeans_iterations, segments_per_part=args.segments),
        global_palette=dict(**palette_json(global_palette, global_raw),
                            statistics=evaluate(every_frame, global_palette)[0]),
        parts=parts, summary=summary,
    ), indent=2) + "\n")

    print(f"{len(parts)} parts, {args.segments} segment(s) each; palettes written to {args.output/'palettes.json'}")
    print(f"{'part':>4} {'frames':>6} {'whole':>8} {'global':>8} {'segmented':>10} {'PSNR dB':>8} {'worst px':>9}")
    for s in summary:
        print(f"{s['part']:>4} {s['frames']:>6} {s['whole']:>8} {s['global_']:>8} "
              f"{s['segmented']:>10} {s['psnr']:>8} {s['worst_pixel']:>9}")

    if args.evidence:
        # Curated summary: what the go/no-go needs, without the full palettes
        # (those stay in reference/amiga-aga/palettes.json).
        args.evidence.parent.mkdir(parents=True, exist_ok=True)
        all_ticks = sorted(tick for entries in loaded.values() for tick, _ in entries)
        stride = min((b - a for a, b in zip(all_ticks, all_ticks[1:]) if b > a), default=0)
        args.evidence.write_text(json.dumps(dict(
            method="median-cut seeded k-means, snapped to AGA 12-bit registers",
            source="native-sdl2/tools/dump_frames.cpp, verified engine, seed 888",
            frame_sampling=dict(stride_ticks=stride, tick_unit_ms=10,
                                sampled_frames=len(all_ticks),
                                tick_range=[all_ticks[0], all_ticks[-1]]),
            frames_per_part={str(k): len(v) for k, v in sorted(loaded.items())},
            palette_size=PALETTE_SIZE, segments_per_part=args.segments,
            global_mean_abs_channel_error=round(float(np.mean(
                [s["global_"] for s in summary[2:]])), 4),
            per_part_mean_abs_channel_error_whole=round(float(np.mean(
                [s["whole"] for s in summary[2:]])), 4),
            global_palette_hex=palette_json(global_palette, global_raw)["palette_hex"],
            summary=summary,
        ), indent=2) + "\n")
        print(f"Evidence:    {args.evidence}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
