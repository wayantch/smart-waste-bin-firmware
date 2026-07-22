#pragma once

#include <Arduino.h>

#include "BackendClient.h"
#include "CameraService.h"
#include "Config.h"
#include "DetectionState.h"
#include "WebDashboard.h"

#include <memory>

class WasteBinApp {
 public:
  bool begin();
  void loop();

 private:
  CameraService camera_;
  BackendClient backend_{config::BACKEND_URL};
  DetectionState state_;
  SemaphoreHandle_t stateMutex_ = nullptr;
  std::unique_ptr<WebDashboard> dashboard_;
  unsigned long lastDetectionMs_ = 0;

  void connectWifi();
  void runDetectionCycle();
};