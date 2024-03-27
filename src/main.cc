#include <csignal>
#include <cstdio>
#include <iostream>

#include "base.h"
#include "net.h"

extern "C" {
#include <rte_lcore.h>
}

volatile bool force_quit;

static void signal_handler(int signum) {
  if (signum == SIGINT || signum == SIGTERM) {
    printf("\n\nSignal %d received, preparing to exit...\n", signum);
    force_quit = true;
  }
}

int main(int argc, char **argv) {
  int count, lcore_id, ret = 0;

  printf("Hello world\n");

  dpdk_init(&argc, &argv);

  /* set signal handler for proper exiting */
  force_quit = false;
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  printf("There are %d cores\n", rte_lcore_count());

  net_init();

  count = 0;
  RTE_LCORE_FOREACH_WORKER(lcore_id) {
    rte_eal_remote_launch(worker_main, (void *)(long)count, lcore_id);
    count++;
  }

  manager_main();

  RTE_LCORE_FOREACH_WORKER(lcore_id) {
    if (rte_eal_wait_lcore(lcore_id) < 0) {
      ret = -1;
      break;
    }
  }

  dpdk_terminate();
  return ret;
}
