#include "AppLog.h"

#include <ArduinoJson.h>

namespace {

constexpr size_t kMaxEntries = 24;

String messages[kMaxEntries];
unsigned long timestamps[kMaxEntries];
size_t head = 0;
size_t count = 0;

}  // namespace

namespace app_log {

void add(const String &message) {
  timestamps[head] = millis();
  messages[head] = message;
  head = (head + 1) % kMaxEntries;
  if (count < kMaxEntries) {
    ++count;
  }
}

String snapshotJson() {
  JsonDocument doc;
  JsonArray entries = doc.to<JsonArray>();

  for (size_t index = 0; index < count; ++index) {
    size_t bufferIndex = (head + kMaxEntries - count + index) % kMaxEntries;
    JsonObject entry = entries.add<JsonObject>();
    entry["ts"] = timestamps[bufferIndex];
    entry["message"] = messages[bufferIndex];
  }

  String json;
  serializeJson(doc, json);
  return json;
}

}  // namespace app_log