#include "net.h"
#include "arp-cfg.h"
#include "config.h"
#include "dpdk.h"
#include "worker.h"

uint32_t local_ip;
static struct rte_ether_addr known_haddrs[ARP_ENTRIES_COUNT];

// dbg only
static uint64_t cnt = 0;

static void arp_init(void) {
  printf("arp_init()\n");
  for (int i = 0; i < ARP_ENTRIES_COUNT; i++)
    str_to_eth_addr(arp_entries[i], &known_haddrs[i]);
  printf("arp_init() finished\n");
}

struct rte_ether_addr *get_mac_addr(uint32_t ip_addr) {
  uint32_t idx = (ip_addr & 0xff) - 1; // FIXME
  return &known_haddrs[idx];
}

void udp_pkt_process(struct rte_mbuf *pkt) {
  printf("Process incoming response\n");
  printf("cnt: %lu\n", cnt++);
  RTE_PER_LCORE(local_worker)->process_response(pkt);
}

int net_init() {
  local_ip = LOCAL_IP;

  arp_init();

  return 0;
}

void net_send_pkt(rte_mbuf *pkt) {
  DPDKManager::dpdk_out(pkt, RTE_PER_LCORE(local_worker)->get_queue_id());
}
