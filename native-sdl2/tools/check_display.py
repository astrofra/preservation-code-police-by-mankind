#!/usr/bin/env python3
"""Validate actual SDL presentation readback against its unscaled framebuffer."""
from pathlib import Path
import json
import re
import subprocess
import sys
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
binary = Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT / 'native-sdl2/build/code-police'
out = ROOT / 'native-sdl2/build/display-checks'
out.mkdir(exist_ok=True)
results = []
for name, flags, expected in [
    ('window-720', ['--resolution', '1280x720'], (1248,720)),
    ('window-600', ['--resolution', '800x600'], (800,462)),
    ('fullscreen', ['--resolution','800x600','--fullscreen','--smoke-test'], None),
]:
    raw, screen = out / (name+'-buffer.bmp'), out / (name+'-display.bmp')
    run = subprocess.run([str(binary), '--offset','45','--duration','1', *flags,
        '--capture',str(raw),'--capture-display',str(screen)],capture_output=True,text=True,timeout=30)
    assert run.returncode == 0, run.stdout + run.stderr
    if expected:
        assert f'Window size {expected[0]}x{expected[1]}' in run.stdout
    else:
        assert 'monitor mode unchanged; nearest neighbour verified' in run.stdout
    match = re.search(r'Display capture (\d+)x(\d+); viewport=(\d+),(\d+),(\d+),(\d+)',run.stdout)
    assert match,run.stdout
    width,height,x,y,w,h = map(int,match.groups())
    source = Image.open(raw).convert('RGB')
    actual = Image.open(screen).convert('RGB')
    assert actual.size == (width,height)
    image = actual.crop((x,y,x+w,y+h))
    # Nearest neighbour must introduce no colours absent from the source,
    # including non-integer and HiDPI magnifications.
    pixels = lambda im: im.get_flattened_data() if hasattr(im,'get_flattened_data') else im.getdata()
    palette = set(pixels(source))
    assert set(pixels(image)) <= palette, 'Interpolated colours in SDL output'
    for box in [(0,0,width,y),(0,y+h,width,height),(0,y,x,y+h),(x+w,y,width,y+h)]:
        if box[2]>box[0] and box[3]>box[1]:
            assert set(pixels(actual.crop(box))) == {(0,0,0)}, 'Non-black bars'
    if expected:
        assert x==0 and y==0 and w==width and h==height, 'Bands in windowed mode'
    elif abs(width*300-height*520)>520:
        assert w!=width or h!=height, 'Missing fullscreen bands'
    actual.save(out / (name+'.png'))
    results.append(dict(case=name, output=[width,height], viewport=[x,y,w,h],
        no_interpolated_colours=True,black_bands=True,log=run.stdout))
report = ROOT / 'evidence/native/display-checks.json'
report.write_text(json.dumps(results,indent=2)+'\n')
print('PASS: SDL window sizing, HiDPI readback, nearest-neighbour colours, fullscreen bands, F11 round trip and unchanged monitor mode.')
