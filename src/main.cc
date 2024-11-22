#include <cstdio>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "app.h"
#include "dpdk.h"
#include "manager.h"
#include "net.h"
#include "rand.h"
#include "stats.h"
#include "worker.h"

extern "C" {
#include <rte_lcore.h>
}

RTE_DEFINE_PER_LCORE(Worker *, local_worker);
struct rte_mempool *pktmbuf_pool;

struct ExpCfg {
public:
  RandGen *r;
  AppGen *a;
  Target t;
  uint16_t spin_usec;
  uint32_t duration;
  uint32_t replay_log_size;
  char* replay_log;
  FILE* log; // for results
};

// Map txn logs into memory
char* mmap_replay_log(char* log_name) {
  int fd = open(log_name, O_RDONLY);
  if (fd == -1) 
  {
    printf("File not existed\n");
    exit(1);
  }
  struct stat sb;
  fstat(fd, &sb);
  char* read_top = reinterpret_cast<char*>(mmap(nullptr, sb.st_size, 
    PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_POPULATE, fd, 0));

  return read_top;
}

static ExpCfg *parse_args(int argc, char **argv) {
  auto *cfg = new ExpCfg;

  int i = 0;
  while (i < argc) {
    if (strcmp(argv[i], "-i") == 0) {
      i++;
      cfg->r = new RandGen(argv[i]);
    } else if (strcmp(argv[i], "-s") == 0) {
      i++;
      char* mmap_ret = mmap_replay_log(argv[i]);
      cfg->replay_log_size = *(reinterpret_cast<uint32_t*>(mmap_ret));
      std::cout << "replay log size is " << cfg->replay_log_size << std::endl;
      cfg->replay_log = mmap_ret + sizeof(uint32_t);
    } else if (strcmp(argv[i], "-u") == 0) {
      i++;
      cfg->spin_usec = atoi(argv[i]);
    } else if (strcmp(argv[i], "-a") == 0) {
      i++;
      cfg->a = new AppGen(argv[i], cfg->replay_log_size, cfg->replay_log, cfg->spin_usec);
    } else if (strcmp(argv[i], "-t") == 0) {
      i++;
      cfg->t.ip = ip_str_to_int(argv[i]);
    } else if (strcmp(argv[i], "-p") == 0) {
      i++;
      cfg->t.port = atoi(argv[i]);
    } else if (strcmp(argv[i], "-d") == 0) {
      i++;
      cfg->duration = atoi(argv[i]);
    } else if (strcmp(argv[i], "-l") == 0) {
      i++;
      cfg->log = fopen(argv[i], "a");
    }
    i++;
  }

  return cfg;
}

static std::vector<Stats *> prepare_data_stores(uint8_t workers_count) {
  std::vector<Stats *> res;
  for (int i = 0; i < workers_count; i++) {
    res.push_back(new Stats);
  }
  return res;
}

int main(int argc, char **argv) {
  int count, lcore_id, ret = 0;

  printf("Hello world\n");

  DPDKManager::dpdk_init(&argc, &argv);

  printf("There are %d cores\n", rte_lcore_count());

  auto *cfg = parse_args(argc, argv);

  net_init();
  auto worker_stats = prepare_data_stores(rte_lcore_count() - 1);

  count = 0;
  RTE_LCORE_FOREACH_WORKER(lcore_id) {
    auto *w = new Worker(cfg->r, cfg->a, worker_stats[count], &cfg->t, count);
    rte_eal_remote_launch(Worker::worker_main, reinterpret_cast<void *>(w),
                          lcore_id);
    count++;
  }

  Manager m(worker_stats);

  m.manager_main(cfg->duration, cfg->log);

  RTE_LCORE_FOREACH_WORKER(lcore_id) {
    if (rte_eal_wait_lcore(lcore_id) < 0) {
      ret = -1;
      break;
    }
  }

  DPDKManager::dpdk_terminate();
  return ret;
}
