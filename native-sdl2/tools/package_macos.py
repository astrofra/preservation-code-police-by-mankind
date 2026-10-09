#!/usr/bin/env python3
"""Build a native macOS bundle, statically linking a pinned upstream SDL2.

No Java, Homebrew SDL, absolute resource paths or downloads at playback time.
Only Python's standard library, CMake, a compiler, curl and macOS tools needed.
"""
import hashlib
import json
import os
from pathlib import Path
import platform
import plistlib
import shutil
import subprocess
import tarfile
import tempfile

PROJECT = Path(__file__).resolve().parents[1]
ROOT = PROJECT.parent
VERSION = '2.32.10'
SHA256 = '5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165'
URL = f'https://github.com/libsdl-org/SDL/releases/download/release-{VERSION}/SDL2-{VERSION}.tar.gz'

def run(*args):
    subprocess.run([str(arg) for arg in args], check=True)

def main():
    if platform.system() != 'Darwin':
        raise SystemExit('This package builder targets macOS. Native CMake builds are portable; other packages are not yet validated.')
    arch = platform.machine()
    build = PROJECT / 'build'
    build.mkdir(exist_ok=True)
    archive = build / f'SDL2-{VERSION}.tar.gz'
    if not archive.exists():
        partial = archive.with_suffix('.download')
        run('curl', '-fL', '--retry', '2', URL, '-o', partial)
        partial.rename(archive)
    if hashlib.sha256(archive.read_bytes()).hexdigest() != SHA256:
        raise SystemExit('SDL source archive SHA-256 mismatch')
    source = build / f'SDL2-{VERSION}'
    if not source.exists():
        with tarfile.open(archive) as tar:
            # The pinned upstream archive contains only this directory tree.
            for member in tar.getmembers():
                target = (build / member.name).resolve()
                if not target.is_relative_to(source.resolve()) or member.issym() or member.islnk():
                    raise SystemExit('Unexpected SDL archive member: ' + member.name)
            tar.extractall(build)
    sdlbuild, prefix = build / 'sdl2-static', build / 'sdl2-install'
    run('cmake', '-S', source, '-B', sdlbuild, '-DCMAKE_BUILD_TYPE=Release',
        '-DSDL_SHARED=OFF', '-DSDL_STATIC=ON', '-DSDL_TEST=OFF', '-DSDL_TESTS=OFF',
        '-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0', f'-DCMAKE_OSX_ARCHITECTURES={arch}',
        f'-DCMAKE_INSTALL_PREFIX={prefix}')
    run('cmake', '--build', sdlbuild, '--parallel')
    run('cmake', '--install', sdlbuild)
    release = PROJECT / 'build-release'
    run('cmake', '-S', PROJECT, '-B', release, '-DCMAKE_BUILD_TYPE=Release',
        f'-DSDL2_DIR={prefix / "lib/cmake/SDL2"}', '-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0',
        f'-DCMAKE_OSX_ARCHITECTURES={arch}')
    run('cmake', '--build', release, '--parallel')
    dependencies = subprocess.check_output(['otool', '-L', str(release / 'code-police')], text=True)
    for line in dependencies.splitlines()[1:]:
        dep = line.strip().split(' (', 1)[0]
        if not dep.startswith(('/System/Library/', '/usr/lib/')):
            raise SystemExit('Non-system runtime dependency: ' + dep)
    destination = ROOT / 'dist/native' / f'macos-{arch}'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='native-package-', dir=build) as temp:
        stage = Path(temp) / f'macos-{arch}'
        resources = stage / 'Code Police.app/Contents/Resources'
        executable = stage / 'Code Police.app/Contents/MacOS/code-police'
        resources.mkdir(parents=True)
        executable.parent.mkdir(parents=True)
        shutil.copy2(release / 'code-police', executable)
        run('strip', '-x', executable)
        shutil.copytree(PROJECT / 'assets', resources / 'assets', ignore=shutil.ignore_patterns('.DS_Store'))
        licenses = resources / 'licenses'
        licenses.mkdir()
        shutil.copy2(source / 'LICENSE.txt', licenses / 'SDL2.txt')
        shutil.copy2(PROJECT / 'vendor/stb/LICENSE', licenses / 'stb.txt')
        shutil.copy2(ROOT / 'original/mkd_codepolice/MKD_CodePOlice.txt', resources / 'original-credits.txt')
        info = dict(CFBundleExecutable='code-police', CFBundleIdentifier='com.mankind.codepolice.native',
                    CFBundleName='Code Police', CFBundleDisplayName='Code Police',
                    CFBundlePackageType='APPL', CFBundleVersion='1', CFBundleShortVersionString='1.0',
                    LSMinimumSystemVersion='11.0', NSHighResolutionCapable=True,
                    NSHumanReadableCopyright='Code Police — Mankind (2001); native preservation port')
        with (resources.parent / 'Info.plist').open('wb') as file:
            plistlib.dump(info, file)
        launcher = stage / 'run-code-police.sh'
        launcher.write_text('''#!/bin/sh
set -eu
base=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
exec "$base/Code Police.app/Contents/MacOS/code-police" "$@"
''')
        launcher.chmod(0o755)
        (stage / 'README.txt').write_text(f'''Code Police / Mankind — native C++11/SDL2 restoration

Open Code Police.app, or run ./run-code-police.sh
macOS {arch}; deployment target macOS 11+. Tested on the build host only.
No Java, Homebrew or network needed. Keep the complete application bundle.
The 520 x 300 artwork is displayed at 2x by default, without the cinema surround.
Escape or close: quit. F11: fullscreen. --scale 1: original pixel size.
Optional --mute. Use --help for diagnostics.

Original demo/code/art/audio credits are in the application Resources directory.
Script and media remain separate under Contents/Resources/assets.
SDL2 {VERSION} is statically linked; SDL2 and stb licenses are included.
This local build is ad-hoc signed, not Apple-notarized.
''')
        run('codesign', '--force', '--sign', '-', stage / 'Code Police.app')
        run('codesign', '--verify', '--deep', '--strict', stage / 'Code Police.app')
        manifest = []
        for path in sorted((resources / 'assets').rglob('*')):
            if path.is_file():
                relative = path.relative_to(resources / 'assets')
                if path.read_bytes() != (PROJECT / 'assets' / relative).read_bytes():
                    raise SystemExit('Packaged resource mismatch: ' + str(relative))
                manifest.append(dict(path=relative.as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
        (stage / 'package-manifest.json').write_text(json.dumps(dict(
            architecture=arch, sdl_version=VERSION, sdl_source_sha256=SHA256,
            executable_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(), assets=manifest), indent=2)+'\n')
        if destination.exists():
            if not (destination / 'package-manifest.json').is_file():
                raise SystemExit('Refusing to replace an unrelated directory: ' + str(destination))
            shutil.rmtree(destination)
        shutil.copytree(stage, destination, symlinks=True)
    zipfile = ROOT / 'dist/native' / f'code-police-macos-{arch}.zip'
    if zipfile.exists(): zipfile.unlink()
    run('ditto', '-c', '-k', '--sequesterRsrc', '--keepParent', destination, zipfile)
    (build / 'native-dependencies.txt').write_text(dependencies)
    print('\nPackaged:', destination / 'Code Police.app')
    print('Archive:', zipfile)

if __name__ == '__main__':
    main()
