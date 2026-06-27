// clawd examples — 04 Touch (XPT2046 + kalibrasyon)  [POLLING / calisan referans]
// Dokundugun yere ekranda nokta cizer, ham (x,y,z) degerleri seri porta basar.
// Dokunurken MAVI LED yanar (aninda geri bildirim).
//
// CYD'de dogrulanmis iki kritik nokta:
//  1) Dokunmatik AYRI bus'ta olmali: TFT_eSPI VSPI'yi kullanir -> dokunmatik HSPI'de.
//  2) SPIClass::begin() 'if(_spi) return' korumalidir -> DOGRU pinleri ts.begin()'DEN ONCE ver.
//  3) IRQ pini VERMEDEN insa et -> kutuphane yoklama (polling) modunda her cagriyi okur.

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// CYD dokunmatik pinleri (ekrandan ayri)
constexpr int T_CLK  = 25;
constexpr int T_CS   = 33;
constexpr int T_MOSI = 32;
constexpr int T_MISO = 39;
constexpr int PIN_B  = 17;   // mavi LED (active-low)

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(HSPI);          // AYRI bus: HSPI (TFT VSPI'de)
XPT2046_Touchscreen ts(T_CS);     // IRQ yok -> polling

// Kalibrasyon: 4 koseye dokunup ham min/max'i buraya yaz (seri monitorden oku).
int RAW_X_MIN = 1800, RAW_X_MAX = 3300;
int RAW_Y_MIN = 1600, RAW_Y_MAX = 3300;

static void header() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("clawd touch", tft.width() / 2, 6, 2);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("dokun: nokta + seri ham deger", tft.width() / 2, 26, 1);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[clawd] 04-touch (polling, HSPI)");

  pinMode(PIN_B, OUTPUT);
  digitalWrite(PIN_B, HIGH);

  tft.init();
  tft.setRotation(0);
  header();

  touchSPI.begin(T_CLK, T_MISO, T_MOSI, T_CS);  // DOGRU pinler once
  ts.begin(touchSPI);
  ts.setRotation(0);

  Serial.println("[clawd] 4 koseye dokun, ham min/max'i kalibrasyona yaz");
}

void loop() {
  TS_Point p = ts.getPoint();
  if (p.z >= 200) {                       // gercek dokunus
    digitalWrite(PIN_B, LOW);             // mavi yan

    int sx = map(p.x, RAW_X_MIN, RAW_X_MAX, 0, tft.width()  - 1);
    int sy = map(p.y, RAW_Y_MIN, RAW_Y_MAX, 0, tft.height() - 1);
    sx = constrain(sx, 0, tft.width()  - 1);
    sy = constrain(sy, 0, tft.height() - 1);

    tft.fillCircle(sx, sy, 4, TFT_GREEN);
    Serial.printf("[clawd] ham x=%4d y=%4d z=%4d  ->  ekran (%3d,%3d)\n",
                  p.x, p.y, p.z, sx, sy);
    delay(20);
  } else {
    digitalWrite(PIN_B, HIGH);            // mavi sonuk
  }
}
