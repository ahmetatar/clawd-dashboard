#!/usr/bin/env python3
# Bir animasyon klasorundeki (out/anim_<ad>) PNG frame'leri -> tek RGB565 C header.
# Seffaf pikseller krem zemine kompoze edilir (cihaz zemini krem). Cihazda scale ile buyutulur.
#
#   python3 tools/pixellab/03_png_to_header.py out/anim_idle clawd_idle
import sys, os, glob
from PIL import Image

BG = (239, 239, 234)  # cihaz krem zemini
src = sys.argv[1]
name = sys.argv[2] if len(sys.argv) > 2 else os.path.basename(src).replace("anim_", "")

frames = sorted(glob.glob(os.path.join(src, "frame_*.png")))
if not frames:
    raise SystemExit("frame bulunamadi: " + src)

def to565(r, g, b): return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

imgs = []
for fp in frames:
    im = Image.open(fp).convert("RGBA")
    bg = Image.new("RGBA", im.size, BG + (255,))
    imgs.append(Image.alpha_composite(bg, im).convert("RGB"))
W, H = imgs[0].size
N = len(imgs)

out = f"out/{name}.h"
with open(out, "w") as f:
    f.write(f"// clawd animasyonu '{name}': {N} frame, {W}x{H}, RGB565 (PixelLab)\n#pragma once\n")
    f.write(f"#define {name.upper()}_W {W}\n#define {name.upper()}_H {H}\n#define {name.upper()}_FRAMES {N}\n")
    f.write(f"static const uint16_t {name}[{name.upper()}_FRAMES][{name.upper()}_W*{name.upper()}_H] = {{\n")
    for im in imgs:
        px = im.load()
        vals = [to565(*px[x, y]) for y in range(H) for x in range(W)]
        f.write("{" + ",".join(f"0x{v:04X}" for v in vals) + "},\n")
    f.write("};\n")
print(f"yazildi {out}  ({N} frame {W}x{H}, ~{N*W*H*2//1024} KB flash)")
