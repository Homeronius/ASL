// Inspired by ASL homework 1

#include <asm/unistd.h>
#include <cassert>
#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "benchmark_util.h"
#include "tsc_x86.h"

#define CYCLES_REQUIRED 1e8
#define CALIBRATE

//------------------------------------------------------------------------------
//---- Helpers
//------------------------------------------------------------------------------

long perf_event_open(perf_event_attr *hw_event, pid_t pid, int cpu,
                     int group_fd, unsigned long flags) {
  int ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
  return ret;
}

//------------------------------------------------------------------------------
//---- Cycle counter
//------------------------------------------------------------------------------

/*
 * Timing function based on the TimeStep Counter of the CPU.
 */

double rdtsc(void (*compute)()) {
  int i, num_runs;
  myInt64 cycles;
  myInt64 start;
  num_runs = NUM_RUNS;

/*
 * The CPUID instruction serializes the pipeline.
 * Using it, we can create execution barriers around the code we want to time.
 * The calibrate section is used to make the computation large enough so as to
 * avoid measurements bias due to the timing overhead.
 */
#ifdef CALIBRATE
  while (num_runs < (1 << 14)) {
    start = start_tsc();
    for (i = 0; i < num_runs; ++i) {
      compute();
    }
    cycles = stop_tsc(start);

    if (cycles >= CYCLES_REQUIRED)
      break;

    num_runs *= 2;
  }
#endif

  start = start_tsc();
  for (i = 0; i < num_runs; ++i) {
    compute();
  }

  cycles = stop_tsc(start) / num_runs;
  return (double)cycles;
}

double c_clock(void (*compute)()) {
  int i, num_runs;
  double cycles;
  clock_t start, end;

  num_runs = NUM_RUNS;
#ifdef CALIBRATE
  while (num_runs < (1 << 14)) {
    start = clock();
    for (i = 0; i < num_runs; ++i) {
      compute();
    }
    end = clock();

    cycles = (double)(end - start);

    // Same as in c_clock: CYCLES_REQUIRED should be expressed accordingly to
    // the order of magnitude of CLOCKS_PER_SEC
    if (cycles >= CYCLES_REQUIRED / (FREQUENCY / CLOCKS_PER_SEC))
      break;

    num_runs *= 2;
  }
#endif

  start = clock();
  for (i = 0; i < num_runs; ++i) {
    compute();
  }
  end = clock();

  return (double)(end - start) / num_runs;
}

double timeofday(void (*compute)()) {
  int i, num_runs;
  double cycles;
  struct timeval start, end;

  num_runs = NUM_RUNS;
#ifdef CALIBRATE
  while (num_runs < (1 << 14)) {
    gettimeofday(&start, NULL);
    for (i = 0; i < num_runs; ++i) {
      compute();
    }
    gettimeofday(&end, NULL);

    cycles = (double)((end.tv_sec - start.tv_sec) +
                      (end.tv_usec - start.tv_usec) / 1e6) *
             FREQUENCY;

    if (cycles >= CYCLES_REQUIRED)
      break;

    num_runs *= 2;
  }
#endif

  gettimeofday(&start, NULL);
  for (i = 0; i < num_runs; ++i) {
    compute();
  }
  gettimeofday(&end, NULL);

  return (double)((end.tv_sec - start.tv_sec) +
                  (end.tv_usec - start.tv_usec) / 1e6) /
         num_runs;
}

void measure_and_print(void (*compute)()) {
  double r = rdtsc(compute);
  printf("RDTSC: %lf cycles (%lf sec @ %lf MHz)\n", r, r / (FREQUENCY),
         (FREQUENCY) / 1e6);

  // On some systems, this number seems to be actually computed from a timer in
  // seconds then transformed into clock ticks using the variable CLOCKS_PER_SEC
  // Unfortunately, it appears that CLOCKS_PER_SEC is sometimes set improperly.
  double c = c_clock(compute) * FREQUENCY / CLOCKS_PER_SEC;
  printf("C clock(): %'lf cycles (%'lf sec @ %'lf MHz)\n", c, c / (FREQUENCY),
         (FREQUENCY) / 1e6);

  double t = timeofday(compute);
  printf("C gettimeofday(): %lf sec\n\n", t);
}

int start_perf_cycle_counter() {
  perf_event_attr attr;

  memset(&attr, 0, sizeof(perf_event_attr));
  attr.size = sizeof(perf_event_attr);
  attr.type = PERF_TYPE_HARDWARE;
  attr.config = PERF_COUNT_HW_CPU_CYCLES;
  attr.disabled = 1;
  attr.exclude_kernel = 1; // do not count instructions the kernel executes
  attr.exclude_hv = 1;

  // open file descriptor
  int fd = perf_event_open(&attr, 0, -1, -1, 0);

  if (fd == -1) {
    return -1;
  }

  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

  return fd;
}

long long stop_perf_cycle_counter(int fd) {
  if (fd == -1) {
    return 1;
  }

  // disable and read out the counter
  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
  long long count;
  read(fd, &count, sizeof(long long));
  close(fd);

  return count;
}

//------------------------------------------------------------------------------
//---- Flops counter
//------------------------------------------------------------------------------

// For some reason max. 4 events work
int start_all_flops_counter(const unsigned long *configs, unsigned long *ids,
                            int n) {
  assert(n > 1);

  perf_event_attr attr;
  int fds[n];

  // group leader
  memset(&attr, 0, sizeof(perf_event_attr));
  attr.size = sizeof(perf_event_attr);
  attr.type = PERF_TYPE_RAW;
  attr.config = configs[0];
  attr.disabled = 1;
  attr.exclude_kernel = 1; // do not count instructions the kernel executes
  attr.exclude_hv = 1;
  attr.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
  fds[0] = perf_event_open(&attr, 0, -1, -1, 0);

  assert(fds[0]);

  ioctl(fds[0], PERF_EVENT_IOC_ID, ids);

  for (int i = 1; i < n; i++) {
    memset(&attr, 0, sizeof(struct perf_event_attr));
    attr.size = sizeof(struct perf_event_attr);
    attr.type = PERF_TYPE_RAW;
    attr.config = configs[i];
    attr.disabled = 1;
    attr.exclude_kernel = 1;
    attr.exclude_hv = 1;
    attr.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
    fds[i] = perf_event_open(&attr, 0, -1, fds[0], 0);

    assert(fds[i]);

    ioctl(fds[i], PERF_EVENT_IOC_ID, ids + i);
  }

  ioctl(fds[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
  ioctl(fds[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

  return fds[0];
}

void stop_all_flops_counter(int fd, unsigned long *ids, unsigned long *result,
                            int n) {
  assert(fd != -1);

  // disable and read out the counter
  ioctl(fd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);

  char buf[4096];
  struct read_format_t *rf = (struct read_format_t *)buf;

  read(fd, &buf, sizeof(buf));

  for (int i = 0; i < rf->nr; i++) {
    for (int k = 0; k < n; k++) {
      if (rf->values[i].id == ids[k]) {
        result[k] = rf->values[i].value;
      }
    }
  }
  close(fd);
}

int start_flops_counter(unsigned long config) {
  perf_event_attr attr;

  memset(&attr, 0, sizeof(perf_event_attr));
  attr.size = sizeof(perf_event_attr);
  attr.type = PERF_TYPE_RAW;
  attr.config = config;
  // attr.type = PERF_TYPE_HARDWARE;
  // attr.config = PERF_COUNT_HW_INSTRUCTIONS;
  attr.disabled = 1;
  attr.exclude_kernel = 1; // do not count instructions the kernel executes
  attr.exclude_hv = 1;

  // open file descriptor
  int fd = perf_event_open(&attr, 0, -1, -1, 0);

  if (fd == -1) {
    return -1;
  }

  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

  return fd;
}

long long stop_flops_counter(int fd) {
  if (fd == -1) {
    return 0;
  }

  // disable and read out the counter
  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
  long long count;
  read(fd, &count, sizeof(long long));
  close(fd);

  return count;
}
