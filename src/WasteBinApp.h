#pragma once
#include <Arduino.h>
#include <memory>
#include "BackendClient.h"
#include "CameraService.h"
#include "DetectionState.h"
#include "ServoController.h"
#include "WebDashboard.h"
#include "Config.h"

class WasteBinApp {
public:
  bool begin();
  void loop();

private:
  void connectWifi();
  void runDetectionCycle();
  void handleActuation(const DetectionState &state);  
  bool isObjectDetected();

  CameraService    camera_;
  BackendClient    backend_{config::BACKEND_URL};
  DetectionState   state_;
  SemaphoreHandle_t stateMutex_ = nullptr;
  std::unique_ptr<WebDashboard> dashboard_;
  ServoController  servo_{config::SERVO_PIN};  

  unsigned long lastDetectionMs_ = 0;
};