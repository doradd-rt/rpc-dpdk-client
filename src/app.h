#pragma once

#include <cstring>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <array>

#include "random_gen/zipfian_random.h"

class LoopRpc {
public:
  LoopRpc(char *arg) { std::cout << "Will create Loop protocol\n"; }

  uint16_t prepare_req(char *payload, uint64_t max_payload_size) {
    memset(payload, 0, max_payload_size);
    *payload = 'x';

    return 1;
  }
};

class YCSBRpc {
  static const size_t ROWS_PER_TX = 10;
  static const size_t ROW_COUNT = 10'000'000;
  static const int NrMSBContentionKey = 6;

  using Rand = foedus::assorted::ZipfianRandom;
  Rand rand;
  double zipf_s{0};
  bool contention{false};

  struct __attribute__((packed)) Marshalled
  {
    uint32_t indices[ROWS_PER_TX];
    uint16_t write_set;
  };

  std::array<uint32_t, ROWS_PER_TX> gen_keys(Rand* r, int contention)
  {
    int nr_lsb = 63 - __builtin_clzll(ROW_COUNT) - NrMSBContentionKey;
    size_t mask = 0;
    if (nr_lsb > 0) mask = (1 << nr_lsb) - 1;
  
    int NrContKey = 0;
    if (contention) NrContKey = 3;
  
    std::array<uint32_t, ROWS_PER_TX> keys;
    for (int i = 0; i < ROWS_PER_TX; i++) {
   again:
      keys[i] = r->next() % ROW_COUNT;
      // if branch below skip the idx-0, i.e., the most contended key
      // since "& mask" will opt it out, thus keys are [1, 10M-1] 
      // To stay consistent w/ Caracal, we keep these here.
      if (i < NrContKey) {
        keys[i] &= ~mask;
      } else {
        if ((keys[i] & mask) == 0)
          goto again;
      }
      for (int j = 0; j < i; j++)
        if (keys[i] == keys[j])
          goto again;
    }
  
    return keys;
  }

  uint16_t gen_write_set(int contention) 
  {
    // Set 0:10 read-write ratio for contented workloads
    /* if (contention) */
    /*   return static_cast<uint16_t>((1 << 10) - 1); */

    std::vector<uint8_t> binDigits = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1};
    uint16_t result = 0;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(binDigits.begin(), binDigits.end(), g);
  
    for (uint8_t digit : binDigits)
      result = (result << 1) | digit;
  
    return result;
  }

public:
  YCSBRpc(char *arg) {
    rand.init(ROW_COUNT, zipf_s, 1238);
    std::cout << "Will create ycsb workloads\n"; 
  }

  uint16_t prepare_req(char *payload, uint64_t max_payload_size) {
    memset(payload, 0, max_payload_size);

    // Create an instance of Marshalled
    Marshalled data;

    // Populate the indices
    auto keys = gen_keys(&rand, contention);
    for (size_t i = 0; i < ROWS_PER_TX; ++i) {
      data.indices[i] = keys[i];
    }

    // Set the write_set
    data.write_set = gen_write_set(contention);

    // Copy data into the payload buffer
    memcpy(payload, &data, sizeof(Marshalled));
    return sizeof(Marshalled);
  }
};

#if defined(LOOP)
using AppGen = LoopRpc;
#elif defined(YCSB)
using AppGen = YCSBRpc;
#endif
