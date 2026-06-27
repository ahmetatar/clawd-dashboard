// clawd examples — 03 LDR (ışık sensörü)
// CYD dahili LDR'sini GPIO34'ten okur, ham değeri + aydınlık/karanlık sınıfını basar.
// Karanlık algılanınca MAVİ LED yanar = clawd'ın "oda karardı -> uyku modu" göstergesi.
//
// ⚠️ BU KARTTA LDR YOK/ARIZALI: GPIO34 ışığa tepki vermiyor, sabit ~2425 okuyor
//    (yalnızca dahili 1M+1M bölücü kalmış; GT36516 fotosel takılı değil/açık).
//    Bilinen bir sorun. Kod doğru — LDR'si SAĞLAM bir CYD'de bu örnek çalışır.
//    İstersen GPIO35'e harici bir LDR (3.3V–LDR–G35–10k–GND) bağlayıp PIN_LDR=35 yap.

#include <Arduino.h>

constexpr int PIN_LDR = 34;   // sadece-giriş ADC pini (harici LDR için 35 yap)
constexpr int PIN_B   = 17;   // mavi LED (active-low) — uyku göstergesi

// ESP32 ADC 12-bit => 0..4095. Sağlam LDR'de: aydınlık DÜŞÜK, karanlık YÜKSEK değer.
// Önce seri monitörden kendi aydınlık/karanlık değerlerini gör, ortasını buraya yaz.
constexpr int DARK_THRESHOLD = 2500;

static void blue(bool on) { digitalWrite(PIN_B, on ? LOW : HIGH); }  // active-low

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[clawd] 03-ldr basliyor (GPIO34)");
  pinMode(PIN_B, OUTPUT);
  blue(false);
  analogReadResolution(12);
  Serial.printf("[clawd] esik (DARK_THRESHOLD) = %d\n", DARK_THRESHOLD);
}

void loop() {
  long sum = 0;
  const int N = 16;
  for (int i = 0; i < N; i++) { sum += analogRead(PIN_LDR); delay(2); }
  int val = sum / N;

  bool dark = val >= DARK_THRESHOLD;   // ilişki ters çıkarsa: val <= DARK_THRESHOLD
  blue(dark);

  Serial.printf("[clawd] LDR=%4d  ->  %s\n", val, dark ? "KARANLIK (uyku, mavi)" : "aydinlik");
  delay(300);
}
