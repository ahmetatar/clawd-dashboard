#pragma once
// clawd guc yonetimi — iki kademeli idle durumu + yumusak arka isik fade.
//
//   ACTIVE  (tam parlaklik)                          <- her event/dokunma buraya doner
//     | T_DIM_MS boyunca olay yok
//   DIM     (kisik ~%11, animasyon doner)
//     | T_SLEEP_MS boyunca olay yok
//   SLEEP   (arka isik 0, animasyon durur, CPU 80MHz, WiFi modem-sleep)
//
// WiFi ayakta kaldigi icin gelen POST /e paketi CPU'yu uyandirir; main loop
// olayi kuyruktan alirken notifyActivity() cagirir -> ACTIVE'e doneriz.
// Bu sinif SADECE arka isik + zamanlamayi yonetir; animasyon/CPU/WiFi yan
// etkilerini main.cpp durum kenarlarina (edge) gore uygular.
#include <Arduino.h>
#include "config.h"

class PowerManager {
public:
  enum State { ACTIVE, DIM, SLEEP };

  void begin() {
    ledcSetup(BL_CH, BL_FREQ, BL_RES);
    ledcAttachPin(PIN_BL, BL_CH);   // tft.init()'ten SONRA cagir (BL pinini devralir)
    _cur = _target = BL_FULL;
    ledcWrite(BL_CH, _cur);
    _state = ACTIVE;
    _lastActivity = millis();
    _lastRamp = _lastActivity;
  }

  // Bir olay/dokunma oldu: idle sayacini sifirla (tick ACTIVE'e cekecek).
  void notifyActivity() { _lastActivity = millis(); }

  State state() const { return _state; }
  bool  asleep() const { return _state == SLEEP; }

  // Her loop cagrilir. Durumu gunceller ve arka isigi hedefe dogru fade eder.
  // Donus: durum bu tick'te DEGISTIYSE true (main yan etkileri edge'de uygular).
  bool tick() {
    uint32_t now = millis();
    uint32_t idle = now - _lastActivity;

    State next = (idle >= T_SLEEP_MS) ? SLEEP
               : (idle >= T_DIM_MS)   ? DIM
                                      : ACTIVE;
    bool changed = (next != _state);
    _state = next;
    _target = (next == SLEEP) ? BL_OFF : (next == DIM) ? BL_DIM : BL_FULL;

    // yumusak, bloklamayan fade
    if (_cur != _target && (now - _lastRamp) >= BL_RAMP_MS) {
      _lastRamp = now;
      if (_cur < _target) _cur = (_target - _cur > BL_STEP) ? _cur + BL_STEP : _target;
      else                _cur = (_cur - _target > BL_STEP) ? _cur - BL_STEP : _target;
      ledcWrite(BL_CH, _cur);
    }
    return changed;
  }

private:
  State    _state       = ACTIVE;
  uint32_t _lastActivity = 0;
  uint32_t _lastRamp     = 0;
  uint16_t _cur          = BL_FULL;   // 0..255, ara degerler icin 16-bit
  uint16_t _target       = BL_FULL;
};
