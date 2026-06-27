# 02 — Blink RGB

CYD'nin **dahili** RGB LED'ini sürer. Rehberdeki **Adım 4.1** karşılığı. clawd'ın
"duygu ışığı"nın temeli: düşünürken nefes, test geçince yeşil, hata olunca kırmızı.

## Bağlantı

**Hiçbir şey bağlama.** RGB LED kartın üstünde dahili (genelde USB konnektörü yakınında
küçük bir LED). Sadece USB'yi tak.

| Renk | GPIO |
|---|---|
| Kırmızı | 4 |
| Yeşil | 16 |
| Mavi | 17 |

> **active-low:** pine `LOW` yazınca yanar, `HIGH` yazınca söner. Sezgiye ters ama
> CYD donanımı böyle (LED'in ortak ucu 3.3V'a bağlı).

## Çalıştır

```bash
cd examples/02-blink-rgb
pio run -t upload
```

## Başarı kriteri

LED sırayla: **kırmızı → yeşil → mavi → sarı → camgöbeği → mor → beyaz → kapalı**,
sonra baştan. Seri monitörde her renk için `[clawd] LED -> ...` satırı.

## Bu ünitedeki bilinen donanım notu

Bu karttaki RGB LED'in **kırmızı kanalı (GPIO4) yanmıyor**; yeşil (16) ve mavi (17)
sağlam. İzole teşhiste (sadece GPIO4 açık) hiç ışık çıkmadı → kırmızı alt-LED arızalı
(soğuk lehim / ölü die) görünüyor. Pin eşlemesi ve active-low mantığı doğru (yeşil+mavi
kanıtlıyor). clawd "duygu ışığı"nı kırmızısız (yeşil/mavi/camgöbeği) kuracağız.

## Sorun giderme

| Belirti | Sebep / çözüm |
|---|---|
| LED hiç yanmıyor | Pinler kart revizyonunda farklı olabilir; R/G/B = 4/16/17 doğrula |
| Renkler **ters** (LOW'da sönük) | active-low değil; `setRGB`'de LOW↔HIGH çevir |
| Yanlış renk adı yanıyor (kırmızı derken mavi) | Pin eşlemesi karışmış; PIN_R/G/B sırasını düzelt |
| Sürekli yanık, değişmiyor | `loop()` çalışmıyor olabilir; seri monitöre bak |

## Sonraki adım

`03-ldr` — ışık sensörü (LDR) okuma, "oda karardı → uyku modu" eşiği.
