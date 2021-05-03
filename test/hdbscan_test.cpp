#include <float.h>
#include <math.h>
#include <prim.h>

#include "distance.h"
#include "gtest/gtest.h"

bool compare_arrays(double *a, double *b, int n) {
  for (int i = 0; i < n; i++) {
    if (fabs(a[i] - b[i]) > DBL_EPSILON)
      return false;
  }
  return true;
}

bool compare_edge_arrays(edge *a, edge *b, int n) {
    for (int i = 0; i < n; i++) {
        if (fabs(a[i].weight - b[i].weight) > DBL_EPSILON ||
            (a[i].u != b[i].u || a[i].v != b[i].v)) {
            std::cout << "Expected:" << a[i].weight << a[i].u << a[i].v << "\n";
            std::cout << "Got:" << b[i].weight << b[i].u << b[i].v << "\n";
            return false;
        }
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

  EXPECT_TRUE(compare_arrays(dist, expected, d));
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
    double adjacency[n * n] = {inf, 2., inf, 6., inf,
                               2., inf, 3., 8., 5.,
                               inf, 3., inf, inf, 7.,
                               6., 8., inf, inf, 9.,
                               inf, 5., 7., 9., inf};
    edge result[(n - 1)];
    prim(adjacency, result, n);

    // @TODO: is edge order unique according to prim
    // and starting with vertex 0?
    edge expected[(n - 1)] = {
        {2., 0, 1},
        {3., 1, 2},
        {5., 1, 4},
        {6., 0, 3},
    };

    EXPECT_TRUE(compare_edge_arrays(expected, result, (n - 1)));
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
    double adjacency[n * n] = {inf, 1., 2., 1.,
                               1., inf, 1., 4.,
                               2., 1., inf, 3.,
                               1., 4., 3., inf};
    edge result[(n - 1)];
    prim(adjacency, result, n);

    // @TODO: is edge order unique according to prim
    // and starting with vertex 0? or just compare edge weight sum, if MST
    // is not unique
    edge expected[(n - 1)] = {
        {1., 0, 1},
        {1., 1, 2},
        {1., 0, 3},
    };

    EXPECT_TRUE(compare_edge_arrays(expected, result, (n - 1)));
}
