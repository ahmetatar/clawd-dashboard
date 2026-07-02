#!/usr/bin/env python3
# clawd'i PixelLab CHARACTER sistemine rig'li karakter olarak yaratir (create-character-v3).
# Bu ASIL yol: rig'li/iskeletli karakter -> sonra /characters/animations ile TUTARLI animasyon.
# character_id'yi out/character_id.txt'e kaydeder (tekrar yaratma = bosa kredi).
#
#   source secrets.sh && python3 04_character.py
import os, sys, json, lib

HERE = os.path.dirname(os.path.abspath(__file__))
BASE = os.path.join(HERE, "out", "clawd_base.png")
IDFILE = os.path.join(HERE, "out", "character_id.txt")

# SEKLI KORUMAK ICIN: create-character-pro + method=rotate_character.
# Bu, referansi YENIDEN CIZMEZ; gercek sprite'i 8 yone dondurur -> clawd'in
# bloklu/koseli silueti korunur (create-character-v3 referansi yeniden hayal edip yuvarluyordu).
DESC = ("clawd mascot, blocky angular pixel art, square boxy rectangular body with "
        "flat straight edges and sharp corners, two big square black eyes, short stubby "
        "legs, orange-red, retro low-resolution sprite, NOT round, NOT smooth")

if os.path.exists(IDFILE) and "--force" not in sys.argv:
    cid = open(IDFILE).read().strip()
    print(f"zaten var: character_id={cid}  (yeniden icin: --force). GET ile yapisini dokuyorum:")
else:
    if not os.path.exists(BASE):
        raise SystemExit("out/clawd_base.png yok")
    if os.path.exists(IDFILE):                       # eskiyi arsivle (sunucuda da durur)
        old = open(IDFILE).read().strip()
        open(IDFILE + ".prev", "a").write(old + "\n")
        print("eski character_id arsivlendi:", old)
    print("clawd karakteri yaratiliyor (create-character-pro, method=rotate_character, side)...")
    res = lib.post("/create-character-pro", {
        "description": DESC,
        "method": "rotate_character",
        "reference_image": lib.b64img(BASE),
        "image_size": {"width": 64, "height": 64},
        "view": "side",
        "template_id": "mannequin",
        "no_background": True,
    })
    cid = res.get("character_id")
    job = res.get("background_job_id")
    print("character_id:", cid, " job:", job, " usage:", res.get("usage"))
    with open(IDFILE, "w") as f:
        f.write(cid or "")
    if job:
        done = lib.poll_job(job, every=4, timeout=420)
        print("job status:", done.get("status"))

# --- karakterin yapisini ogren: yonler, rotation frame'leri nasil geliyor ---
ch = lib.get(f"/characters/{cid}")
def keys(d, pre=""):
    if isinstance(d, dict):
        for k, v in d.items():
            if isinstance(v, (dict, list)):
                n = len(v)
                print(f"{pre}{k}: {type(v).__name__}({n})")
                if isinstance(v, list) and v and isinstance(v[0], dict):
                    print(f"{pre}  [0] keys:", list(v[0].keys()))
                elif isinstance(v, dict):
                    keys(v, pre + "  ")
            else:
                sv = str(v)
                print(f"{pre}{k}: {sv[:60]}")
print("\n=== GET /characters/%s yapisi ===" % cid)
keys(ch)
