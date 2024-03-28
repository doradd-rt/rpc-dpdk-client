#pragma once

#include <iostream>

class FixedGen {
public:
  FixedGen(char *arg) { std::cout << "Will create fixed\n"; }

  double generate() {
    std::cout << "Will generate Fixed\n";
    return 0;
  }
};

class ExpGen {
public:
  ExpGen(char *arg) { std::cout << "Will create Exp\n"; }
  double generate() {
    std::cout << "Will generate Exp\n";
    return 0;
  }
};

#if defined(FIXED)
using RandGen = FixedGen;
#elif defined(EXPONENTIAL)
using RandGen = ExpGen;
#endif
