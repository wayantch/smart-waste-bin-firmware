#pragma once

#include <Arduino.h>

#include "DetectionState.h"

class BackendClient {
 public:
  explicit BackendClient(const char *url);

  bool classify(const String &base64Image, DetectionState &state);

 private:
  const char *url_;
};