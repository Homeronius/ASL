#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "benchmark_util.h"

#define N 4'000

double *matrix;
double *vec;
double *result;


void fill_matrix(double * A, int n) {
    for(int i=0; i < n; i++) {
        for(int j=0; j < n; j++) {
            A[n*i+j] = (double) i / (j+1) + 1.0;
        }
    }
}

void fill_vector(double * x, int n) {
    for(int i=0; i < n; i++) {
        x[i] = (double) i + 1.0;
    }
}

void compute() {
  for (int i = 0; i < N; i++) {
    double res = result[i];
    for (int k = 0; k < N; k+=4) {
      res += vec[k+0] * matrix[i * N + k+0];
      res += vec[k+1] * matrix[i * N + k+1];
      res += vec[k+2] * matrix[i * N + k+2];
      res += vec[k+3] * matrix[i * N + k+3];
    }
    result[i] = res;
  }
}

void compute2() {
  for (int i = 0; i < N; i+=4) {
    result[i+0] = vec[i+0] * matrix[i+0];
    result[i+1] = vec[i+1] * matrix[i+1];
    result[i+2] = vec[i+2] * matrix[i+2];
    result[i+3] = vec[i+3] * matrix[i+3];
  }
}

long long measure_flops(unsigned long config) {
  int fd = start_flops_counter(config);
  compute2();
  return stop_flops_counter(fd);
}

int main(int argc, char **argv) {

  matrix = (double*) malloc(N * N * sizeof(double));
  vec = (double*) malloc(N * sizeof(double));
  result = (double*) calloc(N, sizeof(double));

  fill_matrix(matrix, N);
  fill_vector(vec, N);

  measure_and_print(&compute);

  printf("FLOPS count scalar double: %ld\n",
    measure_flops(FP_ARITH_INST_RETIRED_SCALAR_DOUBLE));
  printf("FLOPS count scalar float: %ld\n",
    measure_flops(FP_ARITH_INST_RETIRED_SCALAR_SINGLE));
  printf("FLOPS count 128 packed double: %ld\n",
    measure_flops(FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE));
  printf("FLOPS count 128 packed float: %ld\n",
    measure_flops(FP_ARITH_INST_RETIRED_128B_PACKED_SINGLE));
  printf("FLOPS count 256 packed double: %ld\n",
    measure_flops(FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE));
  printf("FLOPS count 256 packed float: %ld\n",
    measure_flops(FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE));
  
  free(matrix);
  free(vec);
  free(result);

  return 0;
}
