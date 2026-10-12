#!/usr/bin/env python3
"""Validate the C KarateScript parser against the repository's ground truth.

Runs the m68k port under vamos, parses its output, and compares the counts and
part timings with evidence/diagnostic.json (the static inventory of the
original script). This is the milestone-2 gate: the C tokeniser must agree with
the verified engine on every field before the object graph is ported.
"""
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "amiga" / "build" / "68020" / "code-police"
ASSETS = ROOT / "native-sdl2" / "assets"


def run():
    result = subprocess.run(
        ["vamos", "-C", "20", "-s", "256", "-m", "8192", str(BINARY), "native-sdl2/assets"],
        cwd=ROOT, capture_output=True, text=True, timeout=120,
    )
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr)
        raise SystemExit(f"port exited {result.returncode}")
    return result.stdout


def parse(text):
    header = re.search(r"script: images=(\d+) sounds=(\d+) tables=(\d+) ticles=(\d+) "
                       r"cams=(\d+) rects=(\d+) parts=(\d+) loop=(\d+)", text)
    assert header, "no script header in output:\n" + text
    keys = ("images", "sounds", "tables", "ticles", "cams", "rects", "parts", "loop")
    script = {k: int(v) for k, v in zip(keys, header.groups())}
    parts = []
    for line in re.finditer(r"part (\d+): len=(\d+) start=(\d+) sounds=(\d+) effects=(\d+)", text):
        parts.append(dict(index=int(line.group(1)), length=int(line.group(2)),
                          start=int(line.group(3)), sounds=int(line.group(4)),
                          effects=int(line.group(5))))
    script["parts_detail"] = parts
    return script


def main():
    diagnostic = json.loads((ROOT / "evidence" / "diagnostic.json").read_text())
    actual = parse(run())
    failures = []

    def check(name, got, want):
        ok = got == want
        print(f"{'ok  ' if ok else 'FAIL'} {name}: got {got}, want {want}")
        if not ok:
            failures.append(name)

    check("image declarations", actual["images"], diagnostic["script_image_declarations"])
    check("sound declarations", actual["sounds"], diagnostic["script_sound_declarations"])
    check("parts", actual["parts"], len(diagnostic["parts"]))

    # Part durations are stored in ticks (10 ms); the diagnostic uses seconds.
    want_durations = [round(p["duration_seconds"] * 100) for p in diagnostic["parts"]]
    got_durations = [p["length"] for p in actual["parts_detail"]]
    check("part durations (ticks)", got_durations, want_durations)

    # The per-part effect count is the sum of the diagnostic's effect histogram.
    want_effects = [sum(p["effects"].values()) for p in diagnostic["parts"]]
    got_effects = [p["effects"] for p in actual["parts_detail"]]
    check("per-part effect counts", got_effects, want_effects)

    # The diagnostic calls the 1600-tick local offset "local_offset_seconds".
    want_offsets = [round(p["local_offset_seconds"] * 100) for p in diagnostic["parts"]]
    got_offsets = [p["start"] for p in actual["parts_detail"]]
    check("per-part local offsets (ticks)", got_offsets, want_offsets)

    if failures:
        print(f"\nFAILED: {', '.join(failures)}")
        return 1
    print("\nPASS: the C parser matches diagnostic.json on every checked field.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
