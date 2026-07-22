#pragma once

#include <Arduino.h>

namespace config {

constexpr const char *WIFI_SSID = "Made suwarte";
constexpr const char *WIFI_PASSWORD = "wayan2004";
constexpr const char *BACKEND_URL = "http://192.168.0.103:5000/classify";

constexpr uint16_t WEB_SERVER_PORT = 80;
constexpr uint32_t CLASSIFICATION_INTERVAL_MS = 5000;

constexpr int PWDN_GPIO_NUM = -1;
constexpr int RESET_GPIO_NUM = -1;
constexpr int XCLK_GPIO_NUM = 15;
constexpr int SIOD_GPIO_NUM = 4;
constexpr int SIOC_GPIO_NUM = 5;
constexpr int Y9_GPIO_NUM = 16;
constexpr int Y8_GPIO_NUM = 17;
constexpr int Y7_GPIO_NUM = 18;
constexpr int Y6_GPIO_NUM = 12;
constexpr int Y5_GPIO_NUM = 10;
constexpr int Y4_GPIO_NUM = 8;
constexpr int Y3_GPIO_NUM = 9;
constexpr int Y2_GPIO_NUM = 11;
constexpr int VSYNC_GPIO_NUM = 6;
constexpr int HREF_GPIO_NUM = 7;
constexpr int PCLK_GPIO_NUM = 13;

}  // namespace config