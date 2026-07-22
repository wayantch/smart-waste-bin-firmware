#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "CameraService.h"
#include "DetectionState.h"

class WebDashboard {
 public:
  WebDashboard(CameraService &camera, DetectionState &state, SemaphoreHandle_t stateMutex);

  void begin();
  void handleClient();
  bool isStreamingActive() const;

 private:
  CameraService &camera_;
  DetectionState &state_;
  SemaphoreHandle_t stateMutex_;
  WebServer server_;
  volatile uint32_t activeStreamClients_ = 0;

  void startTask();
  static void taskEntry(void *parameter);
  void taskLoop();

  void handleRoot();
  void handleStatus();
  void handleStream();
  String buildStatusJson();
  String buildHtmlPage() const;
};