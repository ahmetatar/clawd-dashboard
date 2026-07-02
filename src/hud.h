#pragma once
// clawd HUD — clawd'in etrafindaki BOS kenar bantlarina durum bilgisi yazan
// hafif katman. clawd merkezde 192x192 cizilir (y[24,216)); HUD YALNIZ ust bant
// (y[0,24)) ve alt bant (y[216,240)) icine yazar -> clawd'a ASLA dokunmaz.
//
// KOSE YERLESIMI (kullanici secimi = "HUD koseler"):
//   sol-ust : WiFi SINYAL CUBUKLARI (RSSI'den; bagli=yesil, kopuk=kirmizi)
//   sag-ust : (bos)
//   sol-alt : GUNCEL AKSIYON — kisa flavor metni ("Crafting..."), clawd-turuncu
//   sag-alt : CALISAN TOOL ADI (Bash / Grep / Glob / Edit ...), sonuk gri
//
// FONT: GFXFF (FreeSans) — platformio.ini'de LOAD_GFXFF=1. Eski font 2 cirkindi.
//
// CIZIM DISIPLINI: animasyon her frame merkezi tazeler; HUD ise SADECE ilgili
// kosenin verisi degistiginde (event) veya oturum sayaci icin 1 sn'de bir o
// koseyi yeniden cizer. Kose = once fume bg ile temizlenir, sonra icerik.

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

class Hud {
public:
  void begin(TFT_eSPI *tft) {
    _tft = tft;
    _bg  = tft->color565(BG_R, BG_G, BG_B);
    _wifiDirty = _actionDirty = _toolDirty = true;
  }

  // ---- setters: ilgili koseyi kirli isaretle ----
  // rssi: WiFi.RSSI() (dBm, negatif). connected=false -> tek kirmizi cubuk.
  // Periyodik cagrilir; yalniz cubuk sayisi/durum DEGISINCE yeniden cizer.
  void setWifi(bool connected, int rssi) {
    int bars = !connected     ? 1
             : rssi >= -60     ? 4
             : rssi >= -68     ? 3
             : rssi >= -76     ? 2
                               : 1;
    if (connected == _connected && bars == _bars) return;   // degisiklik yok
    _connected = connected;
    _bars = bars;
    _wifiDirty = true;
  }

  // Kisa aksiyon metni (flavor). Bos -> sol-alt temizlenir. Renk sabit clawd-turuncu.
  void setAction(const char *txt) {
    strlcpy(_action, txt ? txt : "", sizeof(_action));
    _actionDirty = true;
  }

  // Calisan tool adi (Bash/Grep/...). Bos -> sag-alt temizlenir.
  void setTool(const char *txt) {
    strlcpy(_tool, txt ? txt : "", sizeof(_tool));
    _toolDirty = true;
  }

  void markAllDirty() { _wifiDirty = _actionDirty = _toolDirty = true; }

  // Her loop cagrilir; yalniz kirli koseleri cizer. Uyku sirasinda main CAGIRMAZ.
  void render() {
    if (!_tft) return;
    if (_wifiDirty)   { drawWifi();   _wifiDirty   = false; }
    if (_actionDirty) { drawAction(); _actionDirty = false; }
    if (_toolDirty)   { drawTool();   _toolDirty   = false; }
  }

private:
  static constexpr int BANDMID_T = 12;    // ust bant dikey orta (0..24)
  static constexpr int BANDMID_B = 228;   // alt bant dikey orta (216..240)

  void clearRect(int x, int y, int w, int h) { _tft->fillRect(x, y, w, h, _bg); }

  // Sol-ust: 4 sinyal cubugu. Dolu cubuklar = _bars; bagli yesil, kopuk kirmizi.
  void drawWifi() {
    clearRect(0, 0, 40, 24);
    const int x0 = 5, base = 19, bw = 3, gap = 2;
    const int h[4] = {5, 9, 13, 17};
    uint16_t on  = _connected ? _tft->color565(70, 200, 110)
                              : _tft->color565(220, 70, 60);
    uint16_t off = _tft->color565(58, 62, 70);
    for (int i = 0; i < 4; i++) {
      int bx = x0 + i * (bw + gap);
      _tft->fillRect(bx, base - h[i], bw, h[i], (i < _bars) ? on : off);
    }
  }

  // Sol-alt: flavor metni, SABIT clawd-turuncu, Font 2 (ilk kullandigimiz).
  void drawAction() {
    clearRect(0, 216, 205, 24);
    if (!_action[0]) return;
    _tft->setTextFont(2);                      // free font'u temizler
    _tft->setTextDatum(ML_DATUM);
    _tft->setTextColor(CLAWD_ORANGE, _bg);
    _tft->drawString(_action, 6, BANDMID_B, 2);
  }

  // Sag-alt: calisan tool adi, sonuk gri, Font 2.
  void drawTool() {
    clearRect(205, 216, 115, 24);
    if (!_tool[0]) return;
    _tft->setTextFont(2);
    _tft->setTextDatum(MR_DATUM);
    _tft->setTextColor(_tft->color565(150, 150, 162), _bg);
    _tft->drawString(_tool, 315, BANDMID_B, 2);
  }

  TFT_eSPI *_tft = nullptr;
  uint16_t _bg = 0;
  bool     _connected  = false;
  int      _bars       = 1;
  char     _action[32] = {0};
  char     _tool[16]   = {0};
  bool     _wifiDirty = true, _actionDirty = true, _toolDirty = true;
};
