# 07 — Async events + permission (protokol gövdesi)

ESPAsyncWebServer ile clawd protokolünün asıl gövdesi: olaylar + **dokunmatik izin akışı**
(killer feature). `06`'daki yerleşik WebServer'dan non-blocking Async'e geçiş.

## Uçlar

| Uç | Ne yapar |
|---|---|
| `POST /e` | Fire-and-forget olay. Body = envelope `{k, d}`. **204** döner, ekran/LED tepki verir. |
| `POST /perm` | İzin sorusu. Body `{id, d:{tool,s,risk}}`. `{"pending":true}` döner, ekranda prompt açılır. |
| `GET /perm/{id}` | Karar yoklaması: `{"decision":"allow"\|"deny"}` ya da `{"pending":true}`. |
| `GET /health` | Canlılık: `{"fw":"0.2.0","caps":["led","touch"]}`. |

## Kritik mimari: callback ekrana dokunamaz

AsyncWebServer callback'leri **ayrı task'ta** çalışır. Display'e (SPI) callback içinden
dokunmak crash'e yol açar. Bu yüzden:
- `POST /e` callback'i olayı **thread-safe FreeRTOS kuyruğuna** push eder; çizim yalnız `loop()`'ta.
- `POST /perm` callback'i **mutex korumalı slota** yazar; prompt'u `loop()` çizer, dokunmatik çözer.

## İki önemli tuzak (bu örnekte çözüldü)

1. **JSON handler GET'i kaçırıyor.** `AsyncCallbackJsonWebHandler("/perm")` varsayılan olarak
   `/perm/7`'yi de + her method'u yakalar → `GET /perm/{id}` POST handler'ına takılıp state'i
   bozar. Çözüm: `handler->setMethod(HTTP_POST)`.
2. **Regex route.** `GET /perm/{id}` için `-D ASYNCWEBSERVER_REGEX=1` + `^\/perm\/([0-9]+)$`.

## Kurulum & çalıştır

`include/secrets.h`'yi doldur (bkz. `06`). Sonra:

```bash
cd examples/07-async-events
pio run -t upload
```

## Test (PC'den)

```bash
IP=<ekrandaki-ip>   # veya clawd.local

# olaylar — ekran/LED degisir
curl -s -XPOST http://$IP/e -H 'content-type:application/json' -d '{"k":"tool.pre","d":{"g":"exec","s":"npm test"}}'
curl -s -XPOST http://$IP/e -H 'content-type:application/json' -d '{"k":"git","d":{"op":"commit"}}'

# izin akisi — ekranda IZIN? acilir; UST yari=allow, ALT yari=deny
curl -s -XPOST http://$IP/perm -H 'content-type:application/json' -d '{"id":7,"d":{"tool":"Bash","s":"git push","risk":"med"}}'
curl -s http://$IP/perm/7     # dokununca -> {"decision":"allow"} | {"deny"}
```

## Başarı kriteri (doğrulandı)

- `/e` olayları 204 döner, ekran MERHABA/HACKING/ARIYOR/OOPS/GIT!/DEFRAG... arası geçer.
- `/perm` → ekranda **IZIN?** + tool + risk; **üst yarı=ALLOW (yeşil)**, **alt yarı=DENY (kırmızı)**.
- `GET /perm/{id}` dokunuştan sonra kararı **kalıcı** ve **idempotent** döner.

## Sonraki adım

PC tarafı **hook script'i**: Claude Code hook event'lerini (`PreToolUse`, `Stop`, ...)
normalize edip `clawd.local`'e POST'lar. İzin gerektiren araçlarda `POST /perm` + poll ile
bloklar. Bundan sonrası gerçek Claude Code entegrasyonu.
