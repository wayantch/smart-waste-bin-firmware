#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class ServoController {
public:
  ServoController(int pin) : pin_(pin) {}

  void begin() {
    servo_.attach(pin_);
    goToNeutral();
    Serial.println("Servo OK!");
  }

  void goToPlastic() { moveTo(90); }
  void goToNormal()  { moveTo(0);  }
  void goToNeutral() { moveTo(45); }

  void moveTo(int degree) {
    servo_.write(degree);
    Serial.printf("Servo → %d°\n", degree);
    delay(800);
  }

private:
  Servo servo_;
  int pin_;
};