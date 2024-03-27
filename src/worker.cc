#include <iostream>

#include "base.h"
#include "worker.h"

int worker_main(void *arg) {
  Worker *w = reinterpret_cast<Worker *>(arg);
  printf("Worker main: %d\n", w->get_queue_id());

  RTE_PER_LCORE(queue_id) = w->get_queue_id();

  while (!force_quit) {
    /* Process Responses */
    dpdk_poll();
    break;
  }

  return 0;
}
