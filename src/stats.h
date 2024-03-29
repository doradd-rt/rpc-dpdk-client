#pragma once

#include <cstdint>
#include <cstring>

const uint32_t SAMPLES = 4096;

struct StatsControl {
  bool should_load;
  bool should_measure;

  StatsControl() : should_load(true), should_measure(false) {}
};

struct Stats {
  StatsControl control;
  uint64_t req_count;
  uint64_t current_idx;
  uint64_t samples[SAMPLES];

  Stats() {
    memset(samples, 0, SAMPLES * 8);
    req_count = 0;
    current_idx = 0;
  };

public:
  void stop_load() { control.should_load = false; }

  bool should_load() { return control.should_load; }
};
