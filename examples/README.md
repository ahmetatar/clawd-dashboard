# clawd examples

CYD / ESP32-2432S028R kartını parça parça tanımak için küçük, bağımsız PlatformIO
projeleri. Her klasör **tek bir şeyi** öğretir ve tek başına derlenir/yüklenir.
Sıra, `../clawd-cyd-guide.md` rehberindeki pratik adımlarını takip eder.

| Proje | Ne öğretir | Rehber adımı |
|---|---|---|
| `01-hello-world` | Ekranı aç, yazı bas (en zor tuzak) | 3.2 |
| `02-blink-rgb` | Dahili RGB LED, active-low | 4.1 |
| `03-ldr` | Işık sensörü okuma (bu kartta LDR yok/arızalı) | 4.2 |
| `04-touch` | Dokunmatik (XPT2046, ayrı SPI/HSPI, polling) + kalibrasyon | 4.3 |
| `05-touch-irq` | Dokunmatik IRQ ile touch-to-wake (uyku/uyan) | 4.3+ |
| `06-wifi-health` | WiFi + mDNS (clawd.local) + GET /health | 6 |

Her projeyi kendi klasöründe çalıştır:

```bash
cd examples/01-hello-world
pio run -t upload && pio device monitor
```
