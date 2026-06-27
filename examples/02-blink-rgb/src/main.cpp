// clawd examples — 02 Blink RGB
// CYD dahili RGB LED'ini tek tek ve karışık renklerle yakar. Harici donanım yok.
// Amaç: doğru pinleri ve active-low davranışını kendi gözünle doğrulamak.
// Bu, clawd'ın "duygu ışığı"nın (thinking=nefes, ok=yeşil, hata=kırmızı) temeli.
//
// NOT (bu kart): kırmızı kanal (GPIO4) bu ünitede yanmıyor — alt-LED arızalı görünüyor.
// Yeşil (16) ve mavi (17) sağlam. Teşhis için README'ye bak.

#include <Arduino.h>

// CYD dahili RGB LED pinleri (active-low)
constexpr int PIN_R = 4;
constexpr int PIN_G = 16;
constexpr int PIN_B = 17;

// active-low: true => yanar (pine LOW yazariz)
static void setRGB(bool r, bool g, bool b) {
  digitalWrite(PIN_R, r ? LOW : HIGH);
  digitalWrite(PIN_G, g ? LOW : HIGH);
  digitalWrite(PIN_B, b ? LOW : HIGH);
}

static void show(const char *name, bool r, bool g, bool b, int ms) {
  Serial.printf("[clawd] LED -> %s\n", name);
  setRGB(r, g, b);
  delay(ms);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[clawd] 02-blink-rgb basliyor (dahili RGB LED)");

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  setRGB(false, false, false);  // hepsi sonuk
}

void loop() {
  // 1) Tek renkler — hangi pin hangi renk, dogrula
  show("KIRMIZI", true,  false, false, 600);
  show("YESIL",   false, true,  false, 600);
  show("MAVI",    false, false, true,  600);

  // 2) Karisimlar — uc kanal da calisiyor mu
  show("SARI (R+G)",  true,  true,  false, 600);
  show("CAMGOBEGI (G+B)", false, true, true, 600);
  show("MOR (R+B)",   true,  false, true,  600);
  show("BEYAZ (R+G+B)", true, true,  true,  600);

  // 3) Kisa kapanis — donguyu ayirt et
  show("KAPALI", false, false, false, 800);
}
