#pragma once

#include <iostream>

#include "app.h"
#include "dpdk.h"
#include "net.h"
#include "rand.h"
#include "stats.h"

extern "C" {
#include <rte_cycles.h>
#include <rte_eal.h>
#include <rte_mbuf.h>
}

struct Target {
  uint32_t ip;
  uint16_t port;
};

class Worker;
RTE_DECLARE_PER_LCORE(Worker *, local_worker);

class Worker {
  RandGen *r;
  AppGen *a;
  Stats *s;
  Target *target;
  uint32_t queue_id;

  rte_mbuf *prepare_req() {
    std::cout << "Will prepare a request\n";
    auto *pkt = rte_pktmbuf_alloc(pktmbuf_pool);
    rte_ether_hdr *ethh = rte_pktmbuf_mtod(pkt, rte_ether_hdr *);
    rte_ipv4_hdr *iph = reinterpret_cast<rte_ipv4_hdr *>(ethh + 1);
    rte_udp_hdr *udph = reinterpret_cast<rte_udp_hdr *>(iph + 1);
    custom_rpc_header *rpch = reinterpret_cast<custom_rpc_header *>(udph + 1);

    char *payload = reinterpret_cast<char *>(rpch + 1);
    uint16_t payload_len = a->prepare_req(payload, get_max_payload_size());
    uint16_t overall_len = payload_len;

    /*There the custom header that we will populate right before transmitting */
    overall_len += sizeof(custom_rpc_header);

    uint16_t rand_src_port = (rand() & 0x3ff) | 0xC00;
    udp_out_prepare(udph, rand_src_port, target->port, overall_len);
    overall_len += sizeof(rte_udp_hdr);

    ip_out_prepare(iph, local_ip, target->ip, 64, 0, IPPROTO_UDP, overall_len);
    overall_len += sizeof(rte_ipv4_hdr);

    eth_out_prepare(ethh, RTE_ETHER_TYPE_IPV4, get_mac_addr(target->ip));
    overall_len += sizeof(rte_ether_hdr);

    pkt->data_len = overall_len;
    pkt->pkt_len = overall_len;

    return pkt;
  }

  inline uint64_t get_cyclces_now() {
    // Use the DPDK one for generality
    return rte_get_timer_cycles();
  }

  uint64_t get_next_deadline() {
    uint64_t cycles_diff = r->generate();
    return get_cyclces_now() + cycles_diff;
  }

  void prepare_custom_header(rte_mbuf *pkt) {
    custom_rpc_header *rpch = rte_pktmbuf_mtod_offset(
        pkt, custom_rpc_header *,
        sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr) + sizeof(rte_udp_hdr));
    rpch->set(get_cyclces_now());
  }

public:
  Worker(RandGen *r_, AppGen *a_, Stats *s_, Target *t, uint32_t q)
      : r(r_), a(a_), s(s_), target(t), queue_id(q) {}

  void do_work() {
    std::cout << "Do work\n";
    uint64_t deadline = get_cyclces_now();
    auto *pkt = prepare_req();

    while (s->should_load()) {
      /* Process Responses */
      DPDKManager::dpdk_poll(queue_id);

      if (get_cyclces_now() < deadline)
        continue;

      /* Get next timestamp */
      deadline = get_next_deadline();

      /* Populate the curstom header with the send timestamp */
      prepare_custom_header(pkt);

      /* Send request */
      DPDKManager::dpdk_out(pkt, queue_id);

      /* Generate new request */
      pkt = prepare_req();
    }
  }

  uint32_t get_queue_id() { return queue_id; }

  void process_response(rte_mbuf *pkt) {
    std::cout << "Will process responce in the worker\n";
    custom_rpc_header *rpch = rte_pktmbuf_mtod_offset(
        pkt, custom_rpc_header *,
        sizeof(rte_ether_hdr) + sizeof(rte_ipv4_hdr) + sizeof(rte_udp_hdr));
    uint64_t latency = get_cyclces_now() - rpch->get();
    uint64_t usec = (latency * 1e6) / rte_get_timer_hz();
    printf("The request took %lu cycles or %lu usec\n", latency, usec);
  }

  static int worker_main(void *arg) {
    Worker *w = reinterpret_cast<Worker *>(arg);
    printf("Worker main: %d\n", w->get_queue_id());

    RTE_PER_LCORE(local_worker) = w;

    w->do_work();

    return 0;
  }
};
