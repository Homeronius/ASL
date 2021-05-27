#include "prim.h"

#include <cstdint>
#include <float.h>
#include <immintrin.h>
#include <math.h>

#include "distance.h"

#include <cstdio>

// Version for 64-bit integers
static inline __m256i blendvpd_si256(__m256i a, __m256i b, __m256i mask) {
  __m256d res = _mm256_blendv_pd(_mm256_castsi256_pd(a), _mm256_castsi256_pd(b),
                                 _mm256_castsi256_pd(mask));
  return _mm256_castpd_si256(res);
}
/**
 * @brief Compute the minimum spanning tree (MST) of a given
 *        graph based on its adjacency matrix with corresponding
 *        edge weights. The graph is assumed to be undirected.
 *
 * @param adjacency the adjacency matrix of the graph [n,n]
 * @param result pointer to store result into
 * @param n number of nodes in the graph
 *
 */
void prim(double *adjacency, edge *result, int n) {
  // number of vectors
  int n_div4 = n / 4;
  // no. of remainder elements
  int n_remain = n - (n & ~0x3);
  // represent tree via each node's parent?
  __m256i parent[n_div4];
  // cheapest cost of edge to node
  __m256d cost[n_div4];
  // nodes contained in tree
  __m256i contained[n_div4];
  // arrays for storing the remaining elements
  uint64_t parent_remain[n_remain] __attribute__((aligned(32)));
  double cost_remain[n_remain] __attribute__((aligned(32)));
  bool contained_remain[n_remain];

  // all other costs initialized to dist to 0
  for (int k = 0; k < n_div4; ++k) {
    cost[k] = _mm256_loadu_pd(adjacency + 4 * k);
    contained[k] = _mm256_setzero_si256();
    parent[k] = _mm256_setzero_si256();
  }
  // Finish last initializations sequentially
  for (int k = 0; k < n_remain; ++k) {
    cost_remain[k] = adjacency[k + 4 * n_div4];
    contained_remain[k] = false;
    parent_remain[k] = 0;
  }

  // start from vertex 0 (there might be a better way?)
  cost[0][0] = 0.0;
  parent[0][0] = int(-1);
  contained[0][0] = uint(1);

  for (int iter = 0; iter < n - 1; iter++) {
    // i is the node we want to add
    uint64_t i;

    // TODO adapt idx to correct type
    // search for best node to add that
    // is not yet included in MST
    __m256d min_cost_reduction = _mm256_set1_pd(__DBL_MAX__);
    __m256i min_cost_idx = _mm256_setzero_si256();
    __m256i curr_idx_vec = _mm256_setr_epi64x(0, 1, 2, 3);
    __m256i step_idx_vec = _mm256_set1_epi64x(4);

    for (int j = 0; j < n_div4; ++j) {
      __m256d cost_mask =
          _mm256_cmp_pd(cost[j], min_cost_reduction, _CMP_LT_OS);
      __m256i condition =
          _mm256_andnot_si256(contained[j], _mm256_castpd_si256(cost_mask));

      // blend min and argmin into result
      min_cost_reduction = _mm256_blendv_pd(min_cost_reduction, cost[j],
                                            _mm256_castsi256_pd(condition));
      min_cost_idx = blendvpd_si256(min_cost_idx, curr_idx_vec, condition);

      // increment idx_vec
      curr_idx_vec = _mm256_add_epi64(curr_idx_vec, step_idx_vec);
    }

    // Sequentially determine min and argmin of vectorized reduction
    double min = __DBL_MAX__;
    uint64_t min_cost_idx_local[4] __attribute__((aligned(32)));
    double min_cost_reduction_local[4] __attribute__((aligned(32)));
    _mm256_store_si256((__m256i *)(min_cost_idx_local), min_cost_idx);
    _mm256_store_pd(min_cost_reduction_local, min_cost_reduction);

    for (int o = 0; o < 4; ++o) {
      if (min_cost_reduction_local[o] < min) {
        min = min_cost_reduction_local[o];
        i = min_cost_idx_local[o];
      }
    }

    // Finish up the loop
    for (int k = 0; k < n_remain; ++k) {
      if (contained_remain[k] == false && cost_remain[k] < min) {
        min = cost_remain[k];
        i = k + 4 * n_div4;
      }
    }

    // add i to mst
    uint64_t offset = i / 4;
    uint64_t rel_pos = i % 4;
    if (offset < n_div4) { // should be stored in instrinsics vector
      // TODO store could be improved with _mm256_maskstore_xx instructions
      // TODO remove narrowing conversion to int by setting attributes in edge
      //      struct to uint64_t
      contained[offset][rel_pos] = 0xFFFFFFFFFFFFFFFF;
      result[iter] = {min, int(parent[offset][rel_pos]), int(i)};
    } else { // contained in remainder array
      contained_remain[rel_pos] = true;
      result[iter] = {min, int(parent_remain[rel_pos]), int(i)};
    }
    // update the cost values of the nodes that
    // are not yet in mst with possible lower distance to i

    // Determine if element should be updated or not
    for (int j = 0; j < n_div4; ++j) {
      __m256d adj_vec = _mm256_loadu_pd(adjacency + i * n + (4 * j));
      __m256d cost_mask = _mm256_cmp_pd(adj_vec, cost[j], _CMP_LT_OS);
      __m256i condition =
          _mm256_andnot_si256(contained[j], _mm256_castpd_si256(cost_mask));

      // Update corresponding values
      //_mm256_maskstore_epi32(parent + j, condition, _mm256_set1_epi32(i));
      parent[j] = blendvpd_si256(_mm256_set1_epi64x(i), parent[j], condition);
      //_mm256_maskstore_pd((cost + j), condition, adj_vec);
      cost[j] =
          _mm256_blendv_pd(adj_vec, cost[j], _mm256_castsi256_pd(condition));
    }

    // Finish up the loop
    for (int j = 0; j < n_remain; j++)
      if (contained_remain[j] == false &&
          adjacency[i * n + (4 * n_div4 + j)] < cost_remain[j]) {
        parent_remain[j] = int(i);
        cost_remain[j] = adjacency[i * n + (4 * n_div4 + j)];
      }
  }
}

void prim_advanced(double *X, double *core_distances, edge *result, int n,
                   int d) {
#ifdef HDBSCAN_VERBOSE
  printf("Calling vectorized version\n");
#endif
  // represent tree via each node's parent?
  int parent[n];
  // current minimal distance found to node
  double cost[n];
  // nodes contained in tree
  bool contained[n];

  for (int i = 0; i < n; i++) {
    cost[i] = __DBL_MAX__;
    contained[i] = false;
  }

  double curr_min_cost_j, dist_between;

  __m256d core_dist_i_vec, core_dist_j_vec, dist_between_vec,
      mutual_reach_dist_vec;

  double core_dist_j_local[4] __attribute__((aligned(32)));
  int idx_j_local[4] __attribute__((aligned(16)));
  double dist_between_local[4] __attribute__((aligned(32)));
  double mutual_reach_dist_local[4] __attribute__((aligned(32)));

  // start from vertex 0
  int i = 0;
  cost[i] = 0.0;
  parent[i] = -1;
  contained[i] = true;
  for (int iter = 0; iter < n - 1; iter++) {

    // i is the node we want to add
    int new_node, j, k, idx, mrt_idx;

    core_dist_i_vec = _mm256_set1_pd(core_distances[i]);
    double min_cost = __DBL_MAX__;
    new_node = 0;

    // search for best node to add which
    // is not yet included in MST
    // and update costs simultaneously
    j = 0;
    mrt_idx = 0;
    // TODO do last iterations sequentially
    for (; j < (n & ~0x3); ++j) {
      // collect 4 points which aren't yet part of the MST
      if (mrt_idx == 0) {
        idx = j;
        k = 0;
        while (k < 4) {
          if (!contained[idx]) {
            idx_j_local[k] = idx;
            core_dist_j_local[k] = core_distances[idx];
            ++k;
          }
          ++idx;
        }

        core_dist_j_vec = _mm256_load_pd(core_dist_j_local);

        if (d == 2) {
          dist_between_vec = euclidean_distance_2_ret_simd(
              X + i * d, X + idx_j_local[0] * d, X + i * d,
              X + idx_j_local[1] * d, X + i * d, X + idx_j_local[2] * d,
              X + i * d, X + idx_j_local[3] * d);
        } else if (d == 4) {
          dist_between_vec = euclidean_distance_4_opt_ret_simd(
              X + i * d, X + idx_j_local[0] * d, X + idx_j_local[1] * d,
              X + idx_j_local[2] * d, X + idx_j_local[3] * d);
        } else {
          for (k = 0; k < 4; ++k) {
            dist_between_local[k] =
                euclidean_distance(X + i * d, X + idx_j_local[k] * d, d);
          }
        }

        mutual_reach_dist_vec = _mm256_max_pd(
            core_dist_i_vec, _mm256_max_pd(core_dist_j_vec, dist_between_vec));

        _mm256_store_pd(mutual_reach_dist_local, mutual_reach_dist_vec);
      }

      if (contained[j]) {
        continue;
      }
      curr_min_cost_j = cost[j];

      if (mutual_reach_dist_local[mrt_idx] < curr_min_cost_j) {
        parent[j] = i;
        cost[j] = mutual_reach_dist_local[mrt_idx];
        if (mutual_reach_dist_local[mrt_idx] < min_cost) {
          min_cost = mutual_reach_dist_local[mrt_idx];
          new_node = j;
        }
      } else {
        if (curr_min_cost_j < min_cost) {
          min_cost = curr_min_cost_j;
          new_node = j;
        }
      }
      mrt_idx = mrt_idx < 3 ? mrt_idx + 1 : 0;
    }

    // Finish last iterations sequentially
    for (j = (n & ~0x3); j < n; j++) {
      if (contained[j]) {
        continue;
      }

      curr_min_cost_j = cost[j];
      double core_dist_j = core_distances[j];
      // @TODO different distance funcs?
      dist_between = euclidean_distance(X + i * d, X + j * d, d);

      double mutual_reach_dist =
          fmax(core_distances[i], fmax(core_dist_j, dist_between));

      if (mutual_reach_dist < curr_min_cost_j) {
        parent[j] = i;
        cost[j] = mutual_reach_dist;
        if (mutual_reach_dist < min_cost) {
          min_cost = mutual_reach_dist;
          new_node = j;
        }
      } else {
        if (curr_min_cost_j < min_cost) {
          min_cost = curr_min_cost_j;
          new_node = j;
        }
      }
      // printf("at node %i min_cost: %f\n", j, min_cost);
    }

    i = new_node;

    // add i to MST
    contained[i] = true;
    result[iter] = {min_cost, parent[i], i};
  }
}

/**
 * @brief Compute the minimum spanning tree (MST) of a given
 *        graph based on its __upper triangular__
 *        adjacency matrix with corresponding
 *        edge weights. The graph is assumed to be undirected.
 *
 * @param adjacency the upper triangular of the adjacency matrix of the graph,
 * size n*(n+1)/2 access translation is: Matrix[i][j] -> adjacency[(n * i - (i *
 * (i + 1) / 2)) + j]
 * @param result pointer to store result into
 * @param n number of nodes in the graph
 *
 */
void prim_diag(double *adjacency, edge *result, int n) {
  // represent tree via each node's parent?
  int parent[n];
  // cheapest cost of edge to node
  double cost[n];
  // nodes contained in tree
  bool contained[n];

  // start from vertex 0
  cost[0] = 0.0;
  parent[0] = -1;
  contained[0] = true;

  // all other costs initialized to dist to 0
  for (int i = 1; i < n; i++) {
    cost[i] = adjacency[i];
    contained[i] = false;
    parent[i] = 0;
  }

  for (int iter = 0; iter < n - 1; iter++) {
    // i is the node we want to add
    int i, j;

    // search for best node to add that
    // is not yet included in MST
    double min = __DBL_MAX__;
    for (j = 0; j < n; j++)
      if (contained[j] == false && cost[j] < min)
        min = cost[j], i = j;

    // add i to MST
    contained[i] = true;
    result[iter] = {min, parent[i], i};
    // update the cost values of the nodes that
    // are not yet in MST with possible lower distance to i
    for (j = 0; j < n; j++) {
      int u, v;
      // use smaller index as u to only access upper triangular
      if (i < j)
        u = i, v = j;
      else
        u = j, v = i;
      if (contained[j] == false &&
          adjacency[(n * u - (u * (u + 1) / 2)) + v] < cost[j])
        parent[j] = i, cost[j] = adjacency[(n * u - (u * (u + 1) / 2)) + v];
    }
  }
}

int compare_edges(const void *e1, const void *e2) {
  double w_diff = ((edge *)e1)->weight - ((edge *)e2)->weight;
  if (w_diff < 0) {
    return -1;
  } else if (w_diff > 0) {
    return 1;
  } else {
    return 0;
  }
}
