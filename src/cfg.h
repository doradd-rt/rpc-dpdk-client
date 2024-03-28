#pragma once

#include "app.h"
#include "rand.h"

struct Target {
  uint32_t ip;
  uint16_t port;
};

struct ExpCfg {
public:
  RandGen *r;
  AppGen *a;
  Target t;
};
