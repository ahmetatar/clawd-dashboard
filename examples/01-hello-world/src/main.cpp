// clawd examples — 01 Hello World
// CYD / ESP32-2432S028R: ekrana yazı bas, arka planda renk testi yap.
// Amaç: kart programlanıyor mu + TFT pinleri doğru mu? Bunu geçersen en zor tuzağı aştın.

#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();  // pinler platformio.ini build_flags'ten gelir

// Ekrana ortalanmış tek satır yaz
static void drawCentered(const char *text, int y, uint16_t color, uint8_t font) {
  tft.setTextColor(color, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);  // ortala (Middle-Center)
  tft.drawString(text, tft.width() / 2, y, font);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("[clawd] 01-hello-world basliyor...");

  tft.init();
  tft.setRotation(0);  // 0/1/2/3 dene: ekran yan/ters gelirse burayi degistir
  tft.fillScreen(TFT_BLACK);

  Serial.printf("[clawd] ekran: %d x %d\n", tft.width(), tft.height());

  // Ana mesaj
  drawCentered("Hello, clawd!", 120, TFT_GREEN, 4);
  drawCentered("CYD ekran calisiyor", 160, TFT_WHITE, 2);
  drawCentered("ESP32-2432S028R", 185, TFT_DARKGREY, 2);

  Serial.println("[clawd] yazi basildi. setup bitti.");
}

void loop() {
  // Kanit: ekran gercekten suruluyor mu? Kenarda nabiz gibi atan bir nokta.
  static bool on = false;
  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last >= 500) {
    last = now;
    on = !on;
    tft.fillCircle(tft.width() - 12, 12, 5, on ? TFT_RED : TFT_BLACK);
    Serial.printf("[clawd] heartbeat %s\n", on ? "on" : "off");
  }
}
