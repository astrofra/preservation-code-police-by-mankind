#!/usr/bin/env python3
"""Generate the launch page from the archived cinema table and package the site."""
import csv
import hashlib
import html
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
ORIGINAL = ROOT / 'original/mkd_codepolice'
OUTPUT = ROOT / 'dist/wasm'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def page():
    source = (ORIGINAL / 'page.html').read_text(encoding='latin-1')
    source, count = re.subn(r'<APPLET\b.*?</APPLET>',
        '<canvas id="screen" width="520" height="300" tabindex="0" '
        'aria-label="Code Police par Mankind">Activez JavaScript pour voir la démo.</canvas>',
        source, flags=re.I | re.S)
    assert count == 1, 'Expected the original applet'
    cinema = re.search(r'<TABLE\b[^>]*WIDTH="640".*?</TABLE>', source, re.I | re.S).group()
    cinema = cinema.replace('<TABLE ', '<TABLE id="cinema" aria-hidden="true" ', 1)
    cinema = cinema.replace('VALIGN="MIDDLE">', 'VALIGN="MIDDLE"><colgroup><col style="width:65px"><col style="width:520px"><col style="width:55px"></colgroup>', 1)
    for name, side in [('haut', 'top'), ('gauche', 'left'), ('droit', 'right'), ('bas', 'bottom')]:
        cinema, count = re.subn(r'(<TD)([^>]*>\s*<IMG SRC="Images/html_bord_' + name + r'\.jpg")',
                                r'\1 class="' + side + r'"\2', cinema)
        assert count == 1, side
    cinema = re.sub(r'(<IMG\b)', r'\1 alt=""', cinema, flags=re.I)
    txt = (ORIGINAL / 'MKD_CodePOlice.txt').read_text(encoding='latin-1')
    credits = []
    for key, label in [('Code', 'Code'), ('Gfx', 'Graphismes'), ('Sons', 'Sons')]:
        value = re.search(r'^\s*' + key + r'\s+([^\n]+)', txt, re.M).group(1).strip()
        value = ', '.join(part.strip() for part in value.split(','))
        credits.append(f'<dt>{label}</dt><dd>{html.escape(value)}</dd>')
    template = (ROOT / 'wasm/web/index.template.html').read_text()
    return template.replace('<!-- CINEMA -->', cinema).replace('<!-- CREDITS -->', '\n        '.join(credits))


def main():
    originals = json.loads((ROOT / 'evidence/original-manifest.json').read_text())
    for item in originals:
        assert sha(ROOT / item['path']) == item['sha256'], item['path']
    for item in csv.DictReader((ROOT / 'native-sdl2/assets/image-manifest.tsv').open(), delimiter='\t'):
        assert sha(ROOT / 'native-sdl2/assets' / item['converted']) == item['converted_sha256']
    for sound in (ROOT / 'native-sdl2/assets/Sounds').iterdir():
        assert sound.read_bytes() == (ORIGINAL / 'Sounds' / sound.name).read_bytes(), sound.name
    script = re.search(br'<PARAM NAME="SCRIPT" VALUE="(.*?)">', (ORIGINAL / 'page.html').read_bytes(), re.S).group(1)
    assert (ROOT / 'native-sdl2/assets/scene.txt').read_bytes() == script, 'Original script bytes'
    compiler = Path(shutil.which('emcc')).resolve()
    sdk = compiler.parent
    sdl_port = (sdk / 'tools/ports/sdl2.py').read_text()
    assert "VERSION = '2.32.8'" in sdl_port, 'Unexpected SDL2 port'
    toolchain = subprocess.check_output([str(compiler), '--version'], text=True).splitlines()[0]
    assert ' 4.0.23 ' in toolchain, toolchain
    OUTPUT.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='package-', dir=OUTPUT) as scratch:
        site = Path(scratch) / 'code-police'
        site.mkdir()
        (site / 'index.html').write_text(page())
        for name in ['bootstrap.js', 'style.css']:
            shutil.copy2(ROOT / 'wasm/web' / name, site / name)
        for name in ['code-police.js', 'code-police.wasm']:
            shutil.copy2(ROOT / 'wasm/build/cmake' / name, site / name)
        shutil.copytree(ROOT / 'native-sdl2/assets', site / 'assets')
        shutil.copytree(ORIGINAL, site / 'original')
        shutil.copy2(ORIGINAL / 'MKD_CodePOlice.txt', site / 'MKD_CodePOlice.txt')
        (site / 'Images').mkdir()
        for p in ORIGINAL.glob('Images/html_*'):
            shutil.copy2(p, site / 'Images' / p.name)
        resources = [dict(path=p.relative_to(site).as_posix(), bytes=p.stat().st_size, sha256=sha(p))
                     for p in sorted((site / 'assets').rglob('*')) if p.is_file()]
        (site / 'resources.json').write_text(json.dumps(resources, indent=2) + '\n')
        licenses = site / 'licenses'
        licenses.mkdir()
        for dest, source in {
            'EMSCRIPTEN.txt': sdk / 'LICENSE',
            'LIBCXX.txt': sdk / 'system/lib/libcxx/LICENSE.TXT',
            'LIBCXXABI.txt': sdk / 'system/lib/libcxxabi/LICENSE.TXT',
            'MUSL.txt': sdk / 'system/lib/libc/musl/COPYRIGHT',
            'COMPILER-RT.txt': sdk / 'system/lib/compiler-rt/LICENSE.TXT',
            'SDL2.txt': sdk / 'cache/ports/sdl2/SDL-release-2.32.8/LICENSE.txt',
            'STB.txt': ROOT / 'native-sdl2/vendor/stb/LICENSE',
        }.items():
            shutil.copy2(source, licenses / dest)
        shutil.copy2(ROOT / 'wasm/UPLOAD.txt', site / 'UPLOAD.txt')
        # Original code/media have their original authorship, not a newly inferred
        # open-source license. Library notices apply only to their dependencies.
        provenance = dict(originals=originals, resources=resources,
            credits='MKD_CodePOlice.txt; complete unchanged release under original/',
            original_page_title='Empty in all three HTML files; accessible title added',
            adaptations=[
                'Cinema table generated from page.html, applet replaced with a canvas; 640x480 layout and original JPG/GIF borders',
                'New landing page, settings and Start gesture; credits parsed from original TXT',
                'Popup/frameset replaced by an in-page player; broken close link returns to home',
                'Cinema optional; requested size fits inside bounds; fullscreen retains monitor mode',
                'Fixed 520x300 SDL software canvas, visible device-pixel canvas with smoothing disabled',
                'External Java-decoded PNG derivatives and unchanged AU/script bytes, as in native distribution',
                'Render then schedule 20 ms timeout, elapsed original 10 ms ticks; no catch-up simulation',
                'Hidden tabs pause both clock and audio; return resumes without skipped scenes',
                'Browser user gesture required for audio; source JS readable, C++ sources included'])
        (site / 'provenance.json').write_text(json.dumps(provenance, indent=2) + '\n')
        patterns = [('native-sdl2/src', '*'), ('native-sdl2/vendor/stb', '*'),
                    ('wasm/src', '*'), ('wasm/web', '*'), ('wasm', '*.py'),
                    ('wasm', '*.sh'), ('wasm', 'CMakeLists.txt'), ('wasm', 'UPLOAD.txt'),
                    ('', '*-wasm.sh')]
        sources = {p.relative_to(ROOT).as_posix(): sha(p) for folder, pattern in patterns
                   for p in sorted((ROOT / folder).glob(pattern)) if p.is_file()}
        for name in sources:
            dest = site / 'sources' / name
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / name, dest)
        build = dict(emscripten=toolchain, sdl='2.32.8', stb='2.30', language='C++11',
            renderer='SDL2 software + Canvas2D; no WebGL context, threads, Asyncify or isolation headers',
            logical_size=[520, 300],
            source_base_commit=subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip(),
            source_identity_note='Base commit plus exact source hashes; generated artifacts are not committed',
            sources_sha256=sources)
        (site / 'build-info.json').write_text(json.dumps(build, indent=2) + '\n')
        (site / 'SHA256SUMS').write_text(''.join(f'{sha(p)}  {p.relative_to(site).as_posix()}\n'
            for p in sorted(site.rglob('*')) if p.is_file()))
        archive = Path(scratch) / 'code-police-web.zip'
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as out:
            for p in sorted(site.rglob('*')):
                if p.is_file():
                    info = zipfile.ZipInfo(p.relative_to(site).as_posix(), (2026, 10, 9, 0, 0, 0))
                    info.compress_type = zipfile.ZIP_DEFLATED
                    info.external_attr = 0o100644 << 16
                    out.writestr(info, p.read_bytes())
        destination = OUTPUT / 'code-police'
        if destination.exists():
            shutil.rmtree(destination)
        shutil.move(str(site), destination)
        archive.replace(OUTPUT / archive.name)
    (OUTPUT / 'code-police-web.zip.sha256').write_text(f'{sha(OUTPUT / "code-police-web.zip")}  code-police-web.zip\n')
    print(f'Upload directory: {destination}\nZIP: {OUTPUT / "code-police-web.zip"}')


if __name__ == '__main__':
    main()
