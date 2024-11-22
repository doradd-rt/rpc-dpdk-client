#pragma once

#include <cstring>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <array>

extern "C" {
#include <rte_memcpy.h>
}

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

  // dbg only
  int rnd = 0;

  using Rand = foedus::assorted::ZipfianRandom;
  Rand rand;
  double zipf_s{0};
  bool contention{false};

  struct __attribute__((packed)) Marshalled
  {
    uint32_t indices[ROWS_PER_TX];
    uint16_t spin_usec;
    uint64_t cown_ptrs[ROWS_PER_TX];
    uint8_t pad[6];
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

  uint16_t spin_usec;
  uint32_t replay_log_size;
  uint32_t read_cnt;
  char* replay_log;

public:
  YCSBRpc(char *arg, uint32_t replay_log_size, char* replay_log, uint16_t spin_usec) : 
              replay_log_size(replay_log_size), replay_log(replay_log), spin_usec(spin_usec) {
    rand.init(ROW_COUNT, zipf_s, 1238);
    std::cout << "Will create ycsb workloads\n"; 

    read_cnt = 0;
  }

  uint16_t prepare_req(char *payload, uint64_t max_payload_size) {
    memset(payload, 0, max_payload_size);

    Marshalled* txm = reinterpret_cast<Marshalled*>(
      replay_log + 
      sizeof(Marshalled) * (read_cnt++ % replay_log_size));

    txm->spin_usec = spin_usec;
 
    // Create an instance of Marshalled
    /* Marshalled data; */

    /* static int rnd = 0; */
    // Populate the indices
    /* auto keys = gen_keys(&rand, contention); */
    /* for (size_t i = 0; i < ROWS_PER_TX; ++i) { */
    /*   printf("txm->indices %u\n", txm->indices[i]); */
      /* data.indices[i] = (i + rnd * ROWS_PER_TX) % ROW_COUNT;//keys[i]; */
      /* data.indices[i] = keys[i]; */
    /* } */

    /* rnd++; */
    // Set the write_set
    /* data.write_set = 0x3;//gen_write_set(contention); */

    // Copy data into the payload buffer
    rte_memcpy(payload, txm, sizeof(Marshalled));
    return sizeof(Marshalled);
  }
};

#if defined(LOOP)
using AppGen = LoopRpc;
#elif defined(YCSB)
using AppGen = YCSBRpc;
#endif
