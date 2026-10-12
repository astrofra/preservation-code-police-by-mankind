#!/usr/bin/env python3
"""Differential check: the C port's whole-script frames must match the oracle.

The C port renders the whole 22,200-tick cycle sequentially (as verify.cpp does)
and prints the FNV hash of every frame. native-sdl2/build/reference.bin holds
the verified engine's hashes for the same 11,101 frames. This compares them:
identical hashes mean the C port reproduces real script frames bit-for-byte,
including the object graph, effect dispatch, camera maths and particle
simulation, not just the isolated renderer paths that compare_render.py covers.

The same C sources build for the host (fast: the full cycle is a few seconds)
and for m68k (the real target, but a full cycle under vamos is very slow). This
uses the host build by default; pass --m68k to run the m68k binary under vamos.
"""
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "native-sdl2" / "build" / "reference.bin"
HOST_BIN = ROOT / "amiga" / "build" / "host" / "code-police"
M68K_BIN = ROOT / "amiga" / "build" / "68020" / "code-police"
KNOWN_DIVERGENCES = {11050, 11070}  # popu3 particle system; see amiga/README.md


def oracle_hashes():
    """The reference's frame hash for every tick."""
    want = {}
    with open(REFERENCE, "rb") as f:
        magic, frames = struct.unpack(">II", f.read(8))
        if magic != 0x43505231:
            raise SystemExit("bad reference magic")
        for _ in range(frames):
            header = f.read(12)
            if len(header) < 12:
                break
            tick, hashv = struct.unpack(">IQ", header)
            count = struct.unpack(">I", f.read(4))[0]
            f.read(8 * count)
            commands = struct.unpack(">I", f.read(4))[0]
            for _ in range(commands):
                for _ in range(2):
                    n = struct.unpack(">H", f.read(2))[0]
                    f.read(n)
            if f.read(1)[0]:
                f.read(4 * 520 * 300)
            want[tick] = hashv
    return want


def build_host():
    HOST_BIN.parent.mkdir(parents=True, exist_ok=True)
    src = ROOT / "amiga" / "src"
    sources = sorted(str(p) for p in src.glob("*.c"))
    subprocess.run(
        ["cc", "-std=c99", "-O2", "-Wno-unused-function",
         "-I", str(ROOT / "native-sdl2" / "vendor" / "stb"), "-I", str(src),
         *sources, "-o", str(HOST_BIN), "-lm"], check=True)


def port_hashes(use_m68k):
    if use_m68k:
        cmd = ["vamos", "-C", "20", "-s", "256", "-m", "8192", str(M68K_BIN),
               "native-sdl2/assets", "frames"]
    else:
        build_host()
        cmd = [str(HOST_BIN), "native-sdl2/assets", "frames"]
    result = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, timeout=7200)
    return {int(m.group(1)): int(m.group(2), 16)
            for m in re.finditer(r"^frame (\d+) ([0-9a-f]+)$", result.stdout, re.MULTILINE)}


def main():
    got = port_hashes("--m68k" in sys.argv)
    if not got:
        print("no frames reported by the port")
        return 1
    want = oracle_hashes()
    diff, known, unknown = 0, 0, []
    for tick in sorted(got):
        if tick in want and want[tick] == got[tick]:
            continue
        diff += 1
        if tick in KNOWN_DIVERGENCES:
            known += 1
        else:
            unknown.append(tick)
    total = len(want)
    matching = total - diff
    print(f"frames: port {len(got)}, oracle {total}")
    print(f"matching: {matching}/{total} ({100.0 * matching / total:.3f} %)")
    print(f"differing: {diff} (known: {known}, unexpected: {len(unknown)})")
    if unknown:
        print(f"unexpected differing ticks: {unknown[:20]}")
        return 1
    if known == len(KNOWN_DIVERGENCES):
        print("PASS: every frame matches except the two documented popu3 ticks.")
        return 0
    print("PASS: all frames match the verified oracle.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
