#include <floats.h>

/*
 * Function:  prim 
 * ----------------
 * Computes the minimum spanning tree (MST) of a
 * given (undirected + complete) graph based on 
 * its adjacency matrix that contains the weights of the edges.
 *  
 *  adjacency: the adjacency matrix, n x n
 *  result: (@TODO specify result format we want)
 *          the location where to store the result into 
 *  n: number of nodes in the graph
 *  
 */
void prim(double *adjacency, int *result, int n) {
    // represent tree via each node's parent?
    int parent[n];
    // cheapest cost of edge to node
    double cost[n];
    // nodes contained in tree
    bool contained[n];

    for (int i = 0; i < n; i++) {
        cost[i] = __DBL_MAX__;
        contained[i] = false;
    }

    // start from vertex 0
    cost[0] = 0.0;
    parent[0] = -1;

    for (int iter = 0; iter < n; iter++) {
        // i is the node we want to add
        int i, j;

        // search for best node to add that
        // is not yet included in MST
        double min = __DBL_MAX__;
        for (j = 0; j < n; j++)
            if (contained[j] == false && cost[j] < min)
                min = cost[v], i = j;

        // add i to MST
        contained[i] = true;

        // update the cost values of the nodes that
        // are not yet in MST and adjacent to i
        for (j = 0; j < n; j++)
            if (contained[j] == false && adjacency[i * n + j] < cost[i])
                parent[j] = i, cost[v] = adjacency[i * n + j];
    }
}