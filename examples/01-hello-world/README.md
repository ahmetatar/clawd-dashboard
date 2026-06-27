# 01 — Hello World

CYD / ESP32-2432S028R ekranına yazı basan ilk proje. Rehberdeki **Adım 3.2 ("İlk ışık")**
karşılığı. Hiç clawd mantığı yok; tek soru: **kart programlanıyor mu, ekran açılıyor mu?**

## Çalıştır

```bash
cd examples/01-hello-world

# Derle
pio run

# Yükle + seri monitörü aç
pio run -t upload && pio device monitor
```

> `pio` komutu yoksa: VS Code + PlatformIO eklentisi kur, ya da
> `pip install platformio`.

## Başarı kriteri

- Ekranda yeşil **"Hello, clawd!"** + altında beyaz açıklama yazısı.
- Sağ üst köşede yarım saniyede bir yanıp sönen kırmızı nokta (heartbeat).
- Seri monitörde (115200 baud) `[clawd] ekran: 240 x 320` ve `heartbeat on/off`.

## Sorun giderme

| Belirti | İlk bakılacak yer |
|---|---|
| Ekran tamamen **beyaz / siyah**, yazı yok | `platformio.ini`'deki `TFT_*` pinleri — kart revizyonun farklı olabilir |
| Yazı var ama **renkler ters** (yeşil↔kırmızı) | `-D TFT_RGB_ORDER=TFT_BGR` satırını aç |
| Görüntü **negatif / soluk** | `-D TFT_INVERSION_ON=1` satırını aç |
| Ekran **yan / ters** duruyor | `main.cpp` içinde `tft.setRotation(0)` → 1/2/3 dene |
| Port bulunamadı / yüklenmiyor | `ls /dev/cu.*` ile portu gör; CH340/CP2102 sürücüsü gerekebilir |
| Arka ışık yanmıyor | `-D TFT_BL=21` ve `-D TFT_BACKLIGHT_ON=HIGH` doğru mu |

## Sonraki adım

Bu çalıştıysa en zor tuzağı aştın. Sıradakiler `examples/` altında:
`02-blink-rgb` (RGB LED), `03-ldr` (ışık sensörü), `04-touch` (dokunmatik + kalibrasyon).
