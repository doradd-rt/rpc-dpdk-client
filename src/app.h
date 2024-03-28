#pragma once

#include <cstring>
#include <iostream>

class LoopRpc {
public:
  LoopRpc(char *arg) { std::cout << "Will create Loop protocol\n"; }

  uint16_t prepare_req(char *payload, uint64_t max_payload_size) {
    memset(payload, 0, max_payload_size);
    *payload = 'x';

    return 1;
  }
};

#if defined(LOOP)
using AppGen = LoopRpc;
#endif
