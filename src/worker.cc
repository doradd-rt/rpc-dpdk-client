#include <iostream>

#include "base.h"

int worker_main(void *arg) {
  uint32_t thread_id = (int)(long)(arg);
  printf("Worker main: %d\n", thread_id);

  RTE_PER_LCORE(queue_id) = thread_id;

  while (!force_quit)
    dpdk_poll();

  return 0;
}
