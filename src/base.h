#pragma once
#include <vector>

#include "stats.h"

extern "C" {
#include <rte_mbuf.h>
}

extern rte_mempool *pktmbuf_pool;

/* DPDK functionality */
void dpdk_init(int *argc, char ***argv);
void dpdk_terminate(void);
void dpdk_poll(uint8_t queue_id);
void dpdk_out(struct rte_mbuf *pkt, uint8_t queue_id);
