#include "WasteBinApp.h"
#include <WiFi.h>
#include "AppLog.h"

bool WasteBinApp::begin() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== Smart Waste Bin — Full System ===");
  app_log::add("=== Smart Waste Bin — Full System ===");

  stateMutex_ = xSemaphoreCreateMutex();
  if (stateMutex_ == nullptr) {
    Serial.println("Gagal membuat mutex state");
    return false;
  }

  // Init servo — masing-masing bin (plastik & non-plastik) punya servo sendiri
  servoPlastic_.begin();
  servoNonPlastic_.begin();

  initMotorDriver();
  initLimitSwitches();

  // Cek posisi awal press plate — TIDAK menggerakkan motor otomatis saat boot,
  // motor kompresi cuma boleh jalan lewat alur deteksi+konfirmasi botol yang normal.
  if (!isAtHomePosition()) {
    Serial.println("PERINGATAN: posisi awal press plate bukan HOME — cek mekanisme secara manual sebelum dipakai");
    app_log::add("PERINGATAN: posisi awal press plate bukan HOME — cek mekanisme secara manual sebelum dipakai");
  } else {
    Serial.println("Press plate sudah di posisi HOME");
    app_log::add("Press plate sudah di posisi HOME");
  }

  // Init sensor IR
  pinMode(config::IR_SENSOR_PIN, INPUT);
  Serial.println("Sensor IR OK!");
  app_log::add("Sensor IR OK!");

  // Init sensor ultrasonic HC-SR04 (konfirmasi botol masuk bin plastik)
  pinMode(config::ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(config::ULTRASONIC_ECHO_PIN, INPUT);
  digitalWrite(config::ULTRASONIC_TRIG_PIN, LOW);
  Serial.println("Sensor ultrasonic HC-SR04 OK!");
  app_log::add("Sensor ultrasonic HC-SR04 OK!");

  initBinLevelSensors();

  if (!camera_.begin()) return false;

  dashboard_.reset(new WebDashboard(camera_, state_, binLevelState_, stateMutex_));

  connectWifi();
  dashboard_->begin();

  Serial.println("Sistem siap — menunggu objek...\n");
  app_log::add("Sistem siap — menunggu objek...");
  return true;
}

void WasteBinApp::connectWifi() {
  Serial.printf("Konek WiFi: %s\n", config::WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("\nWiFi OK! IP: %s\n", WiFi.localIP().toString().c_str());
  app_log::add(String("WiFi OK! IP: ") + WiFi.localIP().toString().c_str());
}

void WasteBinApp::initMotorDriver() {
  pinMode(config::MOTOR_IN1_PIN, OUTPUT);
  pinMode(config::MOTOR_IN2_PIN, OUTPUT);

  stopCompressionMotor();

  Serial.println("Motor driver L298N OK!");
  app_log::add("Motor driver L298N OK!");
}

void WasteBinApp::initLimitSwitches() {
  pinMode(config::LIMIT_HOME_PIN, INPUT_PULLUP);
  pinMode(config::LIMIT_PRESS_PIN, INPUT_PULLUP);
  Serial.println("Limit switch OK!");
  app_log::add("Limit switch OK!");
}

void WasteBinApp::startCompressionMotor() {
  digitalWrite(config::MOTOR_IN1_PIN, HIGH);
  digitalWrite(config::MOTOR_IN2_PIN, LOW);
  Serial.println("Motor kompresi jalan");
  app_log::add("Motor kompresi jalan");
}

void WasteBinApp::stopCompressionMotor() {
  digitalWrite(config::MOTOR_IN1_PIN, LOW);
  digitalWrite(config::MOTOR_IN2_PIN, LOW);
  Serial.println("Motor kompresi berhenti");
  app_log::add("Motor kompresi berhenti");
}

void WasteBinApp::reverseCompressionMotor() {
  digitalWrite(config::MOTOR_IN1_PIN, LOW);
  digitalWrite(config::MOTOR_IN2_PIN, HIGH);
  Serial.println("Motor mundur (retract)");
  app_log::add("Motor mundur (retract)");
}

bool WasteBinApp::isAtHomePosition() {
  // Limit switch NO ke GND + INPUT_PULLUP -> tersentuh = LOW
  return digitalRead(config::LIMIT_HOME_PIN) == LOW;
}

bool WasteBinApp::isAtPressPosition() {
  return digitalRead(config::LIMIT_PRESS_PIN) == LOW;
}

bool WasteBinApp::pressUntilLimit() {
  bool pressAlreadyLow = digitalRead(config::LIMIT_PRESS_PIN) == LOW;
  Serial.printf("Cek awal limit PRESS sebelum motor jalan: %s\n", pressAlreadyLow ? "LOW (sudah tertekan!)" : "HIGH (idle, normal)");
  if (pressAlreadyLow) {
    Serial.println("PERINGATAN: limit PRESS sudah LOW sebelum motor bergerak — cek wiring NO/NC atau mekanisme tersangkut");
    app_log::add("PERINGATAN: limit PRESS sudah LOW sebelum motor bergerak — cek wiring NO/NC atau mekanisme tersangkut");
  }

  Serial.println("Motor kompresi maju — menekan sampai limit PRESS...");
  app_log::add("Motor kompresi maju — menekan sampai limit PRESS...");

  startCompressionMotor();
  unsigned long start = millis();
  bool reached = false;
  while (millis() - start < config::MOTOR_PRESS_TIMEOUT_MS) {
    if (digitalRead(config::LIMIT_PRESS_PIN) == LOW) {
      delay(config::LIMIT_DEBOUNCE_MS);  // debounce
      if (digitalRead(config::LIMIT_PRESS_PIN) == LOW) {
        reached = true;
        break;
      }
      continue;
    }
    delay(config::LIMIT_DEBOUNCE_MS);
  }
  stopCompressionMotor();

  if (reached) {
    Serial.println("Limit switch PRESS tercapai — penekanan selesai");
    app_log::add("Limit switch PRESS tercapai — penekanan selesai");
  } else {
    Serial.println("TIMEOUT menekan — limit switch PRESS tidak tersentuh!");
    app_log::add("ERROR: TIMEOUT menekan — limit switch PRESS tidak tersentuh, cek mekanisme");
  }
  return reached;
}

bool WasteBinApp::retractUntilHome() {
  bool homeAlreadyLow = digitalRead(config::LIMIT_HOME_PIN) == LOW;
  Serial.printf("Cek awal limit HOME sebelum motor jalan: %s\n", homeAlreadyLow ? "LOW (sudah tertekan!)" : "HIGH (idle, normal)");
  if (homeAlreadyLow) {
    Serial.println("Catatan: limit HOME sudah LOW sebelum motor bergerak — wajar kalau memang posisi awal di HOME");
  }

  Serial.println("Motor kompresi mundur — retract sampai limit HOME...");
  app_log::add("Motor kompresi mundur — retract sampai limit HOME...");

  reverseCompressionMotor();
  unsigned long start = millis();
  bool reached = false;
  while (millis() - start < config::MOTOR_RETRACT_TIMEOUT_MS) {
    if (digitalRead(config::LIMIT_HOME_PIN) == LOW) {
      delay(config::LIMIT_DEBOUNCE_MS);  // debounce
      if (digitalRead(config::LIMIT_HOME_PIN) == LOW) {
        reached = true;
        break;
      }
      continue;
    }
    delay(config::LIMIT_DEBOUNCE_MS);
  }
  stopCompressionMotor();

  if (reached) {
    Serial.println("Limit switch HOME tercapai — retract selesai");
    app_log::add("Limit switch HOME tercapai — retract selesai");
  } else {
    Serial.println("TIMEOUT retract — limit switch HOME tidak tersentuh!");
    app_log::add("ERROR: TIMEOUT retract — limit switch HOME tidak tersentuh, cek mekanisme");
  }
  return reached;
}

bool WasteBinApp::isObjectDetected() {
  // Sensor IR aktif LOW — LOW = ada objek, HIGH = kosong
  return digitalRead(config::IR_SENSOR_PIN) == LOW;
}

float WasteBinApp::measureDistanceCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long durationUs = pulseIn(echoPin, HIGH, config::ULTRASONIC_TIMEOUT_US);
  if (durationUs == 0) {
    return -1.0f;
  }

  return static_cast<float>(durationUs) / 58.0f;
}

float WasteBinApp::readUltrasonicDistance() {
  return measureDistanceCm(config::ULTRASONIC_TRIG_PIN, config::ULTRASONIC_ECHO_PIN);
}

void WasteBinApp::initBinLevelSensors() {
  // TRIG dipakai gantian (jumper) untuk sensor bin plastik & non-plastik
  pinMode(config::BIN_LEVEL_TRIG_PIN, OUTPUT);
  digitalWrite(config::BIN_LEVEL_TRIG_PIN, LOW);
  pinMode(config::BIN_PLASTIC_LEVEL_ECHO_PIN, INPUT);
  pinMode(config::BIN_NONPLASTIC_LEVEL_ECHO_PIN, INPUT);
  Serial.println("Sensor level bin OK!");
  app_log::add("Sensor level bin OK!");
}

void WasteBinApp::checkBinLevels() {
  // Dua siklus trigger+baca terpisah (bukan simultan) supaya echo dua sensor tidak tumpang tindih
  float plasticCm = measureDistanceCm(config::BIN_LEVEL_TRIG_PIN, config::BIN_PLASTIC_LEVEL_ECHO_PIN);
  float nonPlasticCm = measureDistanceCm(config::BIN_LEVEL_TRIG_PIN, config::BIN_NONPLASTIC_LEVEL_ECHO_PIN);

  bool plasticFull = (plasticCm > 0.0f && plasticCm < config::BIN_FULL_THRESHOLD_CM);
  bool nonPlasticFull = (nonPlasticCm > 0.0f && nonPlasticCm < config::BIN_FULL_THRESHOLD_CM);

  bool plasticChanged = false;
  bool nonPlasticChanged = false;

  if (stateMutex_ != nullptr && xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(200)) == pdTRUE) {
    plasticChanged = (plasticFull != binLevelState_.plasticFull);
    nonPlasticChanged = (nonPlasticFull != binLevelState_.nonPlasticFull);

    binLevelState_.plasticDistanceCm = plasticCm;
    binLevelState_.nonPlasticDistanceCm = nonPlasticCm;
    binLevelState_.plasticFull = plasticFull;
    binLevelState_.nonPlasticFull = nonPlasticFull;
    xSemaphoreGive(stateMutex_);
  }

  // Log cuma pas ada perubahan status, biar tidak spam tiap 2 detik
  if (plasticChanged) {
    if (plasticFull) {
      Serial.println("PERINGATAN: Bin plastik PENUH!");
      app_log::add("PERINGATAN: Bin plastik PENUH!");
    } else {
      Serial.println("Bin plastik kembali normal (tidak penuh)");
      app_log::add("Bin plastik kembali normal (tidak penuh)");
    }
  }

  if (nonPlasticChanged) {
    if (nonPlasticFull) {
      Serial.println("PERINGATAN: Bin non-plastik PENUH!");
      app_log::add("PERINGATAN: Bin non-plastik PENUH!");
    } else {
      Serial.println("Bin non-plastik kembali normal (tidak penuh)");
      app_log::add("Bin non-plastik kembali normal (tidak penuh)");
    }
  }
}

void WasteBinApp::runDetectionCycle() {
  app_log::add("Mulai klasifikasi");

  // Tunggu objek stabil sebentar sebelum capture
  delay(300);

  String base64Image = camera_.captureBase64();
  if (base64Image.isEmpty()) {
    app_log::add("Klasifikasi dibatalkan: frame kosong");
    return;
  }

  DetectionState newState;
  if (!backend_.classify(base64Image, newState)) {
    app_log::add("Klasifikasi gagal");
    return;
  }

  // Aktuasi servo
  handleActuation(newState);

  if (stateMutex_ != nullptr &&
      xSemaphoreTake(stateMutex_, pdMS_TO_TICKS(200)) == pdTRUE) {
    state_ = newState;
    xSemaphoreGive(stateMutex_);
  }
}

void WasteBinApp::handleActuation(const DetectionState &state) {
  if (!state.valid) return;

  if (state.label == "plastic") {
    Serial.println("→ PLASTIK — buka servo bin plastik");
    app_log::add("→ PLASTIK — buka servo bin plastik");
    servoPlastic_.open();

    // Bin tetap terbuka selama menunggu konfirmasi botol masuk
    unsigned long startWait = millis();
    bool bottleConfirmed = false;
    while (millis() - startWait < 5000UL) {
      float jarak = readUltrasonicDistance();
      if (jarak > 0.0f && jarak < config::ULTRASONIC_THRESHOLD_CM) {
        bottleConfirmed = true;
        break;
      }
      delay(100);
    }

    // Tutup servo plastik dulu — jangan biarkan bin terbuka saat motor menekan
    servoPlastic_.close();

    if (bottleConfirmed) {
      app_log::add("Botol terkonfirmasi masuk bin plastik — siap kompresi");
      delay(config::MOTOR_PRE_COMPRESSION_DELAY_MS);

      bool pressOk = pressUntilLimit();
      delay(500);  // jeda singkat sebelum retract
      bool retractOk = retractUntilHome();

      if (!pressOk || !retractOk) {
        app_log::add("PERINGATAN: siklus kompresi tidak selesai normal — cek mekanisme");
      }
    } else {
      app_log::add("Timeout — botol tidak terdeteksi di bin plastik");
    }

  } else {
    Serial.println("→ NON-PLASTIK — buka servo bin non-plastik");
    app_log::add("→ NON-PLASTIK — buka servo bin non-plastik");
    servoNonPlastic_.open();
    delay(config::SERVO_OPEN_DURATION_MS);
    servoNonPlastic_.close();
  }
}

void WasteBinApp::loop() {
  // Cek level bin secara periodik, tidak tergantung ada objek atau tidak
  unsigned long now = millis();
  if (now - lastBinLevelCheckMs_ >= config::BIN_LEVEL_POLL_MS) {
    lastBinLevelCheckMs_ = now;
    checkBinLevels();
  }

  // Cek sensor IR dulu — kalau ga ada objek, skip
  if (!isObjectDetected()) {
    delay(100);  // polling tiap 100ms
    return;
  }

  Serial.println("--- Objek terdeteksi! Memulai klasifikasi ---");
  app_log::add("--- Objek terdeteksi! ---");
  runDetectionCycle();

  // Tunggu objek diangkat sebelum bisa deteksi lagi
  Serial.println("Menunggu area bersih...");
  while (isObjectDetected()) {
    delay(200);
  }
  delay(500);  // debounce
  Serial.println("Area bersih — sistem siap\n");
  app_log::add("Area bersih — sistem siap");
}