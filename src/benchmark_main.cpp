#include "benchmark_util.h"
#include "hdbscan.h"
#include "unistd.h"

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

  if (argc < 2 || argc > 3) {
    printf("Usage: hdbscan_benchmark <input_path> [<output-file>]\n");
    return -1;
  }

  dataset_path = argv[1];
  load_dataset(dataset_path, &dataset, &labels, &n, &d);

  double r = rdtsc(&compute_hdbscan);
  printf("RDTSC: %lf cycles (%lf sec @ %lf MHz)\n", r, r / (FREQUENCY),
         (FREQUENCY) / 1e6);

  // On some systems, this number seems to be actually computed from a timer in
  // seconds then transformed into clock ticks using the variable CLOCKS_PER_SEC
  // Unfortunately, it appears that CLOCKS_PER_SEC is sometimes set improperly.
  double c = c_clock(&compute_hdbscan);
  printf("C clock(): %lf cycles (%lf sec @ %lf MHz)\n", c, c / CLOCKS_PER_SEC,
         (double)CLOCKS_PER_SEC / 1e6);

  double t = timeofday(&compute_hdbscan);
  printf("C gettimeofday(): %lf sec\n\n", t);

  int fd = start_perf_cycle_counter();
  compute_hdbscan();
  long long p = stop_perf_cycle_counter(fd);
  printf("PERF cycles: %lld\n", p);
#ifndef BENCHMARK_AMD
  long long sd = measure_flops(FP_ARITH_INST_RETIRED_SCALAR_DOUBLE);
  printf("FLOPS count scalar double: %lld\n", sd);
  long long ss = measure_flops(FP_ARITH_INST_RETIRED_SCALAR_SINGLE);
  printf("FLOPS count scalar float: %lld\n", ss);
  long long pd128 = measure_flops(FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE);
  printf("FLOPS count 128 packed double: %lld (multiply by 2 to get scalar "
         "operations)\n",
         pd128);
  long long ps128 = measure_flops(FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE);
  printf("FLOPS count 128 packed float: %lld (multiply by 4 to get scalar "
         "operations)\n",
         ps128);
  long long pd256 = measure_flops(FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE);
  printf("FLOPS count 256 packed double: %lld (multiply by 4 to get scalar "
         "operations)\n",
         pd256);
  long long ps256 = measure_flops(FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE);
  printf("FLOPS count 256 packed float: %lld (multiply by 8 to get scalar "
         "operations)\n",
         ps256);

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
  long long sp_add_sub = measure_flops(SP_ADD_SUB_FLOPS);
  printf("FLOPS count float add/sub: %lld\n", sp_add_sub);
  long long sp_mult = measure_flops(SP_MULT_FLOPS);
  printf("FLOPS count float mult: %lld\n", sp_mult);
  long long sp_div = measure_flops(SP_DIV_FLOPS);
  printf("FLOPS count float div: %lld\n", sp_div);
  long long sp_mult_add = measure_flops(SP_MULT_ADD_FLOPS);
  printf("FLOPS count float multiply-add: %lld\n", sp_mult_add);
  long long dp_add_sub = measure_flops(DP_ADD_SUB_FLOPS);
  printf("FLOPS count double add/sub: %lld\n", dp_add_sub);
  long long dp_mult = measure_flops(DP_MULT_FLOPS);
  printf("FLOPS count double mult: %lld\n", dp_mult);
  long long dp_div = measure_flops(DP_DIV_FLOPS);
  printf("FLOPS count double div: %lld\n", dp_div);
  long long dp_mult_add = measure_flops(DP_MULT_ADD_FLOPS);
  printf("FLOPS count double multiply-add: %lld\n", dp_mult_add);
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
#endif

  free(dataset);
  free(labels);

  return 0;
}
