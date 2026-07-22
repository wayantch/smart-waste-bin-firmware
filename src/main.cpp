#include <Arduino.h>

#include "WasteBinApp.h"

WasteBinApp app;

void setup() {
  app.begin();
}

void loop() {
  app.loop();
}