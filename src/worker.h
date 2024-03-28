#pragma once

#include <iostream>

#include "app.h"
#include "base.h"
#include "rand.h"

class Worker {
  RandGen *r;
  AppGen *a;
  uint32_t queue_id;
  // Location of results?

public:
  Worker(RandGen *r_, AppGen *a_, uint32_t q) : r(r_), a(a_), queue_id(q) {}

  void do_work() {
    while (!force_quit) {
      /* Process Responses */
      dpdk_poll();
      break;
    }
  }

  uint32_t get_queue_id() { return queue_id; }
};
