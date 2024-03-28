#pragma once

#include <iostream>

class FixedGen {
  uint64_t val;

public:
  FixedGen(char *arg) {
    std::cout << "Will create fixed " << arg << std::endl;

    std::cout << "Assume value in usec and need to transform to cycles";
    uint64_t t = atoi(arg);
    uint64_t hz = rte_get_timer_hz();
    std::cout << "hz = " << hz << std::endl;
    std::cout << "t = " << t << std::endl;
    val = (hz * t) / 1e6;

    std::cout << "This will be " << val << std::endl;
  }

  uint64_t generate() {
    std::cout << "Will generate Fixed\n";

    return val;
  }
};

class ExpGen {
public:
  ExpGen(char *arg) { std::cout << "Will create Exp\n"; }
  uint64_t generate() {
    std::cout << "Will generate Exp\n";
    return 0;
  }
};

#if defined(FIXED)
using RandGen = FixedGen;
#elif defined(EXPONENTIAL)
using RandGen = ExpGen;
#endif
