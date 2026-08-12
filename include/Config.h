#pragma once

#include <Arduino.h>

namespace config {

// constexpr const char *WIFI_SSID = "Made suwarte";
// constexpr const char *WIFI_PASSWORD = "wayan2004";
constexpr const char *WIFI_SSID = "wynnsea";
constexpr const char *WIFI_PASSWORD = "wayan123";
constexpr const char *BACKEND_URL = "http://10.225.97.5:5000/classify";
// constexpr const char *BACKEND_URL = "http://192.168.180.42:5000/classify";

constexpr uint16_t WEB_SERVER_PORT = 80;
constexpr uint16_t STREAM_SERVER_PORT = 81;
constexpr uint32_t CLASSIFICATION_INTERVAL_MS = 5000;

constexpr int PWDN_GPIO_NUM = -1;
constexpr int RESET_GPIO_NUM = -1;
constexpr int XCLK_GPIO_NUM = 15;
constexpr int SIOD_GPIO_NUM = 4;
constexpr int SIOC_GPIO_NUM = 5;
constexpr int Y9_GPIO_NUM = 16;
constexpr int Y8_GPIO_NUM = 17;
constexpr int Y7_GPIO_NUM = 18;
constexpr int Y6_GPIO_NUM = 12;
constexpr int Y5_GPIO_NUM = 10;
constexpr int Y4_GPIO_NUM = 8;
constexpr int Y3_GPIO_NUM = 9;
constexpr int Y2_GPIO_NUM = 11;
constexpr int VSYNC_GPIO_NUM = 6;
constexpr int HREF_GPIO_NUM = 7;
constexpr int PCLK_GPIO_NUM = 13;
// Servo Plastic dan Non-Plastic masing-masing satu pintu bin sendiri
constexpr int SERVO_PLASTIC_PIN = 21;
constexpr int SERVO_NONPLASTIC_PIN = 1;
constexpr int SERVO_CLOSED_DEG = 0;
constexpr int SERVO_OPEN_DEG = 90;
// Servo non-plastik dudukan horn-nya beda arah dari servo plastik, jadi sudut
// buka-nya dipisah supaya bisa dikalibrasi sendiri. Kalau arahnya masih belum
// pas ke kiri, coba ganti nilai ini (misal 45 atau 135) lalu flash ulang.
constexpr int SERVO_NONPLASTIC_OPEN_DEG = 90;
constexpr unsigned long SERVO_OPEN_DURATION_MS = 2000;

constexpr int IR_SENSOR_PIN = 2;
constexpr int ULTRASONIC_TRIG_PIN = 3;
constexpr int ULTRASONIC_ECHO_PIN = 14;
constexpr float ULTRASONIC_THRESHOLD_CM = 10.0f;
constexpr unsigned long ULTRASONIC_TIMEOUT_US = 30000UL;

// Sensor kapasitas bin (deteksi penuh). Pin bebas tinggal 42/47/48, jadi TRIG
// dipakai gantian (jumper) untuk kedua sensor, ECHO tetap terpisah per bin.
constexpr int BIN_LEVEL_TRIG_PIN = 42;
constexpr int BIN_PLASTIC_LEVEL_ECHO_PIN = 47;
constexpr int BIN_NONPLASTIC_LEVEL_ECHO_PIN = 48;
constexpr float BIN_FULL_THRESHOLD_CM = 5.0f;
constexpr unsigned long BIN_LEVEL_POLL_MS = 2000UL;

// Motor kompresi via 2 relay (H-bridge relay, bukan lagi driver L298N).
// Kedua terminal motor ke COM masing-masing relay; NC->GND, NO->VCC.
//   Relay1 ON  & Relay2 OFF -> maju/menekan
//   Relay1 OFF & Relay2 ON  -> mundur/retract
//   Keduanya OFF            -> motor direm (kedua terminal ke GND)
// KEDUA RELAY TIDAK BOLEH ON BERSAMAAN — itu korsleting catu daya ke GND.
constexpr int MOTOR_RELAY1_PIN = 39;
constexpr int MOTOR_RELAY2_PIN = 40;
// Banyak modul relay 2-channel itu aktif LOW (relay nyala saat pin ditarik LOW).
// Kalau modul kamu aktif HIGH, ubah ini jadi false.
constexpr bool MOTOR_RELAY_ACTIVE_LOW = true;
// Jeda wajib tiap ganti arah — pastikan relay arah lama benar-benar lepas
// (kontak mekanis butuh waktu) sebelum relay arah baru dinyalakan.
constexpr unsigned long MOTOR_RELAY_SWITCH_DELAY_MS = 50UL;
constexpr uint32_t MOTOR_PRE_COMPRESSION_DELAY_MS = 2000;
// Timeout pengaman — posisi sebenarnya ditentukan limit switch, bukan durasi.
// Lead screw butuh waktu tempuh lama, jadi diberi ruang 1 menit per arah.
constexpr unsigned long MOTOR_PRESS_TIMEOUT_MS = 120000UL;
constexpr unsigned long MOTOR_RETRACT_TIMEOUT_MS = 120000UL;

// Limit switch NO ke GND + INPUT_PULLUP -> tertekan = LOW, idle = HIGH
constexpr int LIMIT_HOME_PIN = 38;
constexpr int LIMIT_PRESS_PIN = 41;
constexpr unsigned long LIMIT_DEBOUNCE_MS = 25UL;

}  // namespace config