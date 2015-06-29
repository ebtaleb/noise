#include "rand.h"

static unsigned int multiplier = 8513;
static unsigned int addend = 179;

unsigned int nextRandInt(unsigned int *v) {
  *v = *v * multiplier + addend;
  return *v;
}
