#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "CameraService.h"
#include "DetectionState.h"

class WebDashboard {
 public:
  WebDashboard(CameraService &camera, DetectionState &state, SemaphoreHandle_t stateMutex);

  void begin();
  void handleControlClient();
  void handleStreamClient();
  bool isStreamingActive() const;

 private:
  struct StreamSessionContext {
    WebDashboard *dashboard;
    WiFiClient client;
  };

  CameraService &camera_;
  DetectionState &state_;
  SemaphoreHandle_t stateMutex_;
  WebServer controlServer_;
  WebServer streamServer_;
  volatile uint32_t activeStreamClients_ = 0;

  void startTasks();
  static void controlTaskEntry(void *parameter);
  static void streamTaskEntry(void *parameter);
  static void streamClientTaskEntry(void *parameter);
  void controlTaskLoop();
  void streamTaskLoop();
  void streamClientLoop(StreamSessionContext *session);

  void handleRoot();
  void handleStatus();
  void handleLogs();
  void handleStream();
  String buildStatusJson();
  String buildLogsJson();
  String buildHtmlPage() const;
};