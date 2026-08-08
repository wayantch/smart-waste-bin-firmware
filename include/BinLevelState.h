#pragma once

// Status kapasitas bin dari sensor ultrasonik. Dipisah dari DetectionState
// supaya tidak ikut ter-reset tiap ada siklus klasifikasi baru.
struct BinLevelState {
  bool plasticFull = false;
  bool nonPlasticFull = false;
  float plasticDistanceCm = -1.0f;
  float nonPlasticDistanceCm = -1.0f;
};
