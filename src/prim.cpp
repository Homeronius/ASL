#include "prim.h"

#include <float.h>
#include <math.h>

#include "distance.h"

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
    for (j = 0; j < n; j++)
      if (contained[j] == false && adjacency[i * n + j] < cost[j])
        parent[j] = i, cost[j] = adjacency[i * n + j];
  }
}

/**
 * @brief Compute the minimum spanning tree (MST) of the
 *        mutual reachabilty graph, given implicitly by the
 *        datapoints and the core distances.
 * 
 * @param X the input data of shape [n,d]
 * @param core_distances array of core_distance for each sample [n]
 * @param result pointer to store result into
 * @param n number of nodes in the graph
 * @param d the dimensionality of the data
 * 
 */
void prim_advanced(double *X, double *core_distances, edge *result, int n, int d) {
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

  double min_cost, core_dist_i, core_dist_j,
      curr_min_cost_j, dist_between, mutual_reach_dist;

  // start from vertex 0
  int i = 0;
  cost[i] = 0.0;
  parent[i] = -1;
  contained[i] = true;
  for (int iter = 0; iter < n - 1; iter++) {
    // i is the node we want to add
    int new_node, j;

    core_dist_i = core_distances[i];
    min_cost = __DBL_MAX__;
    new_node = 0;

    // search for best node to add which
    // is not yet included in MST
    // and update costs simultaneously
    for (j = 0; j < n; j++) {
      if (contained[j])
        continue;

      curr_min_cost_j = cost[j];
      core_dist_j = core_distances[j];
      // @TODO different distance funcs?
      dist_between = euclidean_distance(X + i * d, X + j * d, d);

      mutual_reach_dist = fmax(core_dist_i, fmax(core_dist_j, dist_between));

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
    }

    i = new_node;

    // add i to MST
    contained[i] = true;
    result[iter] = {min_cost, parent[i], i};
  }
}