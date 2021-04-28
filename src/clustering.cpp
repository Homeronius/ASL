#include <stdlib.h>
#include<list>
#include "union_find.h"


std::list<Cluster> clustering(int* edgesA, int* edgesB, float_t* distances, size_t n, size_t number_of_edges){
    /*
    Parameters:
        edgesA: array of idxs of first node of the edges in mst.
        edgesB: array of idxs of second node of the edges in mst.
        weight: array of edges in mst.
        n: number of datapoints
        number_of_edges: number of edges in mst.
    Output:
        std:vector of clusters
    */
   int minimum_cluster_size = 10;
    Union_find heirarchy(n,10);
    
    for (size_t i = 0; i < number_of_edges; i++) {
        heirarchy.unify(edgesA[i],edgesB[i],distances[i]);
    }

    return heirarchy.get_clusters();
    
}