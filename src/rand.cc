#include "rand.h"

RandGen *create_generator(char *arg) { return new FixedGen; }
