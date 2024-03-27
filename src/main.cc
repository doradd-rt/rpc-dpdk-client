#include <csignal>
#include <cstdio>
#include <iostream>

#include "base.h"
#include "net.h"
#include "rand.h"
#include "worker.h"

extern "C" {
#include <rte_lcore.h>
}

volatile bool force_quit;

class ExpCfg {
public:
  RandGen *r;
  // FIXME: This should be the application payload generator
  void *a;
};

static ExpCfg *parse_args(int argc, char **argv) {
  auto *cfg = new ExpCfg;

  int i = 0;
  while (i < argc) {
    if (strcmp(argv[i], "-i") == 0) {
      i++;
      cfg->r = create_generator(argv[i]);
    } else if (strcmp(argv[i], "-a") == 0) {
      printf("Parse application");
      i++;
    }

    i++;
  }

  return cfg;
}

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

  auto *cfg = parse_args(argc, argv);

  net_init();

  count = 0;
  RTE_LCORE_FOREACH_WORKER(lcore_id) {
    auto *w = new Worker(cfg->r, count);
    rte_eal_remote_launch(worker_main, reinterpret_cast<void *>(w), lcore_id);
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
