#pragma once

#include <iostream>

#include "base.h"
#include "rand.h"

class Worker {
  RandGen *r;
  uint32_t queue_id;
  // App Gen
  // Location of results?

public:
  Worker(RandGen *r_, uint32_t q) : r(r_), queue_id(q) {}

  void do_work() {
    while (!force_quit) {
      /* Process Responses */
      dpdk_poll();
      break;
    }
  }

  uint32_t get_queue_id() { return queue_id; }
};
