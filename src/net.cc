#include "net.h"
#include "config.h"
#include "worker.h"

uint32_t local_ip;
char local_mac[] = {0xde, 0xad, 0xbe, 0xef, 0x5e, 0xb1};
static struct rte_ether_addr known_haddrs[ARP_ENTRIES_COUNT];

static inline int str_to_eth_addr(const char *src, struct rte_ether_addr *dst) {
  if (sscanf(src, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &dst->addr_bytes[0],
             &dst->addr_bytes[1], &dst->addr_bytes[2], &dst->addr_bytes[3],
             &dst->addr_bytes[4], &dst->addr_bytes[5]) != 6)
    return -1;
  return 0;
}

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
  RTE_PER_LCORE(local_worker)->process_response(pkt);
}

int net_init() {
  local_ip = LOCAL_IP;

  arp_init();

  return 0;
}
