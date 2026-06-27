# 04 — Touch (XPT2046 dokunmatik + kalibrasyon)

CYD'nin dirençli dokunmatiğini (XPT2046) okur, dokunduğun yere ekranda nokta çizer ve
ham (x,y,z) değerlerini seri porta basar. Rehberdeki **Adım 4.3** karşılığı — clawd'ın
"kafasına dokun = izin ver" killer feature'ının kalbi.

## En kritik nokta: ayrı SPI

CYD'de dokunmatik, ekrandan **ayrı bir SPI hattındadır.** Bu yüzden kodda dokunmatik için
kendi `SPIClass(VSPI)` örneğimizi açıp pinleri elle veriyoruz. Tek SPI sanıp gidersen
dokunmatik hiç okunmaz — bu, CYD'nin en bilinen tuzağıdır.

| Sinyal | GPIO |
|---|---|
| T_CLK | 25 |
| T_CS | 33 |
| T_MOSI | 32 |
| T_MISO | 39 |
| T_IRQ | 36 |

## Çalıştır

```bash
cd examples/04-touch
pio run -t upload
```

## Başarı kriteri

- Ekrana dokununca parmağının altında **yeşil nokta** beliriyor.
- Seri monitörde her dokunuşta `[clawd] ham x=.. y=.. z=.. -> ekran (..,..)`.

## Kalibrasyon (4 köşe)

İlk yüklemede nokta parmağından kaymış / eksenler ters olabilir — bu normal, ham
değerleri ekran piksellerine eşlememiz gerekiyor:

1. **4 köşeye tek tek dokun**, seri monitörden ham `x` ve `y` değerlerini oku.
2. En küçük/büyük ham `x` → `RAW_X_MIN/MAX`, ham `y` → `RAW_Y_MIN/MAX` olarak
   `main.cpp`'ye yaz.
3. Tekrar yükle; nokta artık parmağını takip etmeli.

| Belirti | Çözüm |
|---|---|
| Nokta var ama **eksenler ters** (sağa basınca sol) | `map()` içinde MIN↔MAX yer değiştir |
| **X ve Y çapraz** (yatay basınca dikey gidiyor) | `setRotation` veya p.x↔p.y takası gerekir |
| Dokunmatik **hiç okunmuyor** | Ayrı SPI pinlerini doğrula (25/33/32/39) |
| Sürekli rastgele nokta | `p.z` (basınç) eşiği ekle: çok düşük z'leri yok say |

## Sonraki adım

Dokunmatik + ekran + LED birlikte → clawd'ın izin akışı: dokun = allow, kaydır = deny.
Buradan sonrası `clawd-cyd-guide.md` Bölüm 5-6 (donanımı birleştir + WiFi köprüsü).
