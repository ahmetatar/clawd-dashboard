# 05 — Touch IRQ (touch-to-wake)

XPT2046'nin **IRQ pini** ile "boşta uyu, dokununca uyan" davranışı. clawd'ın gerçek
idle/wake akışının küçük bir provası. `04-touch`'ın IRQ'lu kardeşi.

## Polling (04) vs IRQ (05) — ne fark eder?

| | Polling (04) | IRQ (05) |
|---|---|---|
| Kurulum | `ts(T_CS)` | `ts(T_CS, T_IRQ)` |
| Nasıl çalışır | Her `loop()`'ta SPI'dan basınç okur | Dokunma IRQ'yu LOW yapar; düşen kenarda okur |
| Güçlü yanı | **Sürekli parmak takibi** (çizim, kaydırma) | **Tek dokunuş/uyandırma** (tap), boşta SPI trafiği yok |
| clawd'da | İzin akışı, kafaya dokunma, kaydırma | Uykudan uyandırma |

> IRQ modunda `touched()` esas olarak **düşen kenarı** (dokunma başlangıcı) yakalar;
> sürekli basılı tutarken her döngüde yeniden tetiklenmez. Bu yüzden IRQ "tap/wake"
> için ideal, sürekli izleme için polling daha rahat.

## Çalıştır

```bash
cd examples/05-touch-irq
pio run -t upload
```

## Başarı kriteri

1. Ekranda yeşil **"UYANIK"**.
2. **5 saniye dokunma** → ekran lacivert **"Zzz..."** (uyku).
3. Ekrana **dokun** → anında **"UYANDIM!"** + mavi LED çakar. Seri monitörde
   `DOKUNUS -> UYANDIM!`.

Bu çalışıyorsa bu kartta IRQ kesmesi de sağlıklı tetikleniyor demektir.

## Sorun giderme

| Belirti | Sebep / çözüm |
|---|---|
| Uykuya geçmiyor | `UYKU_MS` çok büyük; ya da bir şey sürekli dokunma sanıyor (gürültü) |
| Dokununca uyanmıyor | IRQ kesmesi tetiklenmiyor → bu kartta IRQ sorunlu; **polling'e (04) dön** |
| Hiç dokunma yok | Ayrı bus/pin: HSPI + 25/33/32/39, IRQ=36 doğrula |

## Sonuç

IRQ çalışıyorsa: clawd uyku modunda dokunuşla uyanabilir.
IRQ çalışmıyorsa: sorun yok — **04'teki polling** zaten yeterli (cihaz nasılsa sürekli
döngüde dokunmatiği yokluyor). Asıl üretimde polling tercih edilebilir.
