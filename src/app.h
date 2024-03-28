#pragma once

#include <iostream>

class LoopRpc {
public:
  LoopRpc(char *arg) { std::cout << "Will create Loop protocol\n"; }
};

#if defined(LOOP)
using AppGen = LoopRpc;
#endif
