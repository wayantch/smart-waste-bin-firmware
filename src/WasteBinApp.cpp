#include "WasteBinApp.h"
#include <WiFi.h>
#include "AppLog.h"

bool WasteBinApp::begin() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== Smart Waste Bin — Full System ===");
  app_log::add("=== Smart Waste Bin — Full System ===");

  stateMutex_ = xSemaphoreCreateMutex();
  if (stateMutex_ == nullptr) {
    Serial.println("Gagal membuat mutex state");
    return false;
  }

  // Init servo
  servo_.begin();

  // Init sensor IR
  pinMode(config::IR_SENSOR_PIN, INPUT);
  Serial.println("Sensor IR OK!");
  app_log::add("Sensor IR OK!");

  if (!camera_.begin()) return false;

  dashboard_.reset(new WebDashboard(camera_, state_, stateMutex_));

  connectWifi();
  dashboard_->begin();

  Serial.println("Sistem siap — menunggu objek...\n");
  app_log::add("Sistem siap — menunggu objek...");
  return true;
}

void WasteBinApp::connectWifi() {
  Serial.printf("Konek WiFi: %s\n", config::WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("\nWiFi OK! IP: %s\n", WiFi.localIP().toString().c_str());
  app_log::add(String("WiFi OK! IP: ") + WiFi.localIP().toString().c_str());
}

bool WasteBinApp::isObjectDetected() {
  // Sensor IR aktif LOW — LOW = ada objek, HIGH = kosong
  // Kalau sensor lo aktif HIGH, ganti LOW → HIGH di bawah
  return digitalRead(config::IR_SENSOR_PIN) == LOW;
}

void WasteBinApp::runDetectionCycle() {
  app_log::add("Mulai klasifikasi");

  // Tunggu objek stabil sebentar sebelum capture
  delay(300);

  String base64Image = camera_.captureBase64();
  if (base64Image.isEmpty()) {
    app_log::add("Klasifikasi dibatalkan: frame kosong");
    return;
  }

  DetectionState newState;
  if (!backend_.classify(base64Image, newState)) {
    app_log::add("Klasifikasi gagal");
    return;
  }

  // Aktuasi servo
  handleActuation(newState);

  if (stateMutex_ != nullptr &&
      xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(200)) == pdTRUE) {
    state_ = newState;
    xSemaphoreGive(stateMutex_);
  }
}

void WasteBinApp::handleActuation(const DetectionState &state) {
  if (!state.valid) return;

  if (state.label == "plastic") {
    Serial.println("→ PLASTIK — buka bin plastik");
    app_log::add("→ PLASTIK — buka bin plastik");
    servo_.goToPlastic();
    delay(3000);  // jeda 3 detik biar sampah masuk
    servo_.goToNeutral();

  } else {
    Serial.println("→ NON-PLASTIK — buka bin biasa");
    app_log::add("→ NON-PLASTIK — buka bin biasa");
    servo_.goToNormal();
    delay(3000);  // jeda 3 detik biar sampah masuk
    servo_.goToNeutral();
  }
}

void WasteBinApp::loop() {
  // Cek sensor IR dulu — kalau ga ada objek, skip
  if (!isObjectDetected()) {
    delay(100);  // polling tiap 100ms
    return;
  }

  Serial.println("--- Objek terdeteksi! Memulai klasifikasi ---");
  app_log::add("--- Objek terdeteksi! ---");
  runDetectionCycle();

  // Tunggu objek diangkat sebelum bisa deteksi lagi
  Serial.println("Menunggu area bersih...");
  while (isObjectDetected()) {
    delay(200);
  }
  delay(500);  // debounce
  Serial.println("Area bersih — sistem siap\n");
  app_log::add("Area bersih — sistem siap");
}