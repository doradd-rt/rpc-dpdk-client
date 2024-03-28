#pragma once

#include <iostream>

#include "app.h"
#include "base.h"
#include "cfg.h"
#include "net.h"
#include "rand.h"

extern "C" {
#include <rte_cycles.h>
#include <rte_mbuf.h>
}

class Worker {
  RandGen *r;
  AppGen *a;
  Target *target;
  uint32_t queue_id;
  // Location of results?

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

  inline void wait_till(uint64_t deadline) {
    // FIXME: Output a notification if the deadline is long past
    while (get_cyclces_now() < deadline)
      ;
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
  Worker(RandGen *r_, AppGen *a_, Target *t, uint32_t q)
      : r(r_), a(a_), target(t), queue_id(q) {}

  void do_work() {
    std::cout << "Do work\n";
    uint64_t deadline = get_cyclces_now();

    while (!force_quit) {
      /* Process Responses */
      dpdk_poll();

      /* Generate new request */
      auto *pkt = prepare_req();

      /* Wait for the amount of time */
      wait_till(deadline);

      /* Get next timestamp */
      deadline = get_next_deadline();

      /* Populate the curstom header with the send timestamp */
      prepare_custom_header(pkt);

      /* Send request */
      dpdk_out(pkt);
    }
  }

  uint32_t get_queue_id() { return queue_id; }
};
