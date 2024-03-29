#include <iostream>
#include <vector>

#include "base.h"

extern volatile bool force_quit;

class Manager {
  std::vector<Stats *> workers;

public:
  Manager(std::vector<Stats *> w) : workers(w) {}

  int manager_main() {
    printf("Manager main\n");

    /* Start plugging your logic here */
    while (!force_quit)
      ;

    std::cout << "Manager will stop all workers now";

    for (Stats *w : workers) {
      w->stop_load();
    }
    return 0;
  }
};
