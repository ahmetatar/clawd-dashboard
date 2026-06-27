// clawd examples — 05 Touch IRQ (touch-to-wake)
// IRQ pini ile "boşta uyu, dokununca uyan". clawd'in gercek idle/wake davranisi.
//
// IRQ modunun ozelligi: XPT2046 dokununca IRQ hattini LOW yapar; kutuphane bu
// DUSEN KENARda 'isrWake' bayragini set eder. Yani bir dokunusu (tap) yakalamak
// icin idealdir -> uyandirma. (Surekli parmak takibi icin polling daha iyi; bkz 04.)
//
// Cihaz: AWAKE iken dokunmayi gosterir; UYKU_MS boyunca dokunulmazsa SLEEPING'e gecer
// (ekran kararir, Zzz). Dokununca (IRQ) aninda uyanir.

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

constexpr int T_CLK  = 25;
constexpr int T_CS   = 33;
constexpr int T_MOSI = 32;
constexpr int T_MISO = 39;
constexpr int T_IRQ  = 36;     // <-- 04'ten farki: IRQ pini de veriliyor
constexpr int PIN_B  = 17;     // mavi LED (active-low)

constexpr uint32_t UYKU_MS = 5000;   // bu kadar dokunulmazsa uyu

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);
XPT2046_Touchscreen ts(T_CS, T_IRQ);   // IRQ ile insa

bool sleeping = false;
uint32_t lastTouch = 0;

static void drawAwake() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("UYANIK", tft.width() / 2, tft.height() / 2 - 10, 4);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("5 sn dokunma -> uyku", tft.width() / 2, tft.height() / 2 + 30, 2);
}

static void drawSleep() {
  tft.fillScreen(TFT_NAVY);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_NAVY);
  tft.drawString("Zzz...", tft.width() / 2, tft.height() / 2 - 10, 4);
  tft.setTextColor(TFT_DARKGREY, TFT_NAVY);
  tft.drawString("dokun -> uyan", tft.width() / 2, tft.height() / 2 + 30, 2);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[clawd] 05-touch-irq (touch-to-wake, HSPI)");

  pinMode(PIN_B, OUTPUT);
  digitalWrite(PIN_B, HIGH);

  tft.init();
  tft.setRotation(0);

  touchSPI.begin(T_CLK, T_MISO, T_MOSI, T_CS);
  ts.begin(touchSPI);
  ts.setRotation(0);

  drawAwake();
  lastTouch = millis();
  Serial.println("[clawd] UYANIK. 5 sn dokunma -> uyku. IRQ ile uyandir.");
}

void loop() {
  bool touched = ts.touched();      // IRQ-gated: dusen kenarda true olur

  if (touched) {
    TS_Point p = ts.getPoint();
    digitalWrite(PIN_B, LOW);
    lastTouch = millis();

    if (sleeping) {
      sleeping = false;
      drawAwake();
      Serial.printf("[clawd] DOKUNUS -> UYANDIM!  (z=%d)\n", p.z);
    } else {
      tft.fillCircle(tft.width() / 2, tft.height() - 30, 5, TFT_GREEN);
      Serial.printf("[clawd] dokunus (uyanikken) z=%d x=%d y=%d\n", p.z, p.x, p.y);
    }
    delay(60);
    digitalWrite(PIN_B, HIGH);
  }

  // Bosta kalinca uyu
  if (!sleeping && millis() - lastTouch > UYKU_MS) {
    sleeping = true;
    drawSleep();
    Serial.println("[clawd] bosta kaldi -> UYKU (Zzz). Dokununca uyanir.");
  }

  delay(10);
}
