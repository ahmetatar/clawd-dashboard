# 03 — LDR (ışık sensörü)

CYD'nin **dahili** LDR'sini GPIO34'ten okur. Rehberdeki **Adım 4.2** karşılığı.
clawd'ın "oda karardı → uyku modu" özelliğinin temeli.

## Bağlantı

**Hiçbir şey bağlama.** LDR kartın üstünde dahili küçük turuncu/kahve bir bileşen.
GPIO34 sadece-giriş (input-only) ADC pini.

## Çalıştır

```bash
cd examples/03-ldr
pio run -t upload
```

Sonra seri monitörü aç ve **sensörü elinle kapat** / oda lambasını değiştir; değerin
oynadığını gör.

## Başarı kriteri

- Seri monitörde sürekli `[clawd] LDR=NNNN -> aydinlik/KARANLIK` satırı.
- Sensörü elinle kapatınca değer belirgin değişiyor.
- Yeterince kararınca **mavi LED yanıyor** (uyku göstergesi).

## Bu ünitedeki bilinen donanım notu

**Bu karttaki LDR yok / arızalı.** GPIO34 ışığa hiç tepki vermedi, sabit ~2425 okudu.
LDR (GT36516) takılı değil veya açık olunca geriye yalnızca dahili 1M+1M bölücü kalır →
sabit orta değer. Bu, ESP32-2432S028R'de **bilinen bir sorun** (forumda "LDR ... not
working" başlıkları var). Bu kartın kırmızı LED'i de arızalı.

clawd için bloklayıcı değil: uyku modu **olaysızlık zamanlayıcısı** ile çalışır.
İstersen GPIO35'e harici LDR bağla (`3.3V–LDR–G35–10kΩ–GND`, sonra `PIN_LDR=35`).
GPIO21'i kullanma (arka ışık).

## Eşiği kendine göre ayarla

ADC 12-bit → değer **0–4095** arası. CYD'de tipik olarak **aydınlık = düşük, karanlık =
yüksek** değer verir (kartına göre ters olabilir).

1. Önce seri monitörden **aydınlık** ve **el ile kapalı (karanlık)** değerlerini oku.
2. İkisinin ortasını `main.cpp` içindeki `DARK_THRESHOLD`'a yaz.
3. Eğer ilişki ters çıktıysa (aydınlıkta yüksek değer), `loop()` içindeki
   `val >= DARK_THRESHOLD` karşılaştırmasını `val <= DARK_THRESHOLD` yap.

## Sorun giderme

| Belirti | Sebep / çözüm |
|---|---|
| Değer hiç değişmiyor | Yanlış pin; CYD LDR = GPIO34 doğrula |
| Değer hep 0 veya hep 4095 | Sensörü kapat/aç; sabitse pin/okuma sorunu |
| Mantık ters (aydınlıkta "karanlık") | Karşılaştırmayı `>=` ↔ `<=` çevir |
| Mavi LED hiç yanmıyor | Eşik aralık dışı; gerçek değerlere göre `DARK_THRESHOLD` ayarla |

## Sonraki adım

`04-touch` — dokunmatik (XPT2046) + 4 köşe kalibrasyonu. clawd'ın "kafasına dokun =
izin ver" killer feature'ının kalbi.
