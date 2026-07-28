#include "CameraService.h"

#include "AppLog.h"
#include "Config.h"

static const char kBase64Table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bool CameraService::begin() {
  if (cameraMutex_ == nullptr) {
    cameraMutex_ = xSemaphoreCreateMutex();
  }

  camera_config_t config{};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = config::Y2_GPIO_NUM;
  config.pin_d1 = config::Y3_GPIO_NUM;
  config.pin_d2 = config::Y4_GPIO_NUM;
  config.pin_d3 = config::Y5_GPIO_NUM;
  config.pin_d4 = config::Y6_GPIO_NUM;
  config.pin_d5 = config::Y7_GPIO_NUM;
  config.pin_d6 = config::Y8_GPIO_NUM;
  config.pin_d7 = config::Y9_GPIO_NUM;
  config.pin_xclk = config::XCLK_GPIO_NUM;
  config.pin_pclk = config::PCLK_GPIO_NUM;
  config.pin_vsync = config::VSYNC_GPIO_NUM;
  config.pin_href = config::HREF_GPIO_NUM;
  config.pin_sccb_sda = config::SIOD_GPIO_NUM;
  config.pin_sccb_scl = config::SIOC_GPIO_NUM;
  config.pin_pwdn = config::PWDN_GPIO_NUM;
  config.pin_reset = config::RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera gagal: 0x%x\n", err);
    app_log::add(String("Kamera gagal: 0x") + String(err, HEX));
    return false;
  }

  Serial.println("Kamera OK!");
  app_log::add("Kamera OK!");
  return true;
}

bool CameraService::capture(camera_fb_t *&frame) {
  if (cameraMutex_ == nullptr) {
    return false;
  }

  if (xSemaphoreTake(cameraMutex_, pdMS_TO_TICKS(2000)) != pdTRUE) {
    Serial.println("Mutex kamera timeout");
    app_log::add("Mutex kamera timeout");
    return false;
  }

  frame = esp_camera_fb_get();
  if (!frame) {
    xSemaphoreGive(cameraMutex_);
    Serial.println("Gagal capture frame");
    app_log::add("Gagal capture frame");
    return false;
  }

  return true;
}

void CameraService::release(camera_fb_t *frame) {
  if (frame) {
    esp_camera_fb_return(frame);
  }

  if (cameraMutex_ != nullptr) {
    xSemaphoreGive(cameraMutex_);
  }
}

String CameraService::encodeBase64(const uint8_t *data, size_t len) {
  String result;
  result.reserve(((len + 2) / 3) * 4 + 1);

  for (size_t index = 0; index < len; index += 3) {
    uint8_t b0 = data[index];
    uint8_t b1 = (index + 1 < len) ? data[index + 1] : 0;
    uint8_t b2 = (index + 2 < len) ? data[index + 2] : 0;

    result += kBase64Table[b0 >> 2];
    result += kBase64Table[((b0 & 0x03) << 4) | (b1 >> 4)];
    result += (index + 1 < len) ? kBase64Table[((b1 & 0x0F) << 2) | (b2 >> 6)] : '=';
    result += (index + 2 < len) ? kBase64Table[b2 & 0x3F] : '=';
  }

  return result;
}

String CameraService::captureBase64() {
  camera_fb_t *frame = nullptr;
  if (!capture(frame)) {
    return "";
  }

  String encoded = encodeBase64(frame->buf, frame->len);
  Serial.printf("Capture OK: %d bytes -> base64 %d chars\n", frame->len, encoded.length());
  app_log::add(String("Capture OK: ") + String(frame->len) + " bytes -> base64 " + String(encoded.length()) + " chars");
  release(frame);
  return encoded;
}