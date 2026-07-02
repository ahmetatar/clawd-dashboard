#!/usr/bin/env python3
# clawd DETERMINISTIK animasyon modulu — PixelLab'in temiz karakter frame'i uzerinde
# kod ile hareket. Amac: HER sahnede clawd AYNI boyut/konum (sabit 64x64 tuval),
# hareket temiz (piksel yeniden cizilmez -> bozulma yok), ifade farki ayirt-edici ogeyle.
#
#   python3 clawd_anim.py <ad>      # idle | hacking  (out/anim_<ad>/ + out/<ad>_montage.png)
#   python3 clawd_anim.py all
import sys, os, glob, shutil, math
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
BG = (239, 239, 234)

# --- kanonik clawd: sadik (rotate_character) karakterin temiz south rotation'i ---
# Sabit yol; yoksa char_zip2 export'undan cek.
_srcpath = os.path.join(HERE, "out/clawd_south.png")
if not os.path.exists(_srcpath):
    _srcpath = glob.glob(os.path.join(HERE, "out/char_zip2/*/rotations/south.png"))[0]
_src = Image.open(_srcpath).convert("RGBA")
CLAWD = _src.crop(_src.getchannel("A").getbbox())
CW, CH = CLAWD.size                       # 44 x 30
_px = CLAWD.load()

# SABIT yerlesim — tum animasyonlarda ayni (boyut + konum tutarli)
CANVAS = 64
CX = (CANVAS - CW) // 2                    # yatay orta
CY = (CANVAS - CH) // 2                     # DIKEY ORTA (tum sahnelerde clawd ekran ortasinda)

# clawd yapisi (piksel analizinden)
FACE = (214, 82, 56, 255)
_dark = [(x, y) for y in range(CH) for x in range(CW) if _px[x, y][3] > 0 and sum(_px[x, y][:3]) < 170]
EYE_COL = _px[_dark[0][0], _dark[0][1]]
EYES = _dark                                       # goz pikselleri
ARM_Y = range(7, 15)                               # kollarin y bandi
LARM = [(x, y) for y in ARM_Y for x in range(0, 7)  if _px[x, y][3] > 0]   # sol kol (govde disi)
RARM = [(x, y) for y in ARM_Y for x in range(37, CW) if _px[x, y][3] > 0]  # sag kol
# ayaklar (y23-29): sol cift x7-17, sag cift x26-36
LEGS_L = [(x, y) for y in range(23, CH) for x in range(7, 18)  if _px[x, y][3] > 0]
LEGS_R = [(x, y) for y in range(23, CH) for x in range(26, 37) if _px[x, y][3] > 0]

def _shift(im, pixels, dx, dy):
    """pixels bolgesini (dx,dy) kaydir: eskiyi temizle, orijinal rengiyle yeniden ciz."""
    if not (dx or dy): return
    p = im.load()
    saved = [(x, y, im.getpixel((x, y))) for (x, y) in pixels]
    for (x, y) in pixels: p[x, y] = (0, 0, 0, 0)
    for (x, y, col) in saved:
        nx, ny = x + dx, y + dy
        if 0 <= nx < CW and 0 <= ny < CH: p[nx, ny] = col

def _happy_eyes(im):
    """Gozleri sil, resimdeki gibi INCE (1px) temiz > (sol) ve < (sag) ciz."""
    p = im.load()
    for (x, y) in EYES: p[x, y] = FACE
    gt = [(11,3),(12,4),(12,5),(11,6)]     # ince ">" vertex sagda
    lt = [(32,3),(31,4),(31,5),(32,6)]     # ince "<" vertex solda
    for (x, y) in gt + lt:
        if 0 <= x < CW and 0 <= y < CH: p[x, y] = EYE_COL

def clawd_variant(eye_dx=0, larm_dy=0, rarm_dy=0, eyes="normal",
                  lleg_dy=0, rleg_dy=0):
    """clawd kopyasi. eyes='happy' -> > < ; kollar VE ayaklar ayni yontemle (dikey) kaydirilir."""
    im = CLAWD.copy(); p = im.load()
    # --- gozler ---
    if eyes == "happy":
        _happy_eyes(im)
    elif eye_dx:
        for (x, y) in EYES: p[x, y] = FACE
        for (x, y) in EYES:
            nx = min(CW - 1, max(0, x + eye_dx)); p[nx, y] = EYE_COL
    # --- kollar VE ayaklar: hepsi dikey _shift (kol yontemi -> temiz, bozulmasiz) ---
    _shift(im, LARM, 0, larm_dy)
    _shift(im, RARM, 0, rarm_dy)
    _shift(im, LEGS_L, 0, lleg_dy)
    _shift(im, LEGS_R, 0, rleg_dy)
    return im

# --- kalp (kucuk), cesitli ekran noktalarinda cikip kaybolur ---
HEART = (222, 40, 44, 255); HEART_HI = (255, 90, 80, 255)   # kirmizi
_HEART_PX = [(1,0),(3,0),(0,1),(1,1),(2,1),(3,1),(4,1),
             (0,2),(1,2),(2,2),(3,2),(4,2),(1,3),(2,3),(3,3),(2,4)]
def draw_heart(c, cx, cy):
    p = c.load()
    for (x, y) in _HEART_PX:
        px_, py_ = cx + x, cy + y
        if 0 <= px_ < CANVAS and 0 <= py_ < CANVAS:
            p[px_, py_] = HEART
    if 0 <= cx+1 < CANVAS and 0 <= cy+1 < CANVAS: p[cx+1, cy+1] = HEART_HI  # parlak nokta

def base_canvas():
    return Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 0))

def place(canvas, clawd_img, dx=0, dy=0):
    canvas.alpha_composite(clawd_img, (CX + dx, CY + dy))

# ---------- modern klavye (yukaridan bakis, renkli Enter + aksan tuslar) ----------
KB_CASE = (40, 38, 56, 255)      # kasa (koyu cerceve)
KB_DECK = (60, 58, 78, 255)      # ic yuzey
KB_LIP  = (30, 28, 44, 255)      # on kalinlik (alt kenar)
CAP     = (168, 164, 192, 255)   # tus kapagi (acik gri)
CAP_SH  = (112, 108, 138, 255)   # tus alt golge (3B his)
CAP_DN  = (96, 92, 120, 255)     # basili tus
ENTER   = (232, 120, 70, 255)    # renkli Enter (clawd turuncusu)
ENTER_SH= (170, 78, 44, 255)
ESC     = (210, 90, 96, 255)     # aksan (Esc, kirmizimsi)
ACCENT  = (96, 180, 170, 255)    # aksan (birkac tus, teal)

def _cap(d, x0, y0, x1, y1, col, sh, pressed):
    if pressed:
        d.rectangle([x0, y0 + 1, x1, y1], fill=CAP_DN)
    else:
        d.rectangle([x0, y0, x1, y1 - 1], fill=col)
        d.line([(x0, y1), (x1, y1)], fill=sh)          # alt golge = 3B

def draw_keyboard(d, x, y, w, h, pressed):
    """Yukaridan bakisli modern klavye. Sik tus izgarasi + genis renkli Enter (sagda) +
    aksan tuslar + spacebar. pressed: basili tus indeksleri kumesi."""
    # kasa (yuvarlak koseli his icin koseleri kesiyoruz) + on kalinlik
    d.rectangle([x, y, x + w - 1, y + h - 1], fill=KB_CASE)
    d.rectangle([x, y + h - 2, x + w - 1, y + h - 1], fill=KB_LIP)
    for cx, cy in [(x, y), (x + w - 1, y), (x, y + h - 1), (x + w - 1, y + h - 1)]:
        d.point((cx, cy), fill=(0, 0, 0, 0))           # koseleri sil
    # ic deck
    dx0, dy0, dx1, dy1 = x + 2, y + 1, x + w - 3, y + h - 3
    d.rectangle([dx0, dy0, dx1, dy1], fill=KB_DECK)

    rows = 4; gap = 1
    kh = (dy1 - dy0 + 1 - (rows - 1) * gap) // rows
    right_col_w = 7                                     # sag blok (Enter) genisligi
    grid_x1 = dx1 - right_col_w - 1
    idx = 0
    for r in range(rows):
        ky = dy0 + r * (kh + gap)
        ky2 = ky + kh - 1
        if r == rows - 1:
            # alt sira: spacebar (genis) + iki yan tus
            _cap(d, dx0, ky, dx0 + 5, ky2, ACCENT if 0 else CAP, CAP_SH, idx in pressed); idx += 1
            _cap(d, dx0 + 7, ky, grid_x1 - 4, ky2, CAP, CAP_SH, idx in pressed); idx += 1   # spacebar
            _cap(d, grid_x1 - 2, ky, grid_x1, ky2, CAP, CAP_SH, idx in pressed); idx += 1
        else:
            cols = 9
            gw = grid_x1 - dx0
            kw = max(2, (gw - (cols - 1)) // cols)
            for c in range(cols):
                kx = dx0 + c * (kw + 1)
                if kx + kw > grid_x1: break
                # aksan tuslar: sol-ust Esc kirmizi, birkac tus teal
                col = CAP; sh = CAP_SH
                if r == 0 and c == 0: col = ESC
                elif (r * 9 + c) in (5, 12, 20): col = ACCENT
                _cap(d, kx, ky, kx + kw - 1, ky2, col, sh, idx in pressed); idx += 1
    # sag blok: renkli genis ENTER (ust 2 sirayi kaplar) + altinda iki tus
    ex0 = grid_x1 + 2; ex1 = dx1
    e_y0 = dy0; e_y1 = dy0 + 2 * kh + gap - 1
    if (99 in pressed):
        d.rectangle([ex0, e_y0 + 1, ex1, e_y1], fill=ENTER_SH)
    else:
        d.rectangle([ex0, e_y0, ex1, e_y1 - 1], fill=ENTER)
        d.line([(ex0, e_y1), (ex1, e_y1)], fill=ENTER_SH)
    for r in (2, 3):                                    # Enter altindaki tuslar
        ky = dy0 + r * (kh + gap)
        _cap(d, ex0, ky, ex1, ky + kh - 1, CAP, CAP_SH, (90 + r) in pressed)

# ---------- animasyonlar ----------
def anim_idle(n=8):
    """Yumusak nefes: dikey squash (ayaklar sabit), gozler cok hafif tarar."""
    out = []
    for i in range(n):
        sy = 1.0 - 0.06 * (0.5 - 0.5 * math.cos(i / n * 2 * math.pi))
        nh = max(1, int(round(CH * sy)))
        eye_dx = (0, 0, 1, 0, 0, 0, -1, 0)[i % 8]
        cl = clawd_variant(eye_dx=eye_dx)
        cl = cl.resize((CW, nh), Image.NEAREST)
        c = base_canvas()
        c.alpha_composite(cl, (CX, CY + (CH - nh)))       # ayaklar sabit (alt hizali)
        out.append(c)
    return out

def anim_hacking(n=8):
    """clawd SABIT durur; sol/sag kollari hafif asagi-yukari; gozler hafif tarar;
    onunde klavye, tuslari basiliyor. clawd zINPLAMAZ."""
    out = []
    kb_w = 54; kb_h = 20
    kb_x = (CANVAS - kb_w) // 2
    kb_y = CY + CH - 3                     # clawd ayak hizasinda, oniyle hafif ortusur
    NK = 30                                # izgara tus sayisi (~)
    for i in range(n):
        ph = i / n * 2 * math.pi
        larm = 1 if math.sin(ph) > 0 else 0          # kollar zit fazda, ±1px
        rarm = 1 if math.sin(ph) <= 0 else 0
        eye_dx = (0, 1, 0, 0, 0, -1, 0, 0)[i % 8]
        cl = clawd_variant(eye_dx=eye_dx, larm_dy=larm, rarm_dy=rarm)
        c = base_canvas()
        place(c, cl)                                  # clawd sabit konum/boyut
        d = ImageDraw.Draw(c)
        pressed = {(i * 3) % NK, (i * 5 + 4) % NK}    # her frame farkli izgara tusu
        if i % 4 == 3: pressed.add(99)                # ara sira Enter'a bas (renkli)
        draw_keyboard(d, kb_x, kb_y, kb_w, kb_h, pressed)
        out.append(c)
    return out

def anim_happy(n=8):
    """Sevincli: clawd ziplar (2 hop), kollar havada kalkar, 4 ayak zit fazda sag-sol
    mekik yapar, gozler > < , cevrede kalpler cikip kaybolur."""
    out = []
    # kalp olaylari: (x, taban_y, baslangic_frame) — age<life iken yukari suzulur
    hearts = [(3, 26, 0), (55, 30, 2), (29, 3, 4), (2, 12, 5), (55, 14, 6)]
    life = 4
    for i in range(n):
        bounce = round(4 * abs(math.sin(i / n * 2 * math.pi)))   # 0..4, iki hop
        airborne = bounce >= 3
        arm = -1 if airborne else 0                               # havada kollar kalkar
        # ayaklar marS: sol cift kalkar / iner, sag cift zit fazda (temiz dikey)
        lleg = -1 if i % 2 == 0 else 0
        rleg = 0 if i % 2 == 0 else -1
        cl = clawd_variant(eyes="normal", larm_dy=arm, rarm_dy=arm,   # kare gozler (eski)
                           lleg_dy=lleg, rleg_dy=rleg)
        c = base_canvas()
        place(c, cl, dy=-bounce)                                  # zipla (yukari)
        for (hx, hy, st) in hearts:                               # kalpler
            age = (i - st) % n
            if age < life: draw_heart(c, hx, hy - age)
        out.append(c)
    return out

ANIMS = {"idle": anim_idle, "hacking": anim_hacking, "happy": anim_happy}

def save(name, frames):
    dst = os.path.join(HERE, "out", f"anim_{name}")
    shutil.rmtree(dst, ignore_errors=True); os.makedirs(dst)
    for i, f in enumerate(frames): f.save(f"{dst}/frame_{i:02d}.png")
    s = 6; pd = 6; N = len(frames)
    mont = Image.new("RGB", (N * (CANVAS * s + pd) + pd, CANVAS * s + 2 * pd), (60, 60, 60))
    for i, f in enumerate(frames):
        bg = Image.new("RGBA", f.size, BG + (255,))
        mont.paste(Image.alpha_composite(bg, f).convert("RGB").resize((CANVAS * s, CANVAS * s), Image.NEAREST),
                   (pd + i * (CANVAS * s + pd), pd))
    mont.save(os.path.join(HERE, "out", f"{name}_montage.png"))
    print(f"{name}: {N} frame {CANVAS}x{CANVAS} -> out/anim_{name}/ , out/{name}_montage.png")

if __name__ == "__main__":
    which = sys.argv[1] if len(sys.argv) > 1 else "hacking"
    for nm in (list(ANIMS) if which == "all" else [which]):
        save(nm, ANIMS[nm]())
