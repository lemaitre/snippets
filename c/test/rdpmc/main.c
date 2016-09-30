#include <stdio.h>
#include <stdlib.h>
#include <linux/perf_event.h>
#include "../../rdpmc.h"
#include "../../rdpmc.c"


//void test() {
//  int i;
//  printf("sleeping");
//  fflush(stdout);
//  sleep(1);
//  for (i = 0; i < 3; i++) {
//    printf(".");
//    fflush(stdout);
//    sleep(1);
//  }
//  printf(" Done.\n");
//  fflush(stdout);
//}

void test() {
  volatile float x = 0;
  float y = 0;
  for (int i = 0; i < 1e6; i++) {
    float f = i;
    y += f*f;
  }
  x = y;
}
int main() {
  uint64_t i0, i1, c0, c1, rc0, rc1;
  pmc_enable_fixed();
  i0   = rdpmc_instr();
  c0   = rdpmc_actual_cycles();
  rc0  = rdpmc_ref_cycles();
  test();
  i1  = rdpmc_instr();
  c1  = rdpmc_actual_cycles();
  rc1 = rdpmc_ref_cycles();
  printf("instructions: %llu\n", i1   - i0);
  printf("      cycles: %llu\n", c1   - c0);
  printf("  ref cycles: %llu\n", rc1  - rc0);
  return 0;
}
