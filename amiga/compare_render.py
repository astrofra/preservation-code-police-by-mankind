#!/usr/bin/env python3
"""Differential check: the C renderer port must match the C++ oracle exactly.

Runs native-sdl2/tools/render_oracle.cpp (the verified engine, built for the
host) and the C port (built for m68k, run under vamos), then compares their
hashes for Position3D, MotionBlur, findtunnel, findfloor, findtwirl, SetzoomP
and flare3D. Identical output means the C renderer is bit-exact with the engine
it must reproduce.
"""
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ORACLE = ROOT / "native-sdl2" / "build" / "render-oracle"
BINARY = ROOT / "amiga" / "build" / "68020" / "code-police"
KEYS = ["position3d", "motionblur", "findtunnel", "findfloor", "findtwirl",
        "setzoomp", "flare3d"]


def hashes(text):
    found = {}
    for key in KEYS:
        match = re.search(rf"^{key} ([0-9a-f]+)$", text, re.MULTILINE)
        if match:
            found[key] = match.group(1)
    return found


def oracle():
    result = subprocess.run([str(ORACLE)], cwd=ROOT, capture_output=True, text=True, timeout=120)
    return hashes(result.stdout)


def port():
    result = subprocess.run(
        ["vamos", "-C", "20", "-s", "256", "-m", "8192", str(BINARY),
         "native-sdl2/assets", "render"],
        cwd=ROOT, capture_output=True, text=True, timeout=180)
    return hashes(result.stdout)


def main():
    want = oracle()
    got = port()
    failures = []
    for key in KEYS:
        ok = want.get(key) and want.get(key) == got.get(key)
        print(f"{'ok  ' if ok else 'FAIL'} {key}: oracle {want.get(key)}, port {got.get(key)}")
        if not ok:
            failures.append(key)
    if failures:
        print(f"\nFAILED: {', '.join(failures)}")
        return 1
    print("\nPASS: the C renderer matches the verified C++ engine on all 7 paths.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
