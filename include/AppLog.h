#pragma once

#include <Arduino.h>

namespace app_log {

void add(const String &message);
String snapshotJson();

}  // namespace app_log