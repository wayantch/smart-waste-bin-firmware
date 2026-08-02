#pragma once

#include <Arduino.h>
#include "esp_camera.h"
#include <vector>

class CameraService {
 public:
  bool begin();
  bool lockCamera(uint32_t timeoutMs = 500);
  void unlockCamera();
  bool captureJpeg(std::vector<uint8_t> &jpegData, bool flushStale = false);
  bool capture(camera_fb_t *&frame);
  void release(camera_fb_t *frame);
  String captureBase64();

 private:
  SemaphoreHandle_t cameraMutex_ = nullptr;
  String encodeBase64(const uint8_t *data, size_t len);
};