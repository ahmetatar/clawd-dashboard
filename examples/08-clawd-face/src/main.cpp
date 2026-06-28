// clawd examples — 08 clawd (GERCEK referans logosu, RGB565 bitmap, LANDSCAPE)
// Kullanicinin gonderdigi clawd gorseli 267x240 RGB565'e cevrilip dogrudan basilir
// (sprite/downsample yok; anti-aliasing dahil birebir). Ekran LANDSCAPE (rotation 1).

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "clawd_img.h"

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[clawd] 08 clawd (gercek logo, RGB565, landscape)");

  tft.init();
  tft.setRotation(1);                 // LANDSCAPE 320x240
  tft.fillScreen(tft.color565(239, 239, 234));   // krem zemin
  tft.setSwapBytes(true);             // RGB565 dizi icin bayt sirasi

  int x = (tft.width()  - CLAWD_IMG_W) / 2;       // ortala
  int y = (tft.height() - CLAWD_IMG_H) / 2;
  tft.pushImage(x, y, CLAWD_IMG_W, CLAWD_IMG_H, clawd_img);
  Serial.println("[clawd] gorsel basildi");
}

void loop() {}
