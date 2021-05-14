#include "benchmark_util.h"
#include "config.h"
#include "hdbscan.h"
#include "unistd.h"
#include <locale.h>

#ifdef OUTPUT_COMPILER_INFO
void output_compiler_info() {
  printf("Compiled with\n");
  printf("- Compiler ID:\t\t %s\n", COMPILER_ID);
  printf("- Compiler version:\t %s\n", COMPILER_VERSION);
  printf("- Compiler flags:\t %s\n", COMPILER_FLAGS);
  // printf("Optimization flag: -%s\n", OPT_LEVEL);
  printf("\n");
}
#endif

static char *dataset_path;
static double *dataset;
static int *labels;
static int n;
static int d;
static HDBSCAN *clusterer;

void compute_hdbscan() {
  const int mpts = 4;
  const int minimum_cluster_size = 5;
  clusterer = new HDBSCAN(mpts, minimum_cluster_size, dataset, labels, n, d);

  clusterer->build_mst();
  clusterer->build_condensed_cluster_tree();
  clusterer->select_clusters();
  clusterer->extract_labels();

  delete clusterer;
}

long long measure_flops(unsigned long config) {
  int fd = start_flops_counter(config);
  compute_hdbscan();
  long long count = stop_flops_counter(fd);

  return count;
}

int main(int argc, char **argv) {

#ifdef OUTPUT_COMPILER_INFO
  output_compiler_info();
#endif

  if (argc < 2 || argc > 3) {
    printf("Usage: hdbscan_benchmark <input_path> [<output-file>]\n");
    return -1;
  }

  dataset_path = argv[1];
  load_dataset(dataset_path, &dataset, &labels, &n, &d);

  setlocale(LC_NUMERIC, "");

  double r = rdtsc(&compute_hdbscan);
  printf("RDTSC: %'lf cycles (%'lf sec @ %'lf MHz)\n", r, r / (FREQUENCY),
         (FREQUENCY) / 1e6);

  // On some systems, this number seems to be actually computed from a timer in
  // seconds then transformed into clock ticks using the variable CLOCKS_PER_SEC
  // Unfortunately, it appears that CLOCKS_PER_SEC is sometimes set improperly.
  double c = c_clock(&compute_hdbscan) * FREQUENCY / CLOCKS_PER_SEC;
  printf("C clock(): %'lf cycles (%'lf sec @ %'lf MHz)\n", c, c / (FREQUENCY),
         (FREQUENCY) / 1e6);

  double t = timeofday(&compute_hdbscan);
  printf("C gettimeofday(): %'lf sec\n\n", t);

  int fd = start_perf_cycle_counter();
  compute_hdbscan();
  long long p = stop_perf_cycle_counter(fd);
  printf("PERF cycles: %'lld\n", p);

#ifndef BENCHMARK_AMD

  int n = 3;
  unsigned long ids[n];
  unsigned long result[n];

  const unsigned long double_configs[] = {
      FP_ARITH_INST_RETIRED_SCALAR_DOUBLE,
      FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE,
      FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE};

  // Note: for some reason, monitoring multiple performance events only works
  // for up to 4 events.
  fd = start_all_flops_counter(double_configs, ids, n);
  compute_hdbscan();
  stop_all_flops_counter(fd, ids, result, n);

  long long sd = result[0];
  long long pd128 = result[1];
  long long pd256 = result[2];

  const unsigned long single_configs[] = {
      FP_ARITH_INST_RETIRED_SCALAR_SINGLE,
      FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE,
      FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE};

  fd = start_all_flops_counter(single_configs, ids, n);
  compute_hdbscan();
  stop_all_flops_counter(fd, ids, result, n);

  long long ss = result[0];
  long long ps128 = result[1];
  long long ps256 = result[2];

  printf("FLOPS count scalar double: %'lld\n", sd);
  printf("FLOPS count scalar float: %'lld\n", ss);
  printf("FLOPS count 128 packed double: %'lld (multiply by 2 to get scalar "
         "operations)\n",
         pd128);
  printf("FLOPS count 128 packed float: %'lld (multiply by 4 to get scalar "
         "operations)\n",
         ps128);
  printf("FLOPS count 256 packed double: %'lld (multiply by 4 to get scalar "
         "operations)\n",
         pd256);
  printf("FLOPS count 256 packed float: %'lld (multiply by 8 to get scalar "
         "operations)\n\n",
         ps256);

  long long scalar_flops = sd + 2 * pd128 + 4 * pd256;
  printf("Performance (PERF) = %'f dflops/cycle\n", (double)scalar_flops / p);
  printf("Performance (RDTSC) = %'f dflops/cycle\n", (double)scalar_flops / r);

  FILE *fptr;
  if (argc == 3) {
    // Check if file already exists
    if (access(argv[2], F_OK) == 0) { // exists -> append
      fptr = fopen(argv[2], "a");
      fprintf(fptr, "%i,%i,%f,%f,%f,%lld,%lld,%lld,%lld,%lld,%lld,%lld\n", n, d,
              r, c, t, p, sd, ss, pd128, ps128, pd256, ps256);
    } else { // file doesn't exist -> create new, inclusive header line
      fptr = fopen(argv[2], "w");
      // Header
      fprintf(fptr, "n,d,r,c,t,p,sd,ss,pd128,ps128,pd256,ps256\n");
      fprintf(fptr, "%i,%i,%f,%f,%f,%lld,%lld,%lld,%lld,%lld,%lld,%lld\n", n, d,
              r, c, t, p, sd, ss, pd128, ps128, pd256, ps256);
    }

    fclose(fptr);
  }
#else
  long long all_flops = measure_flops(ALL_FLOPS);
  printf("FLOPS count overall: %'lld\n", all_flops);
  long long add_sub = measure_flops(ADD_SUB_FLOPS);
  printf("FLOPS count add/sub: %'lld\n", add_sub);
  long long mult = measure_flops(MULT_FLOPS);
  printf("FLOPS count mult: %'lld\n", mult);
  long long div_sqrt = measure_flops(DIV_SQRT_FLOPS);
  printf("FLOPS count div/sqrt: %'lld\n", div_sqrt);
  long long mult_add = measure_flops(MULT_ADD_FLOPS);
  printf("FLOPS count multiply-add: %'lld\n", mult_add);
  FILE *fptr;
  if (argc == 3) {
    // Check if file already exists
    if (access(argv[2], F_OK) == 0) { // exists -> append
      fptr = fopen(argv[2], "a");
      fprintf(fptr, "%i,%i,%f,%f,%f,%lld,%lld,%lld,%lld,%lld,%lld\n", n, d, r,
              c, t, p, all_flops, add_sub, mult, div_sqrt, mult_add);
    } else { // file doesn't exist -> create new, inclusive header line
      fptr = fopen(argv[2], "w");
      // Header
      fprintf(fptr, "n,d,r,c,t,p,all,add_sub,mult,div_sqrt,mult_add\n");
      fprintf(fptr, "%i,%i,%f,%f,%f,%lld,%lld,%lld,%lld,%lld,%lld\n", n, d, r,
              c, t, p, all_flops, add_sub, mult, div_sqrt, mult_add);
    }

    fclose(fptr);
  }
#endif

  free(dataset);
  free(labels);

  return 0;
}
