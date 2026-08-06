#pragma once

#include <Arduino.h>

namespace config {

constexpr const char *WIFI_SSID = "Ayumi";
constexpr const char *WIFI_PASSWORD = "2019ayumi";
// constexpr const char *WIFI_SSID = "wynnsea";
// constexpr const char *WIFI_PASSWORD = "wayan123";
// constexpr const char *BACKEND_URL = "http://192.168.0.102:5000/classify";
constexpr const char *BACKEND_URL = "http://192.168.180.42:5000/classify";

constexpr uint16_t WEB_SERVER_PORT = 80;
constexpr uint16_t STREAM_SERVER_PORT = 81;
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
constexpr int SERVO_PIN = 21;
constexpr int IR_SENSOR_PIN = 2;
constexpr int ULTRASONIC_TRIG_PIN = 3;
constexpr int ULTRASONIC_ECHO_PIN = 14;
constexpr float ULTRASONIC_THRESHOLD_CM = 15.0f;
constexpr unsigned long ULTRASONIC_TIMEOUT_US = 30000UL;
constexpr int MOTOR_IN1_PIN = 39;
constexpr int MOTOR_IN2_PIN = 40;
constexpr int MOTOR_IN3_PIN = 41;
constexpr int MOTOR_IN4_PIN = 42;
constexpr uint32_t MOTOR_PRE_COMPRESSION_DELAY_MS = 2000;
constexpr uint32_t MOTOR_RUN_MS = 30000;

}  // namespace config