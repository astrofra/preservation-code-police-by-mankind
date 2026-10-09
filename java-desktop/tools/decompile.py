#!/usr/bin/env python3
"""Reproduce the independent reconstructions without modifying original files."""
from pathlib import Path
import argparse
import os
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[2]


def command(tool, variable):
    jar = os.environ.get(variable)
    return ["java", "-jar", jar] if jar else [tool]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, required=True,
                        help="A new directory; existing archived decompilations are never overwritten")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    archive = args.output / "original-classes.jar"
    with zipfile.ZipFile(archive, "w") as jar:
        for path in sorted((ROOT / "original/mkd_codepolice").glob("*.class")):
            jar.writestr(path.name, path.read_bytes())
    cfr = command("cfr-decompiler", "CFR_JAR")
    procyon = command("procyon-decompiler", "PROCYON_JAR")
    for name, cli in [("cfr", cfr + [str(archive), "--outputdir", str(args.output / "cfr")]),
                      ("procyon", procyon + ["-jar", str(archive), "-o", str(args.output / "procyon"), "--disable-foreach"])]:
        with (args.output / (name + ".log")).open("w") as log:
            subprocess.run(cli, check=True, stdout=log, stderr=subprocess.STDOUT)
    for name in ["k2Dpos", "kParameterXIMAGE", "kParameterXRC", "kSound", "rotozoom"]:
        subprocess.run(cfr + [str(ROOT / "original/mkd_codepolice" / ("kraycasting$" + name + ".class")),
                             "--innerclasses", "false", "--outputdir", str(args.output / "cfr-unreferenced")], check=True)


if __name__ == "__main__":
    main()
