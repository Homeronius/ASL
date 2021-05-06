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

  if (argc != 2) {
    printf("Usage: hdbscan_benchmark <input_path>\n");
    return -1;
  }

  dataset_path = argv[1];
  load_dataset(dataset_path, &dataset, &labels, &n, &d);

  measure_and_print(&compute_hdbscan);

  printf("FLOPS count scalar double: %lld\n",
         measure_flops(FP_ARITH_INST_RETIRED_SCALAR_DOUBLE));
  printf("FLOPS count scalar float: %lld\n",
         measure_flops(FP_ARITH_INST_RETIRED_SCALAR_SINGLE));
  printf("FLOPS count 128 packed double: %lld (multiply by 2 to get scalar "
         "operations)\n",
         measure_flops(FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE));
  printf("FLOPS count 128 packed float: %lld (multiply by 4 to get scalar "
         "operations)\n",
         measure_flops(FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE));
  printf("FLOPS count 256 packed double: %lld (multiply by 4 to get scalar "
         "operations)\n",
         measure_flops(FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE));
  printf("FLOPS count 256 packed float: %lld (multiply by 8 to get scalar "
         "operations)\n",
         measure_flops(FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE));

  free(dataset);
  free(labels);

  return 0;
}
