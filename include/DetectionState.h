#pragma once

#include <Arduino.h>

struct DetectionState {
  String label = "waiting";
  float confidence = 0.0f;
  int detections = 0;
  int latencyMs = 0;
  bool valid = false;
  unsigned long updatedAt = 0;
};