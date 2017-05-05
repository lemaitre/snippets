#ifndef RDPMC_H
#define RDPMC_H
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t inline __attribute((always_inline)) rdpmc(uint32_t counter)
{
  uint32_t a, d;
  __asm__ volatile("rdpmc" : "=a" (a), "=d" (d) : "c" (counter));
  return ((uint64_t)a) | (((uint64_t)d) << 32);;
}

uint64_t inline __attribute((always_inline)) rdpmc_instr() {
  return rdpmc(1 << 30);
}

uint64_t inline __attribute((always_inline)) rdpmc_actual_cycles() {
  return rdpmc((1 << 30) + 1);
}

uint64_t inline __attribute((always_inline)) rdpmc_ref_cycles() {
  return rdpmc((1 << 30) + 2);
}


int pmc_enable(uint64_t config);
int pmc_enable_fixed();

#ifdef __cplusplus
}
#endif

#endif
