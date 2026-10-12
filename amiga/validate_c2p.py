#!/usr/bin/env python3
"""Validate the C2P against an independent bit-by-bit Python reference.

The C port converts a deterministic 320x200 four-colour pattern to four AGA
bitplanes. This computes the same planes in Python, one pixel at a time, and
compares. Identical hashes mean the C2P is correct against a reference that
shares no code with it, on the host and (with --m68k) on the target.

This is the four-plane, 16-colour conversion documentation/amiga-aga-feasibility.md
specifies; it mirrors the contract of the Kalms adaptation the study adopts.
"""
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HOST_BIN = ROOT / "amiga" / "build" / "host" / "code-police"
M68K_BIN = ROOT / "amiga" / "build" / "68020" / "code-police"
W, H = 320, 200


def fnv(data):
    h = 0xCBF29CE484222325
    for byte in data:
        h = ((h ^ byte) * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return h


def reference():
    """Planes as four lists of bytes, one pixel at a time."""
    planes = [bytearray((W // 8) * H) for _ in range(4)]
    for y in range(H):
        for x in range(W):
            v = (x * 3 + y * 5 + (x >> 4)) & 0x0F
            bit = 0x80 >> (x & 7)
            for p in range(4):
                if v & (1 << p):
                    planes[p][(y * (W // 8)) + (x >> 3)] |= bit
    return [fnv(bytes(p)) for p in planes]


def port(use_m68k):
    if use_m68k:
        cmd = ["vamos", "-C", "20", "-s", "256", "-m", "8192", str(M68K_BIN),
               "native-sdl2/assets", "c2p"]
    else:
        cmd = [str(HOST_BIN), "native-sdl2/assets", "c2p"]
    result = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, timeout=600)
    return [int(m.group(2), 16) for m in re.finditer(r"^c2p plane (\d+) ([0-9a-f]+)$",
                                                     result.stdout, re.MULTILINE)]


def main():
    want = reference()
    got = port("--m68k" in sys.argv)
    if len(got) != 4:
        print(f"port returned {len(got)} planes")
        return 1
    failures = 0
    for p in range(4):
        ok = want[p] == got[p]
        print(f"{'ok  ' if ok else 'FAIL'} plane {p}: port {got[p]:016x}, reference {want[p]:016x}")
        if not ok:
            failures += 1
    if failures:
        print(f"\nFAILED: {failures} planes differ")
        return 1
    print("\nPASS: the C2P matches the independent reference on all four planes.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
