#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "distance.h"
#include "prim.h"
#include "gtest/gtest.h"

bool compare_arrays(const double *a, const double *b, int n) {
  for (int i = 0; i < n; i++) {
    if (fabs(a[i] - b[i]) > DBL_EPSILON)
      return false;
  }
  return true;
}

bool compare_edge_arrays(const edge *a, const edge *b, int n) {
  for (int i = 0; i < n; i++) {
    if (fabs(a[i].weight - b[i].weight) > DBL_EPSILON ||
        (a[i].u != b[i].u || a[i].v != b[i].v)) {
      std::cout << "Expected: {" << a[i].weight << ", " << a[i].u << ", "
                << a[i].v << "}\n";
      std::cout << "Got: {" << b[i].weight << ", " << b[i].u << ", " << b[i].v
                << "}\n";
      return false;
    }
  }
  return true;
}

double edge_sum(edge *a, int n) {
  double acc = 0;
  for (int i = 0; i < n; i++) {
    acc += a[i].weight;
  }
  return acc;
}

TEST(distance, distance_matrix_mpts2) {
  const int d = 4;
  const int n = 3;
  const int mpts = 2;
  double input[n * d] = {
      1.0, 2.0, 3.0, 4.0, 2.0, 3.0, 4.0, 5.0, 3.0, 4.0, 5.0, 6.0,
  };
  double dist[n * n];

  compute_distance_matrix(input, dist, mpts, n, d);

  const double expected[n * n] = {2.0, 2.0, 4.0, 2.0, 2.0, 2.0, 4.0, 2.0, 2.0};

  EXPECT_PRED3(compare_arrays, dist, expected, n * n);
}

TEST(distance, distance_matrix_mpts3) {
  const int d = 4;
  const int n = 3;
  const int mpts = 3;
  double input[n * d] = {
      1.0, 2.0, 3.0, 4.0, 2.0, 3.0, 4.0, 5.0, 3.0, 4.0, 5.0, 6.0,
  };
  double dist[n * n];

  compute_distance_matrix(input, dist, mpts, n, d);

  const double expected[n * n] = {4.0, 4.0, 4.0, 4.0, 2.0, 4.0, 4.0, 4.0, 4.0};

  EXPECT_PRED3(compare_arrays, dist, expected, n * n);
}

TEST(distance, quickselect1) {
  const int n = 4;
  double input[n] = {1.0, 2.0, 3.0, 4.0};

  double result = quickselect(input, 0, n - 1, 2);

  EXPECT_DOUBLE_EQ(result, 3.0);
}

TEST(distance, quickselect2) {
  const int n = 6;
  double input[n] = {7., 4., 6., 3., 9., 1.};

  double result = quickselect(input, 0, n - 1, 2);

  EXPECT_DOUBLE_EQ(result, 4.0);
}

TEST(distance, iterative_quickselect1) {
  const int n = 4;
  double input[n] = {1.0, 2.0, 3.0, 4.0};

  double result = iterative_quickselect(input, 0, n - 1, 2);

  EXPECT_DOUBLE_EQ(result, 3.0);
}

TEST(distance, iterative_quickselect2) {
  const int n = 6;
  double input[n] = {7., 4., 6., 3., 9., 1.};

  double result = iterative_quickselect(input, 0, n - 1, 2);

  EXPECT_DOUBLE_EQ(result, 4.0);
}

TEST(distance, core_distance) {
  const int d = 1;
  const int n = 4;
  const int mpts = 2;
  double input[n * d] = {1.0, 2.0, 3.0, 4.0};
  double core_dist[n];

  compute_core_distances(input, core_dist, mpts, n, d);

  const double expected[n] = {1.0, 1.0, 1.0, 1.0};

  EXPECT_PRED3(compare_arrays, core_dist, expected, n);
}

TEST(Prim, BasicGraph1) {
  double inf = DBL_MAX;
  const int n = 5;
  /* Example graph 1
   ---> consider it fully connected, edges not shown are set to
        infinity
      2     3
  (0)---(1)---(2)
  |    /  \    |
 6|  8/    \5  |7
  |  /      \  |
  (3)---------(4)
        9         */
  double adjacency[n * n] = {inf, 2.,  inf, 6.,  inf, 2., inf, 3., 8.,
                             5.,  inf, 3.,  inf, inf, 7., 6.,  8., inf,
                             inf, 9.,  inf, 5.,  7.,  9., inf};
  edge result[(n - 1)];
  prim(adjacency, result, n);

  // @TODO: is edge order unique according to prim
  // and starting with vertex 0?
  const edge expected[n - 1] = {
      {2., 0, 1},
      {3., 1, 2},
      {5., 1, 4},
      {6., 0, 3},
  };

  EXPECT_PRED3(compare_edge_arrays, expected, result, (n - 1));
}

TEST(Prim, BasicGraph2) {
  double inf = DBL_MAX;
  const int n = 4;
  /* Example graph 1
   ---> consider it fully connected, edges not shown are set to
        infinity
         1
  (0)---------(1)
  |  \_______/  |
 2|  __/   \__  |4
  | /1       1\ |
  (2)---------(3)
         3         */
  double adjacency[n * n] = {inf, 1., 2.,  1., 1., inf, 1., 4.,
                             2.,  1., inf, 3., 1., 4.,  3., inf};
  edge result[(n - 1)];
  prim(adjacency, result, n);

  // @TODO: is edge order unique according to prim
  // and starting with vertex 0? or just compare edge weight sum, if MST
  // is not unique
  const edge expected[n - 1] = {
      {1., 0, 1},
      {1., 1, 2},
      {1., 0, 3},
  };

  EXPECT_PRED3(compare_edge_arrays, expected, result, (n - 1));
}

TEST(AdvancedPrim, ComparisonRandom) {
  /* Compare the results of the basic prim algorithm
     to the output of the advanced version with
     the implicit mutual reachability graph */
  srand(42);
  const int n = 100;
  const int d = 10;
  const int mpts = 2;

  double input[n * d];
  for (int i = 0; i < n * d; i++) {
    // rand between -10 and 10
    input[i] = (double)rand() / RAND_MAX * 20.0 - 10.0;
  }
  double dist[n * n];
  double core_distances[n];

  compute_core_distances(input, core_distances, mpts, n, d);
  compute_distance_matrix(input, dist, mpts, n, d);

  edge result_basic[n - 1];
  prim(dist, result_basic, n);

  edge result_advanced[n - 1];
  prim_advanced(input, core_distances, result_advanced, n, d);

  // probably pick one of those, currently both fail
  EXPECT_TRUE(compare_edge_arrays(result_basic, result_advanced, (n - 1)));
  EXPECT_DOUBLE_EQ(edge_sum(result_basic, (n - 1)),
                   edge_sum(result_advanced, (n - 1)));
}