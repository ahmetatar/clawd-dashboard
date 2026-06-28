# PixelLab → clawd animasyon pipeline

clawd maskotunu PixelLab API ile pixel-art + animasyon olarak üretip CYD ekranına
(RGB565 frame'ler) basmak için araçlar.

## Kurulum

1. PixelLab hesabı aç, **API token**'ı al (Account/API settings).
2. `tools/pixellab/secrets.sh` içine yapıştır (`PIXELLAB_API_KEY="..."`). Bu dosya
   `.gitignore`'da — repoya girmez.
3. Her kullanımdan önce:
   ```bash
   source tools/pixellab/secrets.sh
   ```
   Python için `Pillow` gerekir (genelde kurulu): `pip install Pillow`.

## Akış

```bash
cd tools/pixellab
source secrets.sh

# 0) key + kredi dogrula
python3 00_balance.py

# 1) clawd taban sprite'i (A=gercek gorseli pixel-art'a cevir [sadik], B=stil referansli)
python3 01_clawd_base.py A 64        # -> out/clawd_base.png

# 2) animasyon uret (referans = clawd_base). Ad + aksiyon + boyut.
python3 02_animate.py idle  "idle breathing, gentle bob" 64
python3 02_animate.py happy "happy excited bounce" 64
python3 02_animate.py think "looking up thinking, tapping" 64

# 3) frame'leri tek RGB565 header'a cevir (cihaza)
python3 03_png_to_header.py out/anim_idle clawd_idle   # -> out/clawd_idle.h
```

Üretilen `out/<ad>.h` header'i bir firmware örneğine (ör. `examples/09-clawd-anim`)
kopyalanır; cihaz frame'leri sırayla `pushImage` ile landscape ekrana basar ve
07'deki olay protokolüyle eşlenir (`tool.pre exec`→hacking, `Stop`→idle ...).

## Notlar

- **Boyut/flash:** 64×64 RGB565 = 8KB/frame. 8 frame ≈ 64KB. Birkaç animasyon flash'a
  sığar; büyürse LittleFS/SD'ye taşı.
- **Maliyet:** her üretim USD kredi (64×64 ucuz). `00_balance.py` ile takip et.
- Şemalar `https://api.pixellab.ai/v2/openapi.json`; animasyon alanları (n_frames vb.)
  ilk gerçek çağrıda netleşir — scriptler savunmacı yazıldı, çıktıya göre ayarlanır.
