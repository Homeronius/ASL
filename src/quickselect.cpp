#include "quickselect.h"

// this function needs to be defined here to
// have no compile errors; correctly defined and used in vect. version
void BuildPackMask() {}

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

// only called with iterative_quickselect(list, n, mpts - 1);
// equivalent to iterative_quickselect(list, left = 0, right = n - 1, mpts - 1);
double iterative_quickselect(double *list, int n, int k) {
  int left = 0;
  int right = n - 1;
  while (true) {
    if (left == right) {
      return list[left];
    }

    // Random pivot
    // int pivot_idx = left + rand() % (right - left + 1);

    // Take element in the middle as pivot
    int pivot_idx = left + n / 2;

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