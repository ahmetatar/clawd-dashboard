#!/usr/bin/env python3
# Rig'li clawd karakterine TUTARLI animasyon uretir (/characters/animations).
# mode=template -> hazir iskelet sablonu (en tutarli). mode=v3 -> serbest aksiyon.
# Frame'ler out/anim_<ad>/frame_*.png olarak kaydedilir (03_png_to_header ile header'a).
#
#   source secrets.sh
#   python3 05_char_anim.py idle    template breathing-idle south
#   python3 05_char_anim.py hacking v3       "typing fast on a keyboard, focused" south 8
import os, sys, json, math, base64, urllib.request, lib
from PIL import Image

def save_frame(field, fp):
    """Character API 'image' alani HAM RGBA base64 dondurur (PNG degil). PNG'ye cevir."""
    raw = base64.b64decode(field["base64"].split(",", 1)[-1])
    w = field.get("width"); h = field.get("height")
    if not (w and h):                      # boyut yoksa kareden tahmin et
        side = int(round(math.sqrt(len(raw) // 4))); w = h = side
    if raw[:8] == b"\x89PNG\r\n\x1a\n":     # zaten PNG ise oldugu gibi yaz
        open(fp, "wb").write(raw); return
    Image.frombytes("RGBA", (w, h), raw).save(fp)

HERE = os.path.dirname(os.path.abspath(__file__))
cid = open(os.path.join(HERE, "out", "character_id.txt")).read().strip()

name   = sys.argv[1] if len(sys.argv) > 1 else "idle"
mode   = sys.argv[2] if len(sys.argv) > 2 else "template"
spec   = sys.argv[3] if len(sys.argv) > 3 else "breathing-idle"   # template_id ya da action
direction = sys.argv[4] if len(sys.argv) > 4 else "south"
frames_n  = int(sys.argv[5]) if len(sys.argv) > 5 else 8

body = {"character_id": cid, "animation_name": name, "directions": [direction]}
if mode == "template":
    body["mode"] = "template"; body["template_animation_id"] = spec
else:
    body["mode"] = "v3"; body["action_description"] = spec; body["frame_count"] = frames_n

print(f"animate '{name}' mode={mode} spec='{spec}' dir={direction}")
res = lib.post("/characters/animations", body)
jobs = res.get("background_job_ids") or []
dirs = res.get("directions") or [direction]
print("jobs:", jobs, "dirs:", dirs, "usage:", res.get("enhance_usage"))

def dl_auth(url, fp):
    r = urllib.request.Request(url, headers={"Authorization": "Bearer " + lib.KEY})
    with urllib.request.urlopen(r, timeout=90) as resp, open(fp, "wb") as f:
        f.write(resp.read())

outdir = os.path.join(HERE, "out", f"anim_{name}")
os.makedirs(outdir, exist_ok=True)
import glob
for old in glob.glob(os.path.join(outdir, "frame_*.png")):
    os.remove(old)

saved = 0
for job in jobs:
    done = lib.poll_job(job, every=4, timeout=420)
    # frame'ler nerede? once bilinen yerlere bak, yoksa yapiyi dok
    lr = done.get("last_response") or {}
    imgs = lr.get("images") or done.get("images") or []
    urls = lr.get("image_urls") or done.get("image_urls") or []
    if not imgs and not urls:
        print("  !! frame bulunamadi, job yapisi:")
        print("  keys:", list(done.keys()))
        print("  last_response keys:", list(lr.keys()) if isinstance(lr, dict) else lr)
        print(json.dumps(done, indent=2)[:1500])
        continue
    for i, im in enumerate(imgs):
        save_frame(im, os.path.join(outdir, f"frame_{saved:02d}.png")); saved += 1
    for i, u in enumerate(urls):
        dl_auth(u, os.path.join(outdir, f"frame_{saved:02d}.png")); saved += 1

print(f"{saved} frame -> {outdir}")
