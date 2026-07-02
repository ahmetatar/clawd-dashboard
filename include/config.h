#pragma once
// clawd ana uygulama — tum ayarlanabilir sabitler tek yerde.

// ---- pinler ----
constexpr int PIN_BL   = 21;   // arka isik (LEDC PWM ile suruyoruz)
constexpr int LED_G    = 16;   // RGB yesil (active-low)
constexpr int LED_B    = 17;   // RGB mavi  (active-low)  — kirmizi (GPIO4) bu kartta olu
constexpr int T_CLK    = 25;   // dokunmatik (HSPI)
constexpr int T_CS     = 33;
constexpr int T_MOSI   = 32;
constexpr int T_MISO   = 39;

// ---- ekran / animasyon ----
constexpr int ANIM_W   = 64;   // tum animasyonlar 64x64
constexpr int ANIM_H   = 64;
constexpr int ANIM_S   = 3;    // olcek: 64*3 = 192 px
constexpr int HOLD_MS  = 2500; // gecici ifadeler (happy/oops) sonrasi idle'a donus

// ---- guc yonetimi ----
// idle = son event/dokunmadan bu yana gecen sure.
constexpr uint32_t T_DIM_MS   = 20000;  // 20 sn: arka isigi kis
constexpr uint32_t T_SLEEP_MS = 90000;  // 90 sn: ekrani sondur + uyku

// arka isik parlaklik seviyeleri (8-bit LEDC duty, 0..255)
constexpr uint8_t  BL_FULL = 255;       // aktif: tam parlaklik
constexpr uint8_t  BL_DIM  = 28;        // ~%11: kisik ama okunur/canli
constexpr uint8_t  BL_OFF  = 0;         // uyku: kapali

// LEDC (donanim PWM) — arka isik
constexpr int      BL_CH   = 0;         // LEDC kanali
constexpr int      BL_FREQ = 20000;     // 20 kHz: duyulabilir cizirti olmaz
constexpr int      BL_RES  = 8;         // 8-bit cozunurluk (0..255)

// yumusak fade: her adimda duty bu kadar degisir, RAMP_MS periyotla
constexpr uint8_t  BL_STEP     = 12;
constexpr uint32_t BL_RAMP_MS  = 8;     // ~ (255/12)*8 ≈ 170 ms tam fade

// uyku CPU frekansi (240 -> 80 MHz: WiFi icin min guvenli, ~yari guc)
constexpr int      CPU_HZ_ACTIVE = 240;
constexpr int      CPU_HZ_SLEEP  = 80;
