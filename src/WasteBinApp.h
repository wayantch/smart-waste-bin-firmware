#pragma once
#include <Arduino.h>
#include <memory>
#include "BackendClient.h"
#include "BinLevelState.h"
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
  float readUltrasonicDistance();
  float measureDistanceCm(int trigPin, int echoPin);
  void initMotorDriver();
  void initLimitSwitches();
  void startCompressionMotor();
  void stopCompressionMotor();
  void reverseCompressionMotor();
  bool pressUntilLimit();
  bool retractUntilHome();
  bool isAtHomePosition();
  bool isAtPressPosition();
  void initBinLevelSensors();
  void checkBinLevels();

  CameraService    camera_;
  BackendClient    backend_{config::BACKEND_URL};
  DetectionState   state_;
  BinLevelState    binLevelState_;
  SemaphoreHandle_t stateMutex_ = nullptr;
  std::unique_ptr<WebDashboard> dashboard_;
  ServoController  servoPlastic_{config::SERVO_PLASTIC_PIN};
  ServoController  servoNonPlastic_{config::SERVO_NONPLASTIC_PIN};

  unsigned long lastDetectionMs_ = 0;
  unsigned long lastBinLevelCheckMs_ = 0;
};