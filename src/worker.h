#pragma once

#include <iostream>

#include "app.h"
#include "base.h"
#include "rand.h"

extern "C" {
#include <rte_cycles.h>
}

class Worker {
  RandGen *r;
  AppGen *a;
  uint32_t queue_id;
  // Location of results?

  rte_mbuf *prepare_req() {
    std::cout << "Will prepare a request\n";
    return nullptr;
  }

  inline uint64_t get_cyclces_now() {
    // Use the DPDK one for generality
    return rte_get_timer_cycles();
  }

  inline void wait_till(uint64_t deadline) {
    while (get_cyclces_now() < deadline)
      ;
  }

  uint64_t get_next_deadline() {
    uint64_t cycles_diff = r->generate();
    return get_cyclces_now() + cycles_diff;
  }

public:
  Worker(RandGen *r_, AppGen *a_, uint32_t q) : r(r_), a(a_), queue_id(q) {}

  void do_work() {
    std::cout << "Do work\n";
    uint64_t deadline = get_cyclces_now();

    while (!force_quit) {
      /* Process Responses */
      dpdk_poll();

      /* Generate new request */
      auto *mbuf = prepare_req();

      /* Wait for the amount of time */
      wait_till(deadline);

      /* Send request */

      /* Get next timestamp */
      deadline = get_next_deadline();
    }
  }

  uint32_t get_queue_id() { return queue_id; }
};
