#include "distance.h"
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

double manhattan_distance(double *p1, double *p2, int d) {
  double sum = 0.0;

  for (int i = 0; i < d; i++) {
    double diff = p1[i] - p2[i];
    sum += fabs(diff);
  }

  return sum;
}

int partition(double *list, int left, int right, int pivot) {
  double pivot_val = list[pivot];
  list[pivot] = list[right];
  list[right] = pivot_val;

  int store_idx = left;

  for (int i = left; i < right; i++) {
    if (list[i] < pivot_val) {
      double tmp = list[store_idx];
      list[store_idx] = list[i];
      list[i] = tmp;
      store_idx++;
    }
  }

  double t = list[right];
  list[right] = list[store_idx];
  list[store_idx] = t;

  return store_idx;
}

double quickselect(double *list, int left, int right, int k) {
  if (left == right) {
    return list[left];
  }

  // Random pivot
  // int pivot = left + rand() % (right - left + 1);

  // Take element in the middle as pivot
  int pivot = left + (right - left) / 2;
  pivot = partition(list, left, right, pivot);

  if (k == pivot) {
    return list[k];
  } else if (k < pivot) {
    return quickselect(list, left, pivot - 1, k);
  } else {
    return quickselect(list, pivot + 1, right, k);
  }
}

double iterative_quickselect(double *list, int left, int right, int k) {
  while (true) {
    if (left == right) {
      return list[left];
    }

    // Random pivot
    // int pivot_idx = left + rand() % (right - left + 1);

    // Take element in the middle as pivot
    int pivot_idx = left + (right - left) / 2;

    pivot_idx = partition(list, left, right, pivot_idx);

    if (k == pivot_idx) {
      return list[k];
    } else if (k < pivot_idx) {
      right = pivot_idx - 1;
    } else {
      left = pivot_idx + 1;
    }
  }
}

void compute_core_distances(double *input, double *core_dist, int mpts, int n,
                            int d) {
  double distances[n];

  for (int k = 0; k < n; k++) {
    for (int i = 0; i < n; i++) {
      distances[i] = euclidean_distance(input + i * d, input + k * d, d);
    }

    core_dist[k] = iterative_quickselect(distances, 0, n - 1, mpts - 1);
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
    core_dist[i] = iterative_quickselect(tmp, 0, n - 1, mpts - 1);
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
    core_dist[i] = iterative_quickselect(tmp, 0, n - 1, mpts - 1);
  }

  for (int i = 0; i < n; i++) {
    for (int k = i; k < n; k++) {
      dist[(n * i - (i * (i + 1) / 2)) + k] =
          fmax(fmax(core_dist[i], core_dist[k]),
               dist[(n * i - (i * (i + 1) / 2)) + k]);
    }
  }
}
