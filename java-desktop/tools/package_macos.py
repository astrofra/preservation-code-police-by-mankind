#!/usr/bin/env python3
"""Build a self-contained macOS application, keeping assets outside the JAR."""
from pathlib import Path
import hashlib
import json
import platform
import shutil
import subprocess
import tempfile

from PIL import Image

PROJECT = Path(__file__).resolve().parents[1]
ROOT = PROJECT.parent


def main():
    if platform.system() != "Darwin":
        raise SystemExit("This packaging script targets macOS; run jpackage on each other target OS.")
    subprocess.run([str(PROJECT / "gradlew"), "-p", str(PROJECT), "--console=plain", "installDist"], check=True)
    jpackage = shutil.which("jpackage")
    if not jpackage:
        raise SystemExit("jpackage from the build JDK must be on PATH")
    build = PROJECT / "build"
    stage = Path(tempfile.mkdtemp(prefix="package-", dir=build))
    inputs = stage / "input"
    inputs.mkdir()
    jar = PROJECT / "build/libs/code-police-1.0.0.jar"
    shutil.copy2(jar, inputs / jar.name)
    shutil.copytree(PROJECT / "assets", inputs / "assets")
    iconset = stage / "CodePolice.iconset"
    iconset.mkdir()
    # Reproducible packaging conversion of an original badge; no image synthesis.
    with Image.open(ROOT / "original/mkd_codepolice/Images/badge2.gif") as source:
        source = source.convert("RGBA")
        for size in [16, 32, 128, 256, 512]:
            for factor, suffix in [(1, ""), (2, "@2x")]:
                edge = size * factor
                badge = source.copy()
                badge.thumbnail((edge, edge), Image.Resampling.LANCZOS)
                image = Image.new("RGBA", (edge, edge))
                image.alpha_composite(badge, ((edge - badge.width) // 2, (edge - badge.height) // 2))
                image.save(iconset / f"icon_{size}x{size}{suffix}.png")
    icon = stage / "CodePolice.icns"
    subprocess.run(["iconutil", "-c", "icns", str(iconset), "-o", str(icon)], check=True)
    subprocess.run([jpackage, "--type", "app-image", "--name", "Code Police", "--app-version", "1.0.0",
                    "--vendor", "Mankind", "--description", "Code Police (2001) — preserved Java demo",
                    "--input", str(inputs), "--main-jar", jar.name, "--main-class", "CodePoliceDesktop",
                    "--icon", str(icon), "--add-modules", "java.base,java.desktop",
                    "--dest", str(stage / "app")], check=True)
    target = ROOT / "dist/java" / ("macos-" + platform.machine()) / "Code Police.app"
    target.parent.mkdir(parents=True, exist_ok=True)
    if target.exists():
        if not (target / "Contents/app" / jar.name).is_file():
            raise SystemExit("Refusing to replace an unrelated application: " + str(target))
        shutil.rmtree(target)
    shutil.copytree(stage / "app/Code Police.app", target, symlinks=True)
    files = []
    for file in sorted((target / "Contents/app/assets").rglob("*")):
        if file.is_file():
            relative = file.relative_to(target / "Contents/app/assets")
            source = ROOT / "original/mkd_codepolice" / relative
            assert source.read_bytes() == file.read_bytes(), str(relative)
            files.append({"path": relative.as_posix(), "sha256": hashlib.sha256(file.read_bytes()).hexdigest()})
    (build / "packaged-assets.json").write_text(json.dumps(files, indent=2) + "\n")
    print("Packaged:", target)
    print("Original media/script copies verified:", len(files))


if __name__ == "__main__":
    main()
