#include "benchmark_util.h"
#include "config.h"
#include "hdbscan.h"
#include "unistd.h"
#include <cctype>
#include <cstdio>
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

static char *dataset_path = NULL;
static double *dataset;
static int *labels;
static int n;
static int d;
static HDBSCAN *clusterer;
static int mpts = 4;

void compute_hdbscan() {
  const int minimum_cluster_size = 5;
  clusterer = new HDBSCAN(mpts, minimum_cluster_size, dataset, labels, n, d);

  clusterer->build_mst();
  clusterer->build_condensed_cluster_tree();
  clusterer->select_clusters();
  clusterer->extract_labels();

  delete clusterer;
}

unsigned long measure_flops(unsigned long config) {
  int fd = start_flops_counter(config);
  compute_hdbscan();
  unsigned long count = stop_flops_counter(fd);

  return count;
}

int main(int argc, char **argv) {

#ifdef OUTPUT_COMPILER_INFO
  output_compiler_info();
#endif

  int opt;
  char *ovalue = NULL;
  while ((opt = getopt(argc, argv, "i:k:do:")) != EOF) {
    switch (opt) {
    case 'k':
      mpts = atoi(optarg);
      break;
    case 'i':
      dataset_path = optarg;
      break;
    case 'o':
      ovalue = optarg;
      break;
    case '?':
      if (optopt == 'k')
        fprintf(stderr, "Option -%c requires an integer argument.\n", optopt);
      if (optopt == 'o' || optopt == 'i')
        fprintf(stderr, "Option -%c requires a string argument.\n", optopt);
      else
        fprintf(stderr, "Usage: hdbscan_benchmark -i <input_path> [-o "
                        "<output-file>] [-k <mpts>]\n");
      return 1;
    }
  }
  if (dataset_path == NULL) {
    fprintf(stderr, "Usage: hdbscan_benchmark -i <input_path> [-o "
                    "<output-file>] [-k <mpts>]\n");
    return 1;
  }
  printf("DATASET path: %s\n\n", dataset_path);

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
  unsigned long p = stop_perf_cycle_counter(fd);
  printf("PERF cycles: %'lu\n", p);

#ifndef BENCHMARK_AMD

  int k = 3;
  unsigned long ids[k];
  unsigned long result[k];

  const unsigned long double_configs[] = {
      FP_ARITH_INST_RETIRED_SCALAR_DOUBLE,
      FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE,
      FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE};

  // Note: for some reason, monitoring multiple performance events only works
  // for up to 4 events.
  fd = start_all_flops_counter(double_configs, ids, k);
  compute_hdbscan();
  stop_all_flops_counter(fd, ids, result, k);

  unsigned long sd = result[0];
  unsigned long pd128 = result[1];
  unsigned long pd256 = result[2];

  // const unsigned long single_configs[] = {
  // FP_ARITH_INST_RETIRED_SCALAR_SINGLE,
  // FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE,
  // FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE};

  // fd = start_all_flops_counter(single_configs, ids, k);
  // compute_hdbscan();
  // stop_all_flops_counter(fd, ids, result, k);

  // unsigned long ss = result[0];
  // unsigned long ps128 = result[1];
  // unsigned long ps256 = result[2];

  printf("FLOPS count scalar double: %'lu\n", sd);
  // printf("FLOPS count scalar float: %'lu\n", ss);
  printf("FLOPS count 128 packed double: %'lu (multiply by 2 to get scalar "
         "operations)\n",
         pd128);
  // printf("FLOPS count 128 packed float: %'lu (multiply by 4 to get scalar "
  //"operations)\n",
  // ps128);
  printf("FLOPS count 256 packed double: %'lu (multiply by 4 to get scalar "
         "operations)\n",
         pd256);
  // printf("FLOPS count 256 packed float: %'lu (multiply by 8 to get scalar "
  //"operations)\n\n",
  // ps256);

  unsigned long scalar_flops = sd + 2 * pd128 + 4 * pd256;
  printf(
      "Performance (PERF) = %'f dflops/cycle (only counting add, mul, sub)\n",
      (double)scalar_flops / p);
  printf(
      "Performance (RDTSC) = %'f dflops/cycle (only counting add, mul, sub)\n",
      (double)scalar_flops / r);

  FILE *fptr;
  if (ovalue != NULL) {
    printf("Got ovalue: %s\n\n", ovalue);
    // Check if file already exists
    if (access(ovalue, F_OK) == 0) { // exists -> append
      fptr = fopen(ovalue, "a");
      fprintf(fptr, "%i,%i,%f,%f,%f,%lu,%lu,%lu,%lu,%lu,%d\n", n, d, r, c, t, p,
              scalar_flops, sd, pd128, pd256, mpts);
    } else { // file doesn't exist -> create new, inclusive header line
      fptr = fopen(ovalue, "w");
      // Header
      fprintf(fptr, "n,d,r,c,t,p,sd,pd128,pd256\n");
      fprintf(fptr, "%i,%i,%f,%f,%f,%lu,%lu,%lu,%lu,%lu,%d\n", n, d, r, c, t, p,
              scalar_flops, sd, pd128, pd256, mpts);
    }

    fclose(fptr);
  }
#else
  int k = 3;
  unsigned long ids[k];
  unsigned long result[k];
  const unsigned long amd_configs[] = {
      ADD_SUB_FLOPS, MULT_FLOPS, DIV_SQRT_FLOPS,
      // MULT_ADD_FLOPS
  };

  fd = start_all_flops_counter(amd_configs, ids, k);
  compute_hdbscan();
  stop_all_flops_counter(fd, ids, result, k);

  unsigned long add_sub = result[0];
  printf("FLOPS count add/sub: %'lu\n", add_sub);
  unsigned long mult = result[1];
  printf("FLOPS count mult: %'lu\n", mult);
  unsigned long div_sqrt = result[2];
  printf("FLOPS count div/sqrt: %'lu\n", div_sqrt);
  // for some reason only n = 3 works to count
  // multiple events at the same time, hence do mult_add individually
  unsigned long mult_add = measure_flops(MULT_ADD_FLOPS);
  printf("FLOPS count multiply-add: %'lu\n", mult_add);
  unsigned long all_flops = add_sub + mult + div_sqrt + mult_add;
  printf("FLOPS count overall: %'lu\n", all_flops);

  printf("Performance (PERF) = %'f dflops/cycle\n", (double)all_flops / p);
  printf("Performance (RDTSC) = %'f dflops/cycle\n", (double)all_flops / r);

  FILE *fptr;
  if (ovalue != NULL) {
    // Check if file already exists
    if (access(ovalue, F_OK) == 0) { // exists -> append
      fptr = fopen(ovalue, "a");
      fprintf(fptr, "%i,%i,%f,%f,%f,%lu,%lu,%lu,%lu,%lu,%lu,%d\n", n, d, r, c,
              t, p, all_flops, add_sub, mult, div_sqrt, mult_add, mpts);
    } else { // file doesn't exist -> create new, inclusive header line
      fptr = fopen(ovalue, "w");
      // Header
      fprintf(fptr, "n,d,r,c,t,p,all,add_sub,mult,div_sqrt,mult_add\n");
      fprintf(fptr, "%i,%i,%f,%f,%f,%lu,%lu,%lu,%lu,%lu,%lu,%d\n", n, d, r, c,
              t, p, all_flops, add_sub, mult, div_sqrt, mult_add, mpts);
    }

    fclose(fptr);
  }
#endif

  free(dataset);
  free(labels);

  return 0;
}
