#include "distance.h"
#include "quickselect.h"
#include <math.h>
#include <stdlib.h>

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

inline double euclidean_distance_2(double *p1, double *p2, int d) {
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

  for (int k = 0; k < n; k++) {
    for (int i = 0; i < n; i++) {
      distances[i] = euclidean_distance(input + i * d, input + k * d, d);
    }

    core_dist[k] = iterative_quickselect(distances, n, mpts - 1);
  }
}

void compute_distance_matrix(double *input, double *core_dist, double *dist,
                             int mpts, int n, int d) {
  double tmp[n];
  for (int i = 0; i < n; i++) {
    for (int k = 0; k < n; k++) {
      tmp[k] = euclidean_distance(input + i * d, input + k * d, d);
      dist[i * n + k] = tmp[k];
    }
    core_dist[i] = iterative_quickselect(tmp, n, mpts - 1);
  }

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
      tmp[k] = euclidean_distance(input + i * d, input + k * d, d);
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
  int CACHE_SIZE = 4096; // size in double. assume cache size 32KiB
  int block_size = floor(sqrt(4 * d * d - 4 * CACHE_SIZE) / 2 -
                         d); // blcok*2 + 2*d*block - Cache = 0

  int i_block = 0;
  int k_block = 0;

  for (; i_block < n - block_size; i_block += block_size) {
    for (k_block = 0; k_block < n - block_size; k_block += block_size) {
      for (int i = i_block; i < i_block + block_size; i++) {
        for (int k = k_block; k < k_block + block_size; k++) {
          dist[i * n + k] +=
              euclidean_distance_2(input + i * d, input + k * d, d);
        }
      }
    }
    for (int i = i_block; i < i_block + block_size; i++) {
      for (int k = k_block; k < n; k++) {
        dist[i * n + k] +=
            euclidean_distance_2(input + i * d, input + k * d, d);
      }
    }
  }
  for (k_block = 0; k_block < n - block_size; k_block += block_size) {
    for (int i = i_block; i < n; i++) {
      for (int k = k_block; k < k_block + block_size; k++) {
        dist[i * n + k] +=
            euclidean_distance_2(input + i * d, input + k * d, d);
      }
    }
  }
  for (int i = i_block; i < n; i++) {
    for (int k = k_block; k < n; k++) {
      dist[i * n + k] += euclidean_distance_2(input + i * d, input + k * d, d);
    }
  }

  double tmp[n];
  for (int i = 0; i < n; i++) {
    for (int k = 0; k < n; k++) {
      dist[i * n + k] = sqrt(dist[i * n + k]);
      tmp[i * n + k] = dist[i * n + k];
    }
    core_dist[i] = iterative_quickselect(tmp, n, mpts - 1);
    for (int k = 0; k < i; k++) {
      dist[i * n + k] = fmax(fmax(core_dist[i], core_dist[k]), dist[i * n + k]);
    }
  }
}
