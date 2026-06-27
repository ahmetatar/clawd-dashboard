# clawd Masaüstü Maskotu — Fikir Dökümanı

> **Konsept:** Claude Code maskotu **clawd**'ı, ESP32 tabanlı bir TFT ekran cihazında (CYD — "Cheap Yellow Display" / ESP32-2432S028R) fiziksel olarak yaşatmak. Ben (Claude Code) çalışırken cihaz gerçek zamanlı, sevimli ve komik tepkiler verir.

---

## Donanım: Elimizdeki kart (CYD / ESP32-2432S028R)

Robotistan'ın listesindeki temel speclere ek olarak bu kartta tipik olarak şunlar var:

- **2.8" renkli TFT ekran (ILI9341, 240x320)** → clawd'ın yüzü / animasyon tuvali
- **Dirençli dokunmatik (XPT2046)** → kafasına dokun, sev, izin ver/iptal et
- **RGB LED** → duygu durumu / "düşünüyorum" nefes ışığı
- **LDR ışık sensörü** → oda karanlıksa "uyku" modu
- **Hoparlör çıkışı** → Tamagotchi tarzı ufak sesler
- **SD kart yuvası** → animasyon kareleri, sesler, clawd skinleri
- **WiFi + BLE** → PC ile kablosuz haberleşme
- **USB-C** → güç + programlama + seri haberleşme

---

## Beyin: Claude Code hook event'leri

Tüm fikirlerin temeli bu sinyaller. Claude Code hook'ları her olayda stdin'e JSON basar; bunu cihaza akıtınca clawd canlanır.

| Hook olayı | Ne zaman tetiklenir | Elimizdeki veri |
|---|---|---|
| `SessionStart` | Oturum başlar/resume | cwd, model |
| `UserPromptSubmit` | Sen prompt gönderince | promptun metni |
| `PreToolUse` | Bir araç çalışmadan **önce** | araç adı + inputu (üstelik **bloklayabilir**) |
| `PostToolUse` | Araç bitince | sonuç / hata |
| `Notification` | İzin gerekince / boşta kalınca | mesaj |
| `Stop` | Cevabımı bitirince | — |
| `SubagentStop` | Bir alt-ajan bitince | — |
| `PreCompact` | Context dolup sıkıştırmadan önce | — |
| `SessionEnd` | Oturum biter | — |

Ek kanal: **statusline** script'i her render'da model + context %'si + maliyet bilgisini verir → "canlı sağlık göstergesi" için.

---

## A) Canlı durum yansıtma — clawd ne yaptığımı "yaşar"

- **Araca göre kostüm/animasyon.** `PreToolUse`'taki `tool_name`'e göre clawd kılık değiştirir:
  - `Bash` → hacker modu, terminal tutar, gözlükler iner 🕶️
  - `Edit`/`Write` → eline kalem alır, yazıyor
  - `Read`/`Grep`/`Glob` → büyüteçle bir şey arıyor 🔍
  - `WebFetch`/`WebSearch` → dürbünle ufka bakar / internette sörf yapar 🏄
  - `Task`/subagent → clawd **kendini klonlar**, mini-clawd'lar koşuşturur (fan-out görselleştirmesi!)
- **"Düşünüyorum" nefes ışığı.** Ben düşünürken RGB LED, Claude Code'un kendi spinner'ı gibi yavaşça nefes alır. Cevap bitince (`Stop`) söner.
- **Context/enerji barı.** Statusline'dan context %'sini al, ekranda clawd'ın "kahve/enerji" çubuğu olarak göster. Doldukça clawd yorgunlaşır.

## B) Komik olay tepkileri — projenin ruhu

- **Hata = komedi.** `PostToolUse`'ta sonuç hatalıysa clawd facepalm / "ıngh" sesi. Test geçti → konfeti + yeşil LED + "ta-da!". Test patladı → ufak panik + sad trombone 🎺.
- **`git commit` kutlaması.** Bash inputunda `git commit` görürse clawd bayrak diker; `git push` → roket fırlatır 🚀.
- **`PreCompact` = "beyin defragı".** Context sıkışınca clawd kafasını tutup "uff hafızam doldu" animasyonu yapar, sonra silkinip devam eder.
- **Prompt'una tepki.** `UserPromptSubmit`'te metne bakıp: "fix the bug" → parmaklarını çıtlatır; çok uzun prompt → gözleri büyür "vay be"; küfür/sinir → anlayışlı bakar 😅. (Basit keyword/sentiment yeter.)
- **Uzun komut bekleyişi.** Bash uzun sürerse (build vb.) clawd ıslık çalıp ayak sallar, sıkılır.

## C) Fiziksel etkileşim — cihazdan **Claude Code'a** (killer feature)

`PreToolUse` hook'u **senkron çalışır ve aracı bloklayabilir.** Bu yüzden:

- **Dokunarak izin ver/reddet.** İzin gerektiren bir araçta clawd ekrana vurur, LED atar, "hey!" çalar. **Kafasına dokun → "evet çalıştır"**, yana kaydır → "iptal". Hook script'i cevabını dokunmatikten bekleyip allow/deny döner. Monitöre bakmadan fiziksel izin yönetimi.
- **"Bekliyorum" hatırlatıcısı.** `Stop`/`Notification` sonrası clawd sana bakıp ayak tıklatır; uzun süre dönmezsen (LDR ışık da kısıksa) uyuyakalır 😴.
- **Pet et.** Boşta clawd'a dokununca mırlar / kalp çıkarır.

## D) Donanımı sömüren dokunuşlar

- **LDR**: oda kararınca "geç oldu" modu, clawd esner.
- **Hoparlör**: her olaya Tamagotchi tarzı ufak çıtırtılar.
- **SD kart**: animasyon kareleri + ses + farklı clawd skinleri.

## E) İleri seviye

- **Sesli "clawd, ne yapıyorsun?"** → I2S mikrofon eklenir, ham ses PC'ye gider, Claude özet verir, clawd dudak oynatır.
- **Multi-agent panosu.** Workflow/Task ile birçok ajan koşarken her birini ekranda mini clawd olarak gör.
- **Günün özeti** (`SessionEnd`): kaç araç, kaç dosya, kaç commit — clawd küçük rapor sunar.

---

## Köprü mimarisi

```
Claude Code
   │  hooks → stdin'e JSON
   ▼
küçük local script (Python/Node)  ──serial veya WebSocket──►  ESP32 (CYD)
   │  olayı normalize edip yollar                              clawd: anim + ses + LED + dokunma
   ▲                                                            │
   └───────────  dokunmatik cevap (izin: allow/deny)  ◄────────┘
```

- **v1:** Cihaz zaten USB-C ile aynı makinede → **seri port** üzerinden konuş (WiFi kurulumu yok, gecikme minimum).
- **v2:** WebSocket'e geç, kablosuz tak.

> A ve B'deki her şey doğrudan **gerçek hook event'lerinden** gelir, tahmin yok. Sadece prompt'a göre sentiment/keyword tepkileri biraz heuristik.

---

## Önerilen yol haritası

1. **Olay protokolü** — hook JSON → cihaza giden kompakt mesaj formatı (beynin omurgası).
2. **Hook + köprü script'i** — birkaç olayı (`PreToolUse`, `Stop`) cihaza basıp seri porttan doğrula.
3. **Cihaz tarafı** — LVGL ile basit clawd + 2-3 state, gelen mesaja göre animasyon değişir.
4. **İzin ver/reddet dokunmatik akışı** (killer feature).
5. **Ses + LED + LDR + skinler** ile zenginleştirme.

---

## Açık sorular

- clawd görseli: hazır sprite/art seti var mı, yoksa birkaç ifadeli basit clawd yüzünü sıfırdan mı çizelim?
- Cihaz hep USB ile aynı makinede mi (seri yeterli) yoksa kablosuz/WiFi mı?
