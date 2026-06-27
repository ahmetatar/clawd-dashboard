# CYD (ESP32-2432S028R) — Tanıma & Pratik Rehberi

> clawd projesinin fiziksel beyni olan kartı sıfırdan tanımak ve elini kirletip pratik
> yapmak için Türkçe yol haritası. Henüz clawd firmware'i yazmıyoruz; önce **kartı
> tanıyor, her bir parçasını tek tek "merhaba" dedirterek** çalıştırıyoruz.

---

## 0) Bu kart tam olarak ne? (CYD efsanesi)

**CYD = "Cheap Yellow Display"** — topluluğun verdiği lakap. Resmî adı **ESP32-2432S028R**.
Üzerinde dokunmatik renkli bir ekranla birlikte gelen, ucuz (genelde ~150-250 TL) bir
ESP32 geliştirme kartı. Adındaki sayılar anlamlı:

- **2432** → ekran 240×320 piksel
- **S028** → 2.8 inç
- **R** → dirençli (resistive) dokunmatik panel

Neden bu kadar popüler? Çünkü bir ekran + dokunmatik + ESP32 + USB + SD yuvası + ışık
sensörü + LED + ses çıkışı **tek kartta, lehimsiz** geliyor. clawd için biçilmiş kaftan:
clawd'ın yüzünü ekranda yaşatacağız, dokunarak izin vereceğiz, LED ile "düşünüyorum"
diyecek.

---

## 1) Donanımı parça parça tanıyalım

### 1.1 Beyin — ESP32 (ESP32-D0WD / WROOM-32)
- **Çift çekirdek** Xtensa LX6, ~240 MHz.
- **WiFi (2.4 GHz) + Bluetooth/BLE** dahili → clawd protokolünde `clawd.local` mDNS
  webhook'unu bunun WiFi'si taşıyacak.
- **~520 KB SRAM** + **4 MB flash** (firmware + LittleFS animasyon deposu buraya sığar).
- Çift çekirdek bizim için kritik: bir çekirdek WiFi/HTTP'yi döndürürken diğeri ekranı
  çizebilir (mimari dokümandaki "render ayrı task" kararı buradan geliyor).

### 1.2 Ekran — 2.8" TFT, ILI9341 sürücü, 240×320, SPI
- clawd'ın **yüzü ve animasyon tuvali.**
- Sürücü çipi **ILI9341** — TFT_eSPI kütüphanesinin birinci sınıf desteklediği bir çip.
- **SPI** ile sürülür. Renk: 16-bit (65K renk, RGB565).
- ⚠️ **En çok takılınan yer:** Bu kartta TFT_eSPI'nin pin tanımlarını `build_flags`
  ile doğru girmek. Yanlışsa ekran beyaz/ters/renkleri bozuk gelir. (Bunu Adım 3'te
  hazır reçeteyle çözeceğiz.)

### 1.3 Dokunmatik — XPT2046 (dirençli)
- Ekrana **bas-bırak** mantığıyla çalışır (kapasitif değil; tırnak/eldiven de algılar).
- clawd'da: **kafasına dokun = "izin ver"**, kaydır = "iptal" → killer feature bu.
- ⚠️ **Gotcha:** XPT2046, çoğu CYD'de ekranla **ayrı bir SPI hattında.** Yani kodda
  ekran için bir SPI, dokunmatik için ayrı bir `SPIClass` instance açman gerekir.
  Aynı bus sanıp tek SPI ile gidersen dokunmatik okunmaz.

### 1.4 RGB LED (dahili, tek adet)
- Kartın üstünde tek bir RGB LED. clawd'ın **duygu ışığı**: düşünürken nefes alır,
  test geçince yeşil, hata olunca kırmızı.
- ⚠️ Genelde **active-low** (LOW yazınca yanar, HIGH yazınca söner — sezgiye ters).
- Tahmini pinler (doğrulanacak): **R=GPIO4, G=GPIO16, B=GPIO17.**

### 1.5 LDR — ışık sensörü
- Foto-direnç; ortam ışığını analog okur. clawd'da **"oda karardı → uyku modu"** için.
- Tahmini pin: **GPIO34** (sadece-giriş ADC pini). `analogRead()` ile okunur.

### 1.6 Ses — küçük hoparlör / buzzer çıkışı
- Tamagotchi tarzı ufak "çıtırtılar" için. ESP32'nin **DAC** çıkışından sürülür.
- Tahmini pin: **GPIO26** (DAC1). Basit ton için `tone()` benzeri ya da DAC ile dalga.

### 1.7 microSD kart yuvası
- Animasyon kareleri, sesler, farklı clawd skinleri için bol depo.
- ⚠️ **Önemli tuzak:** SD kart **TFT ile aynı SPI bus'ı** paylaşır. İkisini aynı anda
  yönetmek bus çakışması riski taşır. Bu yüzden mimaride **"ilk etapta animasyonları
  SD yerine LittleFS'e (dahili flash) koy"** kararı var — daha az sürpriz.

### 1.8 USB ve güç
- **USB** (kartına göre USB-C veya micro-USB) → hem güç, hem programlama, hem seri
  haberleşme. Üzerinde **CH340** veya **CP2102** USB-seri çevirici çipi var; sürücüsü
  gerekebilir (Adım 2).
- 5V USB'den beslenir; 3.3V regülatör kartta.

### 1.9 Boş GPIO konnektörleri (JST)
- Kenarlarda birkaç boş pin çıkışı var (I2C/genel GPIO). İleride **I2S mikrofon**
  ("clawd, ne yapıyorsun?" özelliği) buraya takılabilir.

### Pin özeti (cheat-sheet — kodlamadan önce mutlaka doğrula)

| Parça | Çip | Pin(ler) | Not |
|---|---|---|---|
| TFT ekran | ILI9341 | SPI (SCK/MOSI/MISO/CS/DC/RST/BL) | build_flags ile gir |
| Dokunmatik | XPT2046 | **ayrı SPI** + IRQ | ayrı SPIClass şart |
| RGB LED | — | R=4, G=16, B=17 | **active-low** |
| LDR | — | 34 | sadece-giriş, ADC |
| Buzzer/ses | DAC | 26 | DAC1 |
| SD kart | — | SPI (TFT ile paylaşımlı) | çakışmaya dikkat |

> ⚠️ CYD'nin birden çok donanım revizyonu var; pinler **senin elindeki kartta**
> farklı olabilir. Adım 4'te bir "pin tarama" eskizi ile bunları kendi kartında
> doğrulayacağız. Tahmine güvenme, ölç.

---

## 2) Ortam kurulumu (kodlamadan önce, bir kez)

1. **PlatformIO kur.** VS Code + PlatformIO eklentisi (Arduino IDE'den daha rahat,
   mimari de PlatformIO üzerine kurulu).
2. **USB-seri sürücüsü.** Kartı tak; macOS'ta görünmüyorsa CH340 veya CP2102
   sürücüsünü yükle. Terminalde portu doğrula:
   ```bash
   ls /dev/tty.* /dev/cu.*      # macOS'ta /dev/cu.usbserial-XXXX gibi görünür
   ```
3. **Boş bir PlatformIO projesi** aç (board: `esp32dev`). Henüz clawd firmware'i değil —
   sadece kartı tanıma deneyleri.
4. **Seri monitör** alışkanlığı: 115200 baud. Her deneyde `Serial.println()` ile ne
   olduğunu göreceğiz. (Senin gözün-kulağın bu.)

---

## 3) "İlk ışık" — kartın yaşadığını gör

> Amaç: hiçbir clawd mantığı yok. Sadece "kart programlanıyor mu, ekran açılıyor mu?"

**3.1 Blink (LED'i yak).** Donanıma ilk dokunuş. RGB LED'in bir bacağını yak-söndür.
active-low olduğunu burada hissedersin (LOW = yanar). Başarı kriteri: LED yanıp sönüyor
+ seri monitöre "blink" basıyor.

**3.2 Ekranı aç (TFT_eSPI "merhaba").** En kritik adım — pin reçetesi. `platformio.ini`'ye
TFT_eSPI'yi ekle ve CYD pinlerini `build_flags` ile ver:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = bodmer/TFT_eSPI
build_flags =
  -D USER_SETUP_LOADED=1
  -D ILI9341_2_DRIVER=1
  -D TFT_WIDTH=240
  -D TFT_HEIGHT=320
  -D TFT_MISO=12
  -D TFT_MOSI=13
  -D TFT_SCLK=14
  -D TFT_CS=15
  -D TFT_DC=2
  -D TFT_RST=-1
  -D TFT_BL=21
  -D TFT_BACKLIGHT_ON=HIGH
  -D SPI_FREQUENCY=55000000
  -D LOAD_GLCD=1
```
> Bu pinler tipik CYD içindir ama **kartına göre değişebilir** — ekran ters/beyaz
> gelirse ilk şüphelenilecek yer burası. Renkler tersse `TFT_RGB_ORDER` veya
> `TFT_INVERSION_ON/OFF` flag'leriyle oyna.

Kodda: ekranı temizle, `tft.fillScreen(TFT_BLACK)`, ortaya `tft.drawString("clawd")`.
Başarı kriteri: ekranda yazıyı görüyorsun. **Bunu geçtiysen en zor tuzağı aştın.**

**3.3 Renk testi.** Kırmızı/yeşil/mavi tam ekran bas. Renkler doğruysa pin sırası ve
RGB order doğru demektir.

---

## 4) Her sensörü tek tek "konuştur" (donanım envanteri)

> Amaç: cheat-sheet'teki her parçanın senin kartında **gerçekten hangi pinde** olduğunu
> kanıtlamak. Her biri ayrı küçük eskiz, her biri seri monitöre yazsın.

1. **RGB LED — 3 renk ayrı ayrı.** R, G, B pinlerini sırayla yak. Active-low olduğunu,
   doğru pinleri doğrula. (clawd'ın duygu ışığının temeli.)
2. **LDR — ışığı ölç.** `analogRead(34)` değerini döngüde seri monitöre bas. Elinle
   sensörü kapat → değer değişmeli. Karanlık/aydınlık eşiğini not al (uyku modu için).
3. **Dokunmatik (XPT2046) — en önemli pratik.** Ayrı `SPIClass` ile dokunmatiği başlat,
   bastığın (x, y) koordinatını seri monitöre bas. Ekranın 4 köşesine bas, ham
   değerleri not al → **kalibrasyon** için (ekran pikseli ↔ dokunmatik ham değer eşlemesi).
   Bu, clawd'ın "kafasına dokun = izin ver" özelliğinin kalbi.
4. **Ses — bip.** GPIO26'dan basit bir ton çıkar. Hoparlörden ses geliyor mu? clawd
   "ta-da" / "ıngh" seslerinin temeli.
5. **(Opsiyonel) SD kart.** Bir kart tak, dosya listesini seri monitöre bas. TFT ile
   bus çakışmasını burada gözlemle — bu yüzden ilk etapta LittleFS tercih ediyoruz.

Bu adımın sonunda elinde **kendi kartının doğrulanmış pin haritası** olacak. clawd
firmware'i yazarken tahminle değil bu haritayla çalışacaksın.

---

## 5) Donanımı birleştir — mini etkileşim

> Tek tek çalışan parçaları ilk kez bir araya getir. Hâlâ clawd değil; "donanım orkestrası".

- **Dokun → ekran tepki versin.** Ekrana dokununca bastığın yere bir nokta/daire çiz.
  (Dokunmatik + ekran birlikte.)
- **Dokun → LED + ses.** Dokununca LED yeşil yan + kısa bip. (3 parça birden.)
- **Karanlık → ekran kararsın.** LDR eşiğin altına inince arka ışığı kıs / ekranı
  uykuya al. (Sensör → davranış.)

Buraya geldiğinde kartın **tüm I/O'sunu** kontrol ediyorsun demektir. Artık clawd
karakterini giydirmeye hazırsın.

---

## 6) WiFi & ağ — clawd protokolüne köprü

> Donanımı tanıdıktan sonra, clawd protokolünün taşıma katmanını dene.

1. **WiFi'ye bağlan** (önce hardcode SSID/şifre, sonra WiFiManager captive portal).
2. **mDNS** ile kendini `clawd.local` yap (`MDNS.begin("clawd")`).
3. **ESPAsyncWebServer** kur, tek bir `GET /health` ucu aç → `{"fw":"0.1.0"}` dönsün.
4. PC'den dene:
   ```bash
   curl http://clawd.local/health
   ```
   Yanıt geldiyse **PC ↔ cihaz köprüsü ayakta** demektir. clawd protokolündeki diğer
   uçları (`POST /e`, `POST /perm`) bunun üstüne ekleyeceğiz.

---

## 7) Buradan sonrası — clawd'a geçiş

Donanımı ve ağı tanıdıktan sonra plan dosyalarındaki gerçek yol haritasına bağlanırsın:

1. **Olay protokolü** (`clawd-device-protocol.md`) — hook JSON → cihaz mesajı.
2. **Hook + köprü script'i** — Claude Code event'lerini `clawd.local`'e bas.
3. **Cihaz tarafı** — basit clawd yüzü + birkaç state (`thinking`, `hacking`, `oops`...).
4. **İzin ver/reddet dokunmatik akışı** — Adım 4.3'teki kalibrasyonun meyvesi.
5. **Ses + LED + LDR + skinler** ile zenginleştir.

---

## Pratik kontrol listesi (sırayla işaretle)

- [ ] PlatformIO + USB-seri sürücü kurulu, port görünüyor
- [ ] Blink çalışıyor (LED + seri monitör)
- [ ] **Ekranda yazı görünüyor** (en zor tuzak aşıldı)
- [ ] Renk testi doğru (RGB order tamam)
- [ ] RGB LED 3 renk ayrı ayrı, active-low doğrulandı
- [ ] LDR değeri elle değişiyor, eşik not edildi
- [ ] **Dokunmatik (x,y) okunuyor, 4 köşe kalibre edildi**
- [ ] Buzzer'dan ses çıkıyor
- [ ] Dokun → ekran+LED+ses birlikte tepki veriyor
- [ ] WiFi + mDNS + `/health` → `curl http://clawd.local/health` yanıt veriyor
- [ ] **Kendi kartımın doğrulanmış pin haritası** elimde

> İki yıldızlı (**) adımlar projenin geri kalanını kilitleyen kritik eşikler:
> ekranı açmak ve dokunmatiği kalibre etmek. Bu ikisini geçtiysen gerisi sevimlilik.
