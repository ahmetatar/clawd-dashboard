# clawd — ana uygulama

Fiziksel Claude Code maskotu. **CYD** (ESP32-2432S028R "Cheap Yellow Display")
üzerinde çalışır, WiFi'dan olay alır ve 5 ifade animasyonuyla tepki verir.

> Cihazın yeteneklerini keşfeden örnekler `examples/` altında; bu kök proje
> artık **gerçek uygulamadır**. Animasyon üretim hattı `tools/pixellab/`.

## Özellikler

- **5 ifade animasyonu** (idle / hacking / happy / think / oops), 64×64 RGB565,
  landscape ekrana 3× ölçekli, her biri kendi fps'inde. (`src/anims/`)
- **Olay protokolü** — `POST /e` (fire-and-forget, 204). Olay → animasyon eşlemesi
  (`tool.pre`→hacking, `git`→happy, `think`→think, `tool.post ok=false`→oops …).
  `GET /health` canlılık. mDNS: `clawd.local`.
- **Güç yönetimi** (ilk özellik — `src/power.h`):

  | Durum  | Tetik              | Davranış |
  |--------|--------------------|----------|
  | ACTIVE | olay/dokunma       | Tam parlaklık, animasyon oynar |
  | DIM    | 20 sn olaysız      | Arka ışık ~%11 (yumuşak fade), animasyon oynamaya devam |
  | SLEEP  | 90 sn olaysız      | Ekran söner, animasyon durur, CPU 80 MHz, WiFi modem-sleep |

  **Uyanma:** gelen `POST /e` paketi (WiFi association korunduğu için CPU'yu
  uyandırır) veya dokunmatik → anında tam parlaklığa fade + 240 MHz + animasyon.
  Eşikler/parlaklık `include/config.h` içinde.

## Yapı

```
platformio.ini        huge_app partition (5 anim + ağ yığını 1.25MB'a sığmaz)
include/config.h      tüm ayarlanabilir sabitler (pin, süre, parlaklık)
include/secrets.h      WiFi bilgileri (.gitignore'da)
src/main.cpp          setup/loop: WiFi + server + animasyon + güç
src/power.h           PowerManager: iki kademeli idle + arka ışık fade
src/anims.h           animasyon kaydı (fps, geçici mi, LED rengi)
src/anims/clawd_*.h   PixelLab ile üretilmiş kare verileri
```

## Derle & yükle

```sh
SSL_CERT_FILE=~/.platformio/system-ca-bundle.pem REQUESTS_CA_BUNDLE=~/.platformio/system-ca-bundle.pem \
  ~/.platformio/penv/bin/pio run -t upload --upload-port /dev/cu.usbserial-XXXX
```

Test:
```sh
curl http://clawd.local/health
curl -X POST http://clawd.local/e -H 'Content-Type: application/json' -d '{"k":"git","d":{"s":"commit"}}'
```
