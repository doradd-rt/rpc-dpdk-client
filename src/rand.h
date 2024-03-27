#pragma once

#include <iostream>

class RandGen {
public:
  virtual double generate() = 0;
};

class FixedGen : public RandGen {
  double generate() {
    std::cout << "Will generate Fixed\n";
    return 0;
  }
};

class ExpGen : public RandGen {
  double generate() {
    std::cout << "Will generate Exp\n";
    return 0;
  }
};

RandGen *create_generator(char *arg);
