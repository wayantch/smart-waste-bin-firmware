#include "BackendClient.h"

#include "AppLog.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include <WiFi.h>

BackendClient::BackendClient(const char *url) : url_(url) {}

bool BackendClient::classify(const String &base64Image, DetectionState &state) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi terputus!");
    app_log::add("WiFi terputus!");
    return false;
  }

  HTTPClient http;
  http.begin(url_);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(20000);

  String body = "{\"image\":\"" + base64Image + "\"}";

  unsigned long start = millis();
  int httpCode = http.POST(body);
  unsigned long latency = millis() - start;

  if (httpCode != 200) {
    Serial.printf("HTTP Error: %d\n", httpCode);
    app_log::add(String("HTTP Error: ") + String(httpCode));
    http.end();
    return false;
  }

  String response = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, response);
  if (err) {
    Serial.println("JSON parse error");
    app_log::add("JSON parse error");
    return false;
  }

  state.label = doc["result"].as<String>();
  state.confidence = doc["confidence"].as<float>();
  state.detections = doc["detections"].as<int>();
  state.latencyMs = doc["latency_ms"].as<int>();
  state.valid = true;
  state.updatedAt = millis();

  Serial.printf("Response (%lums): %s\n", latency, response.c_str());
  Serial.println("\n=============================");
  Serial.printf("Hasil     : %s\n", state.label.c_str());
  Serial.printf("Confidence: %.2f%%\n", state.confidence * 100.0f);
  Serial.printf("Deteksi   : %d objek\n", state.detections);
  Serial.printf("Latency   : %d ms\n", state.latencyMs);
  Serial.println("=============================\n");
  app_log::add(String("Hasil: ") + state.label + " | confidence " + String(state.confidence * 100.0f, 2) + "% | deteksi " + String(state.detections) + " | latency " + String(state.latencyMs) + " ms");

  if (state.label == "plastic") {
    Serial.println("-> PLASTIK TERDETEKSI - trigger motor sorting ke bin plastik");
    app_log::add("-> PLASTIK TERDETEKSI - trigger motor sorting ke bin plastik");
  } else {
    Serial.println("-> NON-PLASTIK - trigger motor sorting ke bin biasa");
    app_log::add("-> NON-PLASTIK - trigger motor sorting ke bin biasa");
  }

  return true;
}