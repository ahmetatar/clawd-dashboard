#!/usr/bin/env python3
# clawd_base.png'i referans alip bir animasyon uret (animate-with-text-v2).
# Job arka planda calisir; poll edip frame'leri out/anim_<ad>/frame_*.png olarak kaydeder.
#
#   source tools/pixellab/secrets.sh && python3 tools/pixellab/02_animate.py <ad> "<aksiyon>" [SIZE]
# ornek: python3 tools/pixellab/02_animate.py idle "idle breathing, gentle bob" 64
import sys, os, lib
from PIL import Image

name   = sys.argv[1] if len(sys.argv) > 1 else "idle"
action = sys.argv[2] if len(sys.argv) > 2 else "idle breathing, gentle bob"
size   = int(sys.argv[3]) if len(sys.argv) > 3 else 64

base = "out/clawd_base.png"
if not os.path.exists(base):
    raise SystemExit("once 01_clawd_base.py calistir (out/clawd_base.png yok)")
w, h = Image.open(base).size

print(f"animate '{name}': {action}  ({w}x{h})")
res = lib.post("/animate-with-text-v2", {
    "reference_image": lib.b64img(base),                 # {type:base64, base64:raw}
    "reference_image_size": {"width": w, "height": h},
    "action": action,
    "image_size": {"width": size, "height": size},
    "view": "side",
})

job = res.get("background_job_id")
done = lib.poll_job(job) if job else res
# frame'ler last_response.images altinda
imgs = (done.get("last_response") or {}).get("images") or done.get("images") or []

outdir = f"out/anim_{name}"
os.makedirs(outdir, exist_ok=True)
for i, im in enumerate(imgs):
    lib.save_image_field(im, f"{outdir}/frame_{i:02d}.png")
print(f"{len(imgs)} frame -> {outdir}")
u = (done.get("usage") or {})
print("kullanim:", u.get("generations"), "generation")
