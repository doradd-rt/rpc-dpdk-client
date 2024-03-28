#pragma once

#include <iostream>
#include <random>

class FixedGen {
  uint64_t val;

public:
  /**
   * Used for the interarrival distribution.
   * Assume input in ASCII usec and transform to cycles
   */
  FixedGen(char *arg) {
    uint64_t t = atoi(arg);
    uint64_t hz = rte_get_timer_hz();
    val = (hz * t) / 1e6;
  }

  FixedGen(uint64_t v) : val(v) {}

  uint64_t generate() { return val; }
};

class ExpGen {
  std::exponential_distribution<double> dist;
  std::mt19937 gen;

public:
  /**
   * Used for the interarrival distribution.
   * Assume input in ASCII usec and transform to cycles
   */
  ExpGen(char *arg) : gen(std::random_device()()) {
    uint64_t t = atoi(arg);
    uint64_t hz = rte_get_timer_hz();
    uint64_t val = (hz * t) / 1e6;
    dist = std::exponential_distribution<double>(1.0 / val);
  }

  ExpGen(uint64_t val) : gen(std::random_device()()) {
    dist = std::exponential_distribution<double>(1.0 / val);
  }
  uint64_t generate() { return static_cast<uint64_t>(dist(gen)); }
};

#if defined(FIXED)
using RandGen = FixedGen;
#elif defined(EXPONENTIAL)
using RandGen = ExpGen;
#endif
