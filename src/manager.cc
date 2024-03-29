#include <iostream>
#include <vector>

#include "base.h"

int manager_main(std::vector<Stats *> workers) {
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
