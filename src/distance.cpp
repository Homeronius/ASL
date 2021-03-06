#include "distance.h"
#include "quickselect.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef FINE_GRAINED_BENCH
#include "benchmark_util.h"
#endif

#ifdef HDBSCAN_INSTRUMENT
extern long hdbscan_sqrt_counter;
#endif

double euclidean_distance(double *p1, double *p2, int d) {
  double sum = 0.0;

  for (int i = 0; i < d; i++) {
    double diff = p1[i] - p2[i];
    sum += diff * diff;
  }

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter++;
#endif

  return sqrt(sum);
}

inline double euclidean_distance_squared(double *p1, double *p2, int d) {
  double sum = 0.0;

  for (int i = 0; i < d; i++) {
    double diff = p1[i] - p2[i];
    sum += diff * diff;
  }

  return sum;
}

double manhattan_distance(double *p1, double *p2, int d) {
  double sum = 0.0;

  for (int i = 0; i < d; i++) {
    double diff = p1[i] - p2[i];
    sum += fabs(diff);
  }

  return sum;
}

void compute_core_distances(double *input, double *core_dist, int mpts, int n,
                            int d) {
  double distances[n];

#ifdef FINE_GRAINED_BENCH
  struct measurement_data_t mdata;
  init_measurement(&mdata);
#endif

  for (int k = 0; k < n; k++) {
#ifdef FINE_GRAINED_BENCH
    auto compute = [&]() {
#endif
      for (int i = 0; i < n; i++) {
#ifdef HDBSCAN_MANHATTAN
        distances[i] = manhattan_distance(input + i * d, input + k * d, d);
#else
      distances[i] = euclidean_distance(input + i * d, input + k * d, d);
#endif
      }
#ifdef FINE_GRAINED_BENCH
    };
    add_measurement(&mdata, compute);
#endif

    core_dist[k] = iterative_quickselect(distances, n, mpts - 1);
  }

#ifdef FINE_GRAINED_BENCH
  print_measurement(&mdata, "(distance)");
#endif
}

void compute_distance_matrix(double *input, double *core_dist, double *dist,
                             int mpts, int n, int d) {
  double tmp[n];

#ifdef FINE_GRAINED_BENCH
  struct measurement_data_t mdata;
  init_measurement(&mdata);
#endif

  for (int i = 0; i < n; i++) {
#ifdef FINE_GRAINED_BENCH
    auto compute = [&]() {
#endif
      for (int k = 0; k < n; k++) {
#ifdef HDBSCAN_MANHATTAN
        tmp[k] = manhattan_distance(input + i * d, input + k * d, d);
#else
      tmp[k] = euclidean_distance(input + i * d, input + k * d, d);
#endif
        dist[i * n + k] = tmp[k];
      }
#ifdef FINE_GRAINED_BENCH
    };
    add_measurement(&mdata, compute);
#endif
    core_dist[i] = iterative_quickselect(tmp, n, mpts - 1);
  }

#ifdef FINE_GRAINED_BENCH
  print_measurement(&mdata, "(distance)");
#endif

  for (int i = 0; i < n; i++) {
    for (int k = 0; k < n; k++) {
      dist[i * n + k] = fmax(fmax(core_dist[i], core_dist[k]), dist[i * n + k]);
    }
  }
}

void compute_distance_matrix_triang(double *input, double *core_dist,
                                    double *dist, int mpts, int n, int d) {
  double tmp[n];
  // idea: dist matrix is of size (n*1)*n / 2 (upper triangular)
  // - the translation then is
  //   pairwise_dist[i][k] = dist[(n * i - (i * (i + 1)/2)) + k]
  // - invariant: i <= k
  // !! verify this yourself !!
  for (int i = 0; i < n; i++) {
    int k;
    /* comment: this here seems to be the reason why the triangular
       version is much slower than the naive full one.
       most of these reads end up being cache misses and take down
       the performance by a lot
    */
    for (k = 0; k < i; k++)
      tmp[k] = dist[(n * k - (k * (k + 1) / 2)) + i];
    // dist to self is zero
    dist[(n * i - (i * (i + 1) / 2)) + i] = 0;
    tmp[i] = 0;
    for (k = i + 1; k < n; k++) {
#ifdef HDBSCAN_MANHATTAN
      tmp[k] = manhattan_distance(input + i * d, input + k * d, d);
#else
      tmp[k] = euclidean_distance(input + i * d, input + k * d, d);
#endif
      dist[(n * i - (i * (i + 1) / 2)) + k] = tmp[k];
    }
    core_dist[i] = iterative_quickselect(tmp, n, mpts - 1);
  }

  for (int i = 0; i < n; i++) {
    for (int k = i; k < n; k++) {
      dist[(n * i - (i * (i + 1) / 2)) + k] =
          fmax(fmax(core_dist[i], core_dist[k]),
               dist[(n * i - (i * (i + 1) / 2)) + k]);
    }
  }
}

void compute_distance_matrix_blocked(double *input, double *core_dist,
                                     double *dist, int mpts, int n, int d) {

#ifndef HDBSCAN_BLOCK_SIZE
  int CACHE_SIZE = 4096; // size in double. assume cache size 32KiB
  int block_size =
      (int)(sqrt(d * d + CACHE_SIZE) - d); // blcok*2 + 2*d*block - Cache = 0
#else
  int block_size = HDBSCAN_BLOCK_SIZE;
#endif

  // printf("Running Blocked Computation: Cache = %d, dim = %d, Blocksize = %d,
  // n=%d\n", CACHE_SIZE, d, block_size, n);
  int i_block = 0;
  int k_block = 0;
  for (; i_block < n - block_size; i_block += block_size) {
    for (k_block = 0; k_block < n - block_size; k_block += block_size) {
      for (int i = i_block; i < i_block + block_size; i++) {
        for (int k = k_block; k < k_block + block_size; k++) {
          dist[i * n + k] +=
#ifdef HDBSCAN_MANHATTAN
              manhattan_distance(input + i * d, input + k * d, d);
#else
              euclidean_distance_squared(input + i * d, input + k * d, d);
#endif
        }
      }
    }
    for (int i = i_block; i < i_block + block_size; i++) {
      for (int k = k_block; k < n; k++) {
        dist[i * n + k] +=
#ifdef HDBSCAN_MANHATTAN
            manhattan_distance(input + i * d, input + k * d, d);
#else
            euclidean_distance_squared(input + i * d, input + k * d, d);
#endif
      }
    }
  }
  for (k_block = 0; k_block < n - block_size; k_block += block_size) {
    for (int i = i_block; i < n; i++) {
      for (int k = k_block; k < k_block + block_size; k++) {
        dist[i * n + k] +=
#ifdef HDBSCAN_MANHATTAN
            manhattan_distance(input + i * d, input + k * d, d);
#else
            euclidean_distance_squared(input + i * d, input + k * d, d);
#endif
      }
    }
  }
  for (int i = i_block; i < n; i++) {
    for (int k = k_block; k < n; k++) {
      dist[i * n + k] +=
#ifdef HDBSCAN_MANHATTAN
          manhattan_distance(input + i * d, input + k * d, d);
#else
          euclidean_distance_squared(input + i * d, input + k * d, d);
#endif
    }
  }
  double *tmp = static_cast<double *>(malloc(n * n * sizeof(double)));
  ;
  for (int i = 0; i < n; i++) {
    for (int k = 0; k < n; k++) {
#ifndef HDBSCAN_MANHATTAN
      dist[i * n + k] = sqrt(dist[i * n + k]);
#endif
      tmp[i * n + k] = dist[i * n + k];
    }
    core_dist[i] = iterative_quickselect(tmp, n, mpts - 1);
    for (int k = 0; k < i; k++) {
      dist[i * n + k] = fmax(fmax(core_dist[i], core_dist[k]), dist[i * n + k]);
    }
  }
}
