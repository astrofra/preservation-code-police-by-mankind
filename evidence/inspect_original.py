#!/usr/bin/env python3
"""Reproduce the static diagnosis; never execute or modify the original classes.

Requires Pillow, ffprobe, ffmpeg and the separately generated evidence/classes.json.
"""
import collections
import hashlib
from html.parser import HTMLParser
import json
from pathlib import Path
import re
import struct
import subprocess
import zipfile

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ORIGINAL = ROOT / "original"
RELEASE = ORIGINAL / "mkd_codepolice"


class AppletPage(HTMLParser):
    def __init__(self):
        super().__init__()
        self.applet = None
        self.params = {}

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag == "applet":
            self.applet = attrs
        if tag == "param":
            self.params[attrs["name"]] = attrs["value"]


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def split_tags(text, tag):
    # Match kraycasting.SplitSTag's exact, case-sensitive delimiters.
    opening, closing = f"<{tag}>", f"</{tag}>"
    start = 0
    result = []
    while (pos := text.find(opening, start)) != -1:
        start = pos + len(opening)
        end = text.find(closing, start)
        if end == -1:
            raise ValueError(f"Unclosed {tag}")
        result.append(text[start:end])
    return result


def main():
    manifest = []
    for path in sorted(ORIGINAL.rglob("*")):
        if path.is_file() and path.name != ".DS_Store":
            data = path.read_bytes()
            manifest.append({"path": path.relative_to(ROOT).as_posix(),
                             "bytes": len(data), "sha256": sha256(data),
                             "provenance": "User-supplied local file; acquisition date unknown"})
    (ROOT / "evidence/original-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")

    with zipfile.ZipFile(ORIGINAL / "mkd_codepolice.zip") as archive:
        entries = [e for e in archive.infolist() if not e.is_dir()]
        mismatches = [e.filename for e in entries
                      if not (RELEASE / e.filename).is_file()
                      or archive.read(e) != (RELEASE / e.filename).read_bytes()]
        zip_status = {"crc_failure": archive.testzip(), "files": len(entries),
                      "missing_or_different_extracted_files": mismatches}

    page = AppletPage()
    page.feed((RELEASE / "page.html").read_text(encoding="latin1"))
    script = page.params["SCRIPT"]
    refs = [value.split("|", 1)[1].strip() for tag in ("KIMAGES", "KSOUNDS")
            for value in split_tags(script, tag)]
    # Exact path case is checked against the inventory, even on macOS.
    release_names = {p.relative_to(RELEASE).as_posix() for p in RELEASE.rglob("*") if p.is_file()}
    images = []
    for path in sorted((RELEASE / "Images").iterdir()):
        if not path.is_file() or path.name == ".DS_Store":
            continue
        with Image.open(path) as im:
            entry = {"path": path.relative_to(ROOT).as_posix(), "format": im.format,
                     "size": list(im.size), "mode": im.mode, "frames": getattr(im, "n_frames", 1),
                     "transparency": "transparency" in im.info}
            for frame in range(entry["frames"]):
                im.seek(frame)
                im.load()
            images.append(entry)
    sounds = []
    for path in sorted((RELEASE / "Sounds").iterdir()):
        if not path.is_file() or path.name == ".DS_Store":
            continue
        probe = subprocess.run(["ffprobe", "-v", "error", "-show_entries",
                                "stream=codec_name,sample_rate,channels:format=duration",
                                "-of", "json", str(path)], check=True, capture_output=True, text=True)
        subprocess.run(["ffmpeg", "-nostdin", "-v", "error", "-i", str(path),
                        "-f", "null", "-"], check=True, capture_output=True, text=True)
        data = path.read_bytes()
        header = struct.unpack(">6I", data[:24])
        sounds.append({"path": path.relative_to(ROOT).as_posix(),
                       "au_magic_valid": data[:4] == b".snd",
                       "au_encoding": header[3],
                       "full_decode_passed": True,
                       "au_declared_payload_matches": header[2] == len(data) - header[1],
                       **json.loads(probe.stdout)})

    classes = json.loads((ROOT / "evidence/classes.json").read_text())
    lookup = {item["name"]: item for item in classes["classes"]}
    seen, missing, todo = set(), set(), ["kraycasting"]
    while todo:
        name = todo.pop()
        if name in seen:
            continue
        seen.add(name)
        if name not in lookup:
            missing.add(name)
        else:
            todo.extend(lookup[name]["application_references"])

    parts, total = [], 0
    effect_counts = collections.Counter()
    for part in split_tags(split_tags(script, "KSCRIPT")[0], "KPART"):
        duration, offset = map(int, split_tags(part, "D")[0].split(","))
        effects = [split_tags(fx, "Pa")[0] for fx in split_tags(part, "Fx")]
        effect_counts.update(effects)
        parts.append({"index": len(parts), "start_seconds": total / 100,
                      "duration_seconds": duration / 100, "local_offset_seconds": offset / 100,
                      "effects": dict(collections.Counter(effects))})
        total += duration
    conversion_sites = []
    for path in sorted((ROOT / "evidence/bytecode").glob("*.txt")):
        content = path.read_text()
        for match in re.finditer(r"\d+: [fd]2l\s+\d+: l2i", content):
            conversion_sites.append({"file": path.name, "instructions": match.group()})
    report = {"zip": zip_status, "applet": page.applet,
              "script_image_declarations": len(split_tags(script, "KIMAGES")),
              "script_sound_declarations": len(split_tags(script, "KSOUNDS")),
              "missing_script_files_exact_case": sorted(set(refs) - release_names),
              "images": images, "sounds": sounds,
              "reachable_application_classes": sorted(seen),
              "missing_reachable_application_classes": sorted(missing),
              "classes_outside_main_static_closure": sorted(set(lookup) - seen),
              "parts": parts, "nominal_cycle_seconds_from_bytecode_time_unit": total / 100,
              "end_mode": split_tags(script, "KEND"), "active_effects": dict(effect_counts),
              "float_or_double_to_long_to_int_sites_in_inspected_bytecode": conversion_sites,
              "limitations": ["Static inspection only; no original class execution or playback.",
                              "Dependency closure does not prove runtime correctness.",
                              "Media decoding does not validate Java playback, mixing or synchronization.",
                              "The supplied Pouet snapshot is a third-party historical reference."]}
    (ROOT / "evidence/diagnostic.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({"zip": zip_status, "images_decoded": len(images), "sounds_probed": len(sounds),
                      "missing_script_files": report["missing_script_files_exact_case"],
                      "missing_reachable_classes": sorted(missing), "parts": len(parts),
                      "cycle_seconds": total / 100, "active_effects": dict(effect_counts)}, indent=2))


if __name__ == "__main__":
    main()
