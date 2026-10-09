#!/usr/bin/env python3
"""Verify the distributed ZIP, not the development tree (macOS only)."""
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
arch = platform.machine()
archive = ROOT / 'dist/native' / f'code-police-macos-{arch}.zip'
report = dict(platform=platform.platform(), architecture=arch,
              archive_sha256=hashlib.sha256(archive.read_bytes()).hexdigest())
with tempfile.TemporaryDirectory(prefix='Code Police natif été ') as temp:
    directory = Path(temp)
    subprocess.run(['ditto','-x','-k',str(archive),str(directory)],check=True)
    package = directory / f'macos-{arch}'
    app = package / 'Code Police.app'
    binary = app / 'Contents/MacOS/code-police'
    subprocess.run(['codesign','--verify','--deep','--strict',str(app)],check=True)
    manifest = json.loads((package / 'package-manifest.json').read_text())
    assert hashlib.sha256(binary.read_bytes()).hexdigest() == manifest['executable_sha256']
    for entry in manifest['assets']:
        assert hashlib.sha256((app / 'Contents/Resources/assets' / entry['path']).read_bytes()).hexdigest() == entry['sha256']
    deps = subprocess.check_output(['otool','-L',str(binary)],text=True)
    assert all(line.strip().startswith(('/usr/lib/','/System/Library/')) for line in deps.splitlines()[1:])
    env = {'PATH':'/usr/bin:/bin','HOME':os.environ['HOME'],'TMPDIR':os.environ.get('TMPDIR','/tmp'),'LANG':'en_US.UTF-8'}
    result = subprocess.run(['sandbox-exec','-p','(version 1)(allow default)(deny network*)',
        str(package / 'run-code-police.sh'),'--smoke-test','--offset','42','--capture',str(directory / 'offline.bmp')],
        cwd='/tmp',env=env,text=True,capture_output=True,timeout=25,check=True)
    assert 'requested duration' in result.stdout and 'audio active' in result.stdout
    assert (directory / 'offline.bmp').stat().st_size > 520*300*3
    # LaunchServices / Finder-style application launch, using the exact extracted bundle.
    capture = directory / 'launchservices.bmp'
    subprocess.run(['open','-n','-W',str(app),'--args','--duration','3','--offset','42','--capture',str(capture)],
        cwd='/tmp',env=env,timeout=25,check=True)
    assert capture.is_file() and capture.stat().st_size > 520*300*3
    report.update(resources_verified=len(manifest['assets']), system_dependencies_only=True,
                  signature_valid=True, executable_hash_valid=True, external_cwd='/tmp',
                  path_contains_spaces_and_accents=True, java_and_homebrew_removed_from_path=True,
                  network_denied_for_shell_launch=True, live_audio=True,
                  shell_smoke_log=result.stdout, shell_stderr=result.stderr,
                  launchservices_capture=True)
target = ROOT / 'evidence/native/package-check.json'
target.write_text(json.dumps(report,indent=2)+'\n')
print('PASS: extracted native ZIP, hashes, signature, system-only dependencies, offline shell/audio/window and LaunchServices launch.')
