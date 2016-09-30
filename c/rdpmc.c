#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

// There is currently no glibc wrapper, so you have to call it as syscall
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
  int ret;

  ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                 group_fd, flags);
  return ret;
}

int pmc_enable(uint64_t config) {
  int i, err;
  struct perf_event_attr attr;
  int perf_hw;
  long long result = 0;
 
  // Configure th event
  memset(&attr, 0, sizeof(struct perf_event_attr));
  attr.type = PERF_TYPE_HARDWARE;
  attr.size = sizeof(struct perf_event_attr);
  attr.config = config;
  attr.inherit = 1;
 
  // Due to the setting of attr.inherit, it will also count all child
  perf_hw = perf_event_open(&attr, 0, -1, -1, 0);
  if (perf_hw < 0) return 0;
 
  // Resetting counter to zero
  ioctl(perf_hw, PERF_EVENT_IOC_RESET, 0);
  // Start counters
  ioctl(perf_hw, PERF_EVENT_IOC_ENABLE, 0);

  return 1;
}

int pmc_enable_fixed() {
  int res = 1;
  if (!pmc_enable(PERF_COUNT_HW_INSTRUCTIONS)) {
    fprintf(stderr, "Failed to start HW_INSTRUCTIONS\n");
    res = 0;
  }
  if (!pmc_enable(PERF_COUNT_HW_REF_CPU_CYCLES)) {
    fprintf(stderr, "Failed to start HW_REF_CPU_CYCLES\n");
    res = 0;
  }
  return res;
}
