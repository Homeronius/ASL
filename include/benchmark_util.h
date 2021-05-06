#ifndef _BENCHMARK_UTIL_H_
#define _BENCHMARK_UTIL_H_

#define NUM_RUNS 10
#define FREQUENCY 2.6e9

// Intel Skylake architecture performance events
// CAUTION! Highly processor dependent!
// See Intel 64 and IA-32 Architectures Software Developers's Manual
// Volume 3B, Part 2, Chapter 19 (page 19-11 for Skylake)
// CAUTION! Does not count divisions and sqrt. Further tests should be done on
// what is actually counted and what not.
#define FP_ARITH_INST_RETIRED_SCALAR_DOUBLE 0x01C7
#define FP_ARITH_INST_RETIRED_SCALAR_SINGLE 0x02C7
#define FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE 0x04C7
#define FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE 0x08C7
#define FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE 0x10C7
#define FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE 0x20C7

//------------------------------------------------------------------------------
//---- Cycle counter
//------------------------------------------------------------------------------

double rdtsc(void (*compute)());
double c_clock(void (*compute)());
double timeofday(void (*compute)());
void measure_and_print(void (*compute)());
int start_perf_cycle_counter();
long long stop_perf_cycle_counter(int fd);

//------------------------------------------------------------------------------
//---- Flops counter
//------------------------------------------------------------------------------

int start_flops_counter(unsigned long config);
long long stop_flops_counter(int fd);

#endif // _BENCHMARK_UTIL_H_
