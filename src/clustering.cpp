#include <stdlib.h>
#include<list>
#include "union_find.h"
#include <cstdio>


std::vector<Cluster> clustering(int* edgesA, int* edgesB, float_t* distances, size_t n, size_t minimum_cluster_size){
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

    Union_find heirarchy(n,minimum_cluster_size);

    for (size_t i = 0; i < n; i++) {
        printf("Unifying %i, %i,\t dist %lf\n",edgesA[i],edgesB[i], distances[i]);
        heirarchy.unify(edgesA[i],edgesB[i],distances[i]);
    }
    heirarchy.finalize();
    printf("Created %i cluster(s)\n", heirarchy.get_clusters().size());
    return heirarchy.get_clusters();

}

std::vector<Cluster> extract_clusters(std::vector<Cluster> clusters){
    std::vector<Cluster> selected_clusters;

    for (auto &&c : clusters){
        if (c.selected){
            Cluster* parent = c.parent;
            if (parent != nullptr && parent->get_cluster_weight() > parent->get_max_cluster_weight()){
                parent->selected = true;
                parent->child1->selected = false;
                parent->child2->selected = false;
            }
        }
    }
    for (auto &&c : clusters){
        if (c.selected){
            selected_clusters.push_back(c);
        }
    }
    return selected_clusters;
}


int* point_labels(std::vector<Cluster> selected_clusters, int number_of_points){
    int* labels = (int*) calloc(number_of_points,sizeof(int));
    for(int cluster_idx = 1; cluster_idx <= selected_clusters.size(); cluster_idx++){
        for (auto &&component : selected_clusters[cluster_idx-1].get_components()){
            labels[component] = cluster_idx;
        }
    }

    return labels;
}

