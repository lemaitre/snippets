#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include "timer.h"

unsigned long long int read_time(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  return (unsigned long long int) t.tv_nsec + (unsigned long long int) 1000000000 * (unsigned long long int) t.tv_sec;
}
