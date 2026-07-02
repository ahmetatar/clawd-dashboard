# PixelLab → clawd animasyon pipeline

clawd maskotunu PixelLab API ile pixel-art üretip CYD ekranına (RGB565 frame'ler)
basmak için araçlar. **Reçete: sanat = PixelLab, hareket = deterministik kod (hibrit).**

## Neden hibrit?

- `animate-with-text-v2/v3` her frame'i yeniden çizer → karakter kayar, prop titrer (kötü).
- `create-character-v3` referansı **yeniden hayal eder** → clawd yuvarlaklaşır (kimlik kaybı).
- **`create-character-pro` + `method=rotate_character`** referansı yeniden çizmeden döndürür
  → clawd'ın **bloklu/köşeli kimliği korunur** (bunu kullanıyoruz).
- Karakterden `mode=v3` animasyon tutarlı ama yine de kafada/kolda hafif **bozulma** yapar.
- Çözüm: PixelLab'in ürettiği **tek temiz frame** + **kodla deterministik hareket**
  (`clawd_anim.py`) → sıfır bozulma, her sahnede clawd aynı boyut. İfade farkını
  **ayırt edici öğe** verir (hacking → klavye, happy → kıvılcım, vb.).

## Kurulum

`source secrets.sh` (API key, gitignored). `pip install Pillow`. TLS proxy için
build/flash'ta `SSL_CERT_FILE=~/.platformio/system-ca-bundle.pem` gerekir.

## Akış

```bash
cd tools/pixellab && source secrets.sh

# 0) kredi/plan
python3 00_balance.py

# 1) clawd taban sprite'i (image-to-pixelart)          -> out/clawd_base.png
python3 01_clawd_base.py A 64

# 2) clawd'i SADIK rig'li karaktere cevir (rotate_character, bloklu kalir)
#    -> character_id out/character_id.txt'e, 8 yon rotation char_zip2'ye
python3 04_character.py            # tekrar icin --force
#    kanonik on-yuz frame: out/clawd_south.png (clawd_anim.py bunu kullanir)

# 3a) DETERMINISTIK animasyon (idle, hacking...) — ONERILEN, temiz, 0 kredi
python3 clawd_anim.py all          # -> out/anim_<ad>/ + out/<ad>_montage.png

# 3b) (opsiyonel) karakterden v3/template animasyon — kredi harcar, bozulma olabilir
python3 05_char_anim.py <ad> v3 "<aksiyon>" south 8

# 4) frame'leri RGB565 header'a cevir                  -> out/<ad>.h
python3 03_png_to_header.py out/anim_idle    clawd_idle
python3 03_png_to_header.py out/anim_hacking clawd_hacking
```

## clawd_anim.py (asıl animasyon aracı)

- Sabit **64×64 tuval**, clawd her sahnede **aynı boyut/konum**.
- Kanonik clawd = `out/clawd_south.png` (rotate_character karakterin ön yüzü).
- Hareketler piksel yeniden çizmeden: göz tarama, kol oynatma (yan çıkıntılar y7–14),
  dikey nefes squash'ı. İfadeye özel öğeler kod ile (ör. `draw_keyboard`).
- Yeni ifade eklemek: `anim_<ad>()` yaz + `ANIMS`'e ekle.

## Notlar

- 64×64 RGB565 = 8KB/frame. Cihaz `S` katıyla büyütür (`320/64=5`, `192px` için S=3).
- Header'lar `examples/`'a kopyalanır; cihaz frame'leri sırayla `pushImage` ile basar.
- `examples/09-clawd-anim` = ESKİ v2-idle yaklaşımı (arşiv; hâlâ çalışır).
- Şemalar: `https://api.pixellab.ai/v2/openapi.json`, LLM dokümanı: `/v2/llms.txt`.
