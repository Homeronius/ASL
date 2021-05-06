#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "benchmark_util.h"
#include "hdbscan.h"

#define N 4'000

static double *matrix;
static double *vec;
static double *result;

static char *dataset_path;
static double *dataset;
static int *labels;
static int n;
static int d;
static HDBSCAN *clusterer;

void fill_matrix(double *A, int n) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      A[n * i + j] = (double)i / (j + 1) + 1.0;
    }
  }
}

void fill_vector(double *x, int n) {
  for (int i = 0; i < n; i++) {
    x[i] = (double)i + 1.0;
  }
}

void compute() {
  for (int i = 0; i < N; i++) {
    double res = result[i];
    for (int k = 0; k < N; k += 4) {
      res += vec[k + 0] * matrix[i * N + k + 0];
      res += vec[k + 1] * matrix[i * N + k + 1];
      res += vec[k + 2] * matrix[i * N + k + 2];
      res += vec[k + 3] * matrix[i * N + k + 3];
    }
    result[i] = res;
  }
}

void compute2() {
  for (int i = 0; i < N; i += 4) {
    result[i + 0] = vec[i + 0] * matrix[i + 0];
    result[i + 1] = vec[i + 1] * matrix[i + 1];
    result[i + 2] = vec[i + 2] * matrix[i + 2];
    result[i + 3] = vec[i + 3] * matrix[i + 3];
  }
}

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
  // compute2();
  compute_hdbscan();
  long long count = stop_flops_counter(fd);

  return count;
}

int main(int argc, char **argv) {

  // matrix = (double *)malloc(N * N * sizeof(double));
  // vec = (double *)malloc(N * sizeof(double));
  // result = (double *)calloc(N, sizeof(double));

  // fill_matrix(matrix, N);
  // fill_vector(vec, N);

  if (argc != 2) {
    printf("Usage: hdbscan_benchmark <input_path>\n");
    return -1;
  }

  dataset_path = argv[1];

  load_dataset(dataset_path, &dataset, &labels, &n, &d);

  measure_and_print(&compute_hdbscan);

  // measure_and_print(&compute);

  printf("FLOPS count scalar double: %lld\n",
         measure_flops(FP_ARITH_INST_RETIRED_SCALAR_DOUBLE));
  printf("FLOPS count scalar float: %lld\n",
         measure_flops(FP_ARITH_INST_RETIRED_SCALAR_SINGLE));
  printf("FLOPS count 128 packed double: %lld\n",
         measure_flops(FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE));
  printf("FLOPS count 128 packed float: %lld\n",
         measure_flops(FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE));
  printf("FLOPS count 256 packed double: %lld\n",
         measure_flops(FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE));
  printf("FLOPS count 256 packed float: %lld\n",
         measure_flops(FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE));

  free(dataset);
  free(labels);

  // free(matrix);
  // free(vec);
  // free(result);

  return 0;
}
