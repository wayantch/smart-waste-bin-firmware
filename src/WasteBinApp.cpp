#include "WasteBinApp.h"

#include <WiFi.h>

bool WasteBinApp::begin() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== Smart Waste Bin — Clean Architecture ===");

  stateMutex_ = xSemaphoreCreateMutex();
  if (stateMutex_ == nullptr) {
    Serial.println("Gagal membuat mutex state");
    return false;
  }

  if (!camera_.begin()) {
    return false;
  }

  dashboard_.reset(new WebDashboard(camera_, state_, stateMutex_));

  connectWifi();
  dashboard_->begin();

  Serial.println("Sistem siap — klasifikasi otomatis dan live stream aktif\n");
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
}

void WasteBinApp::runDetectionCycle() {
  String base64Image = camera_.captureBase64();
  if (base64Image.isEmpty()) {
    return;
  }

  DetectionState newState;
  if (!backend_.classify(base64Image, newState)) {
    return;
  }

  if (stateMutex_ != nullptr && xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(200)) == pdTRUE) {
    state_ = newState;
    xSemaphoreGive(stateMutex_);
  }
}

void WasteBinApp::loop() {
  if (dashboard_ && dashboard_->isStreamingActive()) {
    delay(20);
    return;
  }

  unsigned long now = millis();
  if (now - lastDetectionMs_ >= config::CLASSIFICATION_INTERVAL_MS) {
    lastDetectionMs_ = now;
    Serial.println("--- Memulai klasifikasi ---");
    runDetectionCycle();
  }

  delay(5);
}