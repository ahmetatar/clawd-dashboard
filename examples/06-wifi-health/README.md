# 06 — WiFi + mDNS + /health

WiFi'ye bağlanır, kendini `clawd.local` olarak duyurur, `GET /health` ile JSON döner.
clawd protokolünün **taşıma katmanının ilk adımı** — PC ↔ cihaz köprüsü ayağa kalkıyor.

Yerleşik `WebServer` (ESP32 çekirdeği) kullanılır — harici kütüphane yok. `/e` ve
`/perm` uçlarını eklerken (sonraki adım) ESPAsyncWebServer'a geçeceğiz.

## Kurulum: WiFi bilgileri

`include/secrets.h` dosyasını kendi ağ bilgilerinle doldur (bu dosya `.gitignore`'da,
şifre repoya girmez):

```c
#define WIFI_SSID "ev_wifi_adin"
#define WIFI_PASS "wifi_sifren"
```

> ⚠️ ESP32 yalnızca **2.4 GHz** WiFi'ye bağlanır (5 GHz desteklemez). Ağın 2.4 GHz
> bandını kullan.

## Çalıştır

```bash
cd examples/06-wifi-health
pio run -t upload
```

## Başarı kriteri

- Ekranda yeşil **"clawd ONLINE"** + cihazın **IP adresi** + `http://clawd.local/health`.
- Seri monitörde `WiFi OK. IP: ...` ve `HTTP sunucu :80 ayakta`.
- PC'den (aynı ağda):
  ```bash
  curl http://clawd.local/health
  # -> {"fw":"0.1.0","caps":["led","touch"]}
  ```
  `clawd.local` çözülmezse ekrandaki IP ile dene: `curl http://<IP>/health`.

## Sorun giderme

| Belirti | Sebep / çözüm |
|---|---|
| Ekranda "WiFi BAGLANAMADI" | SSID/şifre yanlış, ya da ağ 5 GHz (2.4 GHz kullan) |
| `clawd.local` çözülmüyor | mDNS bazı ağlarda/PC'lerde takılır; ekrandaki IP ile curl yap |
| curl bağlanmıyor ama IP ekranda var | PC ile cihaz **aynı ağda/VLAN'da** mı? Misafir ağı izolasyonu? |
| Sürekli bağlanıyor, IP yok | Sinyal zayıf olabilir; routera yaklaştır |

## Sonraki adım

`07` — ESPAsyncWebServer'a geçiş + `POST /e` (fire-and-forget olay) ve `POST /perm`
(dokunmatik izin akışı). clawd protokolünün asıl gövdesi. Hook script'i ile gerçek
Claude Code event'lerini `clawd.local`'e basacağız.
