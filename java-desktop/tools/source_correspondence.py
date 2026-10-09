#!/usr/bin/env python3
"""Account for every original class and method, and expose desktop source changes."""
from pathlib import Path
import difflib
import json
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def methods(classpath, name):
    output = subprocess.check_output(["javap", "-classpath", str(classpath), "-p", "-s", name], text=True)
    result = []
    declaration = None
    for line in output.splitlines():
        if re.match(r"\s+.*\(.*\);$", line):
            declaration = line.strip().split("(")[0].split()[-1]
        elif declaration and "descriptor:" in line:
            result.append((declaration, line.split("descriptor:")[1].strip()))
            declaration = None
    return result


def normalize(signature):
    return signature[0], signature[1].replace("Ljava/applet/Applet;", "LDesktopSurface;").replace("Ljava/applet/AudioClip;", "LDemoAudio;")


def main():
    diagnostic = json.loads((ROOT / "evidence/diagnostic.json").read_text())
    active = set(diagnostic["reachable_application_classes"])
    old = ROOT / "original/mkd_codepolice"
    new = ROOT / "java-desktop/build/classes/java/main"
    rows = []
    missing = []
    for file in sorted(old.glob("*.class")):
        name = file.stem
        original_methods = methods(old, name)
        row = {"original_class": name, "original_methods": original_methods}
        if name in active:
            compiled_methods = set(methods(new, name))
            absent = [m for m in original_methods if not m[0].startswith("access$") and normalize(m) not in compiled_methods]
            missing += [(name, m) for m in absent]
            row.update(status="restored", counterpart=name,
                       missing_non_synthetic_methods=absent,
                       added_methods=sorted(compiled_methods - {normalize(m) for m in original_methods}),
                       compiler_accessors=[m for m in original_methods if m[0].startswith("access$")])
        else:
            row.update(status="archived_only", reason="Outside the applet entry-point static closure; both independent reconstructions retained. Not linked into the active desktop engine.")
        rows.append(row)
    evidence = ROOT / "evidence/java-reactivation"
    evidence.mkdir(parents=True, exist_ok=True)
    (evidence / "class-method-correspondence.json").write_text(json.dumps(rows, indent=2) + "\n")
    changes = ""
    for name in ["kraycasting.java", "kSound.java"]:
        baseline = ROOT / "reverse/cfr" / name
        restored = ROOT / "java-desktop/src/main/java" / name
        changes += "".join(difflib.unified_diff(baseline.read_text().splitlines(True), restored.read_text().splitlines(True),
                                             fromfile=str(baseline.relative_to(ROOT)), tofile=str(restored.relative_to(ROOT))))
    (evidence / "desktop-platform.patch").write_text(changes)
    if missing:
        raise SystemExit("Missing original methods: " + str(missing))
    print(f"PASS: {len(active)} active classes retain every non-synthetic original method; {len(rows) - len(active)} unused classes accounted for.")


if __name__ == "__main__":
    main()
