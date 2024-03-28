#include <iostream>

#include "base.h"
#include "worker.h"

RTE_DEFINE_PER_LCORE(Worker *, local_worker);

int worker_main(void *arg) {
  Worker *w = reinterpret_cast<Worker *>(arg);
  printf("Worker main: %d\n", w->get_queue_id());

  RTE_PER_LCORE(queue_id) = w->get_queue_id();
  RTE_PER_LCORE(local_worker) = w;

  w->do_work();

  return 0;
}
