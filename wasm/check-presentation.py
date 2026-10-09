#!/usr/bin/env python3
"""Check actual browser screenshots, including CSS compositing after Canvas2D."""
import json
from pathlib import Path
import sys
from PIL import Image, ImageChops, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
name = '-'.join(sys.argv[1:3])
g = json.loads((ROOT / f'wasm/build/presentation/{name}.json').read_text())
captures = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / 'reference/wasm'
shot = Image.open(captures / f'{name}.png').convert('RGB')
canvas = Image.open(ROOT / f'wasm/build/presentation/{name}.png').convert('RGB')
dpr = g['dpr']
x, y, w, h = [round(g[n] * dpr) for n in ('x', 'y', 'width', 'height')]
actual = shot.crop((x, y, x + w, y + h))
assert actual.size == canvas.size
# The original 15px close icon can overlap the top-left edge on an exact-fit
# viewport. Mask that documented HTML control only, not the artwork elsewhere.
for im in (actual, canvas):
    right, bottom = round(15*dpr)-x-1, round(15*dpr)-y-1
    if right >= 0 and bottom >= 0:
        ImageDraw.Draw(im).rectangle((0,0,right,bottom), fill=(0,0,0))
assert ImageChops.difference(actual, canvas).getbbox() is None, 'CSS compositor changed the canvas pixels'
source = Image.open(ROOT / 'reference/native/original-4500.png').convert('RGB')
palette = set(source.get_flattened_data())
extra = sum(count for count, colour in actual.getcolors(w*h) if colour not in palette and colour != (0,0,0))
assert extra == 0, 'Screenshot contains interpolated colours'
if sys.argv[2] == 'cinema':
    assert ImageChops.difference(actual, source.resize(actual.size, Image.Resampling.NEAREST)).getbbox() is None
else:
    bands = [(30,0,shot.width,y), (0,y+h,shot.width,shot.height), (0,30,x,y+h), (x+w,0,shot.width,y+h)]
    for box in bands:
        if box[2] > box[0] and box[3] > box[1]:
            assert shot.crop(box).getextrema() == ((0,0),(0,0),(0,0)), f'Non-black fullscreen band {box}'
print(json.dumps(dict(screenshot=name+'.png', exact_visible_canvas=True,
                      interpolated_pixels=extra, original_2x_exact=sys.argv[2]=='cinema',
                      fullscreen_bands_black=sys.argv[2]=='fullscreen')))
