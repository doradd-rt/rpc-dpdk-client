#pragma once

#include <algorithm>
#include <cmath>
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
  void start_load() { control.should_load = true; }
  void stop_measure() { control.should_measure = false; }
  void start_measure() { control.should_measure = true; }
  bool should_load() { return control.should_load; }
  void new_sample(uint64_t sample) {
    samples[current_idx++ % SAMPLES] = sample;
  }
  uint64_t get_req_count() { return req_count; }
  uint64_t *get_samples() { return samples; }

  // Function to compute the percentile of a vector
  static double percentile(const std::vector<uint64_t> &vec,
                           double percentile) {
    // Make sure the vector is not empty
    if (vec.empty()) {
      return 0.0;
    }

    // Sort the vector
    std::vector<uint64_t> sortedVec = vec;
    std::sort(sortedVec.begin(), sortedVec.end());

    // Calculate the index corresponding to the percentile
    double index = (percentile / 100.0) * (sortedVec.size() - 1);

    // If index is an integer
    if (index == std::floor(index)) {
      return sortedVec[static_cast<int>(index)];
    }

    // If index is a fractional value, interpolate
    int lowerIndex = std::floor(index);
    int upperIndex = std::ceil(index);
    double lowerValue = sortedVec[lowerIndex];
    double upperValue = sortedVec[upperIndex];
    return lowerValue + (index - lowerIndex) * (upperValue - lowerValue);
  }
};
