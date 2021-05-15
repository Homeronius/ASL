#ifndef _BENCHMARK_UTIL_H_
#define _BENCHMARK_UTIL_H_

#define NUM_RUNS 3
#ifndef BENCHMARK_AMD
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

#else
// base frequency of AMD Ryzen 7 4800H (alex' machine)
#define FREQUENCY 2.9e9

// AMD Family 17h Zen 2 performance events
// for RETIRED_SSE_AVX_OPERATIONS.
// Zen 2 does not seem to differ between single or double precision.
// see:
// https://lore.kernel.org/lkml/20200318190002.307290-3-vijaythakkar@me.com/ I
// also followed the instructions here:
// https://stackoverflow.com/questions/46104738/amd-perf-events
// i.e. inspect the performance events as given by
// libpfm4, specifically the ones given by ./showevtinfo

#define ALL_FLOPS 0xff03
#define ADD_SUB_FLOPS 0x0103
#define MULT_FLOPS 0x0203
#define DIV_SQRT_FLOPS 0x0403
#define MULT_ADD_FLOPS 0x0803

#endif

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

// Reference for monitoring multiple perf events:
// https://stackoverflow.com/questions/42088515/perf-event-open-how-to-monitoring-multiple-events
struct read_format_t {
  unsigned long nr;
  struct {
    unsigned long value;
    unsigned long id;
  } values[];
};

int start_flops_counter(unsigned long config);
long long stop_flops_counter(int fd);
int start_all_flops_counter(const unsigned long *configs, unsigned long *ids,
                            int n);
void stop_all_flops_counter(int fd, unsigned long *ids, unsigned long *result,
                            int n);

#endif // _BENCHMARK_UTIL_H_
