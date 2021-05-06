#include "benchmark_util.h"
#include "hdbscan.h"

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

  free(dataset);
  free(labels);

  if (argc == 3) {
    FILE *fptr = fopen(argv[2], "w");

    fprintf(fptr, "%f\n", r);
    fprintf(fptr, "%f\n", c);
    fprintf(fptr, "%f\n", t);
    fprintf(fptr, "%lld\n", p);
    fprintf(fptr, "%lld\n", sd);
    fprintf(fptr, "%lld\n", ss);
    fprintf(fptr, "%lld\n", pd128);
    fprintf(fptr, "%lld\n", ps128);
    fprintf(fptr, "%lld\n", pd256);
    fprintf(fptr, "%lld\n", ps256);

    fclose(fptr);
  }

  return 0;
}
