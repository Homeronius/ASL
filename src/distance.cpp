#include "distance.h"
#include <math.h>
#include <stdlib.h>

double euclidean_distance(double *p1, double *p2, int d) {
  double sum = 0.0;

  for (int i = 0; i < d; i++) {
    double diff = p1[i] - p2[i];
    sum += diff * diff;
  }

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

  int pivot = left + rand() % (right - left + 1);
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
    int pivot_idx = left + rand() % (right - left + 1);

    // Take element in the middle as pivot
    // int pivot_idx = left + (right - left) / 2;

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

void compute_distance_matrix(double *input, double *dist, int mpts, int n,
                             int d) {
  double tmp[n * n];

  for (int i = 0; i < n; i++) {
    for (int k = 0; k < n; k++) {
      tmp[i * n + k] = euclidean_distance(input + i * d, input + k * d, d);
      dist[i * n + k] = tmp[i * n + k];
    }
  }

  // d_mreach(p1, p2) = max(d_core(p1), d_core(p2), euclidean_distance(p1, p2))
  for (int i = 0; i < n; i++) {
    quickselect(tmp + i * n, 0, n - 1, mpts - 1);
  }

  for (int i = 0; i < n; i++) {
    for (int k = 0; k < n; k++) {
      dist[i * n + k] = fmax(fmax(tmp[i * n + mpts - 1], tmp[k * n + mpts - 1]),
                             dist[i * n + k]);
    }
  }
}
