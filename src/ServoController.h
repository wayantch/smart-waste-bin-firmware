#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

#include "Config.h"

class ServoController {
public:
  ServoController(int pin) : pin_(pin) {}

  void begin() {
    // Kunci LEDC ke timer 0 supaya kedua servo berbagi timer yang sama dan
    // tidak rebutan/bentrok dengan LEDC_TIMER_1 yang dipakai esp_camera untuk XCLK.
    if (!timerReserved_) {
      ESP32PWM::allocateTimer(0);
      timerReserved_ = true;
    }
    servo_.attach(pin_);
    close();
    Serial.println("Servo OK!");
  }

  void open() { moveTo(config::SERVO_OPEN_DEG); }
  void close() { moveTo(config::SERVO_CLOSED_DEG); }

  void moveTo(int degree) {
    servo_.write(degree);
    Serial.printf("Servo (pin %d) -> %d derajat\n", pin_, degree);
    delay(800);
  }

private:
  static bool timerReserved_;
  Servo servo_;
  int pin_;
};