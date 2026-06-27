# clawd — Hook + ESP32 Mimari Tasarımı

> Hook (PC köprüsü) ve ESP32 (PlatformIO) firmware'inin mimari planı. **Henüz kod yok** — katmanlar, klasör yapısı, kütüphane seçimleri ve açık kararlar.

## Genel yerleşim

İki parça, tek repo:

```
clawd-dashboard/
├── hooks/                  # PC tarafı — Claude Code'a takılan köprü
│   ├── dispatch            # tek giriş noktası (stdin JSON → HTTP POST)
│   ├── lib/                # normalize: tool-grup, git, mood, risk
│   └── clawd.config        # host (clawd.local), token, timeout'lar
│
├── firmware/               # ESP32 PlatformIO projesi
│   ├── platformio.ini
│   ├── src/
│   ├── include/
│   ├── lib/
│   └── data/               # animasyon kareleri, sesler (LittleFS/SD)
│
├── clawd-device-protocol.md
└── clawd-architecture.md
```

---

## A) Hook tarafı mimarisi (PC)

**Bağlanma noktası:** `~/.claude/settings.json` içindeki `hooks` bloğu. Her event (`PreToolUse`, `PostToolUse`, `Stop`, `UserPromptSubmit`, `Notification`, `SessionStart/End`, `PreCompact`) aynı tek script'i çağırır; script `hook_event_name`'e bakıp ne yapacağına karar verir.

**Tek dispatcher** (event başına ayrı script değil):

```
stdin (Claude Code'dan JSON)
   │
   ▼
dispatch
   ├─ parse: hook_event_name, tool_name, tool_input, ...
   ├─ normalize: tool→grup, "git commit/push" tespiti, mood, risk
   ├─ map: hook event → protokol kind (k)
   └─ emit:
        ├─ normal olay  → POST /e  (fire-and-forget, arka plan, ~0.3s timeout)
        └─ PreToolUse-izin → POST /perm + GET /perm/{id} poll (bloklar)
```

Sorumluluk ayrımı: **dispatcher = "ne oldu"yu bilen ve normalize eden kısım.** Cihaz aptal kalır.

**Mimari kararlar (hook tarafı):**
- **Dil:** `bash + curl + jq` (yalın, bağımlılıksız) vs `python` (mantık büyüyünce okunaklı).
- **İzin hook'u ayrı davranır:** `PreToolUse` matcher'ı sadece izin gerektiren araçlarda bloklar; gerisi fire-and-forget.
- **Hata dayanıklılığı:** cihaz kapalıysa `dispatch` sessizce 0 dönmeli — hook hatası Claude Code'u etkilememeli.

---

## B) ESP32 firmware mimarisi (PlatformIO)

### Katmanlar

```
┌─────────────────────────────────────────────┐
│  net/        WiFi + mDNS + AsyncWebServer    │  ← /e, /perm, /health route'ları
├─────────────────────────────────────────────┤
│  protocol/   JSON envelope → Event struct    │  ← ArduinoJson parse
├─────────────────────────────────────────────┤
│  core/       EventQueue + StateMachine       │  ← clawd state, mood engine, perm store
├─────────────────────────────────────────────┤
│  render/     ekran + animasyon oynatıcı       │  ← frame player
├─────────────────────────────────────────────┤
│  io/         touch · RGB LED · buzzer · LDR  │
└─────────────────────────────────────────────┘
```

### En kritik mimari karar: eşzamanlılık modeli

`ESPAsyncWebServer` callback'leri arka planda, ayrı bağlamda çalışır. **Display'e callback içinden dokunmak yasak** (SPI çakışması, crash). Doğru akış:

```
HTTP callback (async)                 render loop (loop() / kendi task'ı)
  parse → Event                          drain EventQueue
  EventQueue'ya push  ───────────────►   StateMachine.apply(event)
  /perm: PermStore'a kaydet,             animasyonu çiz, LED/ses sür
         ticket/id ver                   touch oku → PermStore.resolve(id)
```

**net ile render arasında thread-safe kuyruk** (FreeRTOS queue). Web sunucusu sadece kuyruğa yazar; tüm çizim tek yerde olur. İzin de aynı: callback `PermStore`'a "bekliyor" kaydı atar, `GET /perm/{id}` bunu okur, dokunmatik çözer.

**Dual-core:** AsyncTCP kendi task'ında döner; render `loop()`'ta ya da Core 1'e pinli ayrı task'ta. Başlangıç için `loop()` yeterli.

### Stack / kütüphane seçimleri

| Konu | Seçenek A | Seçenek B | Not |
|---|---|---|---|
| **Çizim** | LVGL (widget/animasyon hazır) | TFT_eSPI ham + sprite blit | LVGL şık ama ağır; kare-bazlı karakter için ham TFT_eSPI daha yalın/hızlı olabilir |
| **Animasyon deposu** | SD kart (bol yer) | LittleFS (flash, dahili) | CYD'de SD, TFT ile SPI paylaşır → dikkat; LittleFS dertsiz ama ~1.5MB |
| **Web sunucu** | ESPAsyncWebServer + AsyncTCP | senkron WebServer | Async öneri (non-blocking, izin tutması rahat) |
| **JSON** | ArduinoJson | — | standart |
| **WiFi kurulum** | WiFiManager | hardcode | captive portal öneri |
| **mDNS** | ESPmDNS | — | `clawd.local` |

**Öneri başlangıç stack'i:** TFT_eSPI + sprite tabanlı kare animasyon + LittleFS (en az sürpriz, en hızlı ilk ışık). LVGL'i menü/izin ekranı zenginleşince ekleriz.

### CYD'ye özgü tuzaklar (mimaride şimdiden hesaba katalım)

- **Board:** `ESP32-2432S028R`. `platformio.ini`'de `esp32dev` env + TFT_eSPI pin'leri **build_flags** ile (ILI9341, özel pinler) — en çok takılınan yer.
- **Dokunmatik:** XPT2046, çoğu CYD'de TFT'den **ayrı SPI** → ayrı SPI instance (bilinen gotcha).
- **Donanım pinleri (doğrulanacak):** RGB LED ~GPIO 4/16/17 (active-low), LDR ~GPIO34, buzzer ~GPIO26 (DAC), SD ayrı SPI.
- **SD vs TFT SPI paylaşımı:** ikisi aynı anda bus yönetimi ister → animasyonları LittleFS'e koymak ilk etapta kolaylaştırır.

---

## Açık mimari kararlar (kodlamadan önce)

1. **Hook dili:** bash+curl mu, python mı?
2. **Çizim katmanı:** TFT_eSPI ham sprite mi, LVGL mi?
3. **Animasyon deposu:** LittleFS mi, SD mi?
4. **Render yeri:** `loop()` mu, pinli ayrı task mı? (başlangıç: `loop()`)
5. **Animasyon kaynağı:** hazır clawd sprite seti var mı, yoksa ifadeleri sıfırdan mı üreteceğiz?

**Öneri varsayılanlar:** bash · TFT_eSPI · LittleFS · loop().
