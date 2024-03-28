#pragma once

struct __attribute__((__packed__)) custom_rpc_header {
  uint64_t sent_timestamp;

public:
  void set(uint64_t timestamp) { sent_timestamp = timestamp; }
};
