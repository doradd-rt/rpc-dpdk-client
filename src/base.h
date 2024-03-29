#pragma once
#include <vector>

#include "stats.h"

extern "C" {
#include <rte_mbuf.h>
}

RTE_DECLARE_PER_LCORE(int, queue_id);
extern volatile bool force_quit;
extern rte_mempool *pktmbuf_pool;

/* DPDK functionality */
void dpdk_init(int *argc, char ***argv);
void dpdk_terminate(void);
void dpdk_poll(void);
void dpdk_out(struct rte_mbuf *pkt);

/* Client functionality */
int worker_main(void *arg);
int manager_main(std::vector<Stats *> workers);
