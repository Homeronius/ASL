#include "gtest/gtest.h"
#include "distance.h"
#include <math.h>
#include <float.h>

bool compare_arrays(double *a, double *b, int n) {
  for (int i = 0; i < n; i++) {
    if (fabs(a[i] - b[i]) > DBL_EPSILON)
      return false;
  }
  return true;
}

TEST(Distance, Test) {
  const int d = 4;
  const int n = 2;
  double input[n*d] = {1.0, 2.0, 3.0, 4.0, 2.0, 3.0, 4.0, 5.0};
  double dist[n*n];

  compute_distance_matrix(input, dist, 2, 2, 4);

  double expected[n*n] = {0.0, 2.0, 2.0, 0.0};

  EXPECT_TRUE(compare_arrays(dist, expected, 4));
}
