#include <stdlib.h>
#include<list>
#include "union_find.h"
#include <cstdio>


std::vector<Cluster*> clustering(int* edgesA, int* edgesB, float_t* distances, size_t n, size_t minimum_cluster_size){
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
        // printf("Unifying %i, %i,\t dist %lf\n",edgesA[i],edgesB[i], distances[i]);
        heirarchy.unify(edgesA[i],edgesB[i],distances[i]);
    }
    heirarchy.finalize();
    printf("Created %i cluster(s)\n", heirarchy.get_clusters().size());
    return heirarchy.get_clusters();

}



void get_preorder(std::vector<Cluster*>& ordered_clusters,Cluster* c){
    if(c == nullptr ) {
        // printf("null, returning\n");
        return;
    };
    ordered_clusters.push_back(c);
    // printf("Call %d, ptr %p with children: %p,%p\n",ordered_clusters.size(),c,c->child1,c->child2);
    get_preorder(ordered_clusters,c->child1);
    get_preorder(ordered_clusters,c->child2);
    return;
}

std::vector<Cluster*> extract_clusters(std::vector<Cluster*> ordered_clusters){
    std::vector<Cluster*> selected_clusters;

    for (int i = ordered_clusters.size()-1; i >=0; i--)
    {
        if(ordered_clusters[i]->selected){
            Cluster* parent = ordered_clusters[i]->parent;
            if (parent != nullptr ){
                double current_weight = parent->get_cluster_weight();
                double children_weight = parent->get_children_cluster_weight();
                if(current_weight > children_weight){
                    parent->selected = true;
                    parent->unselect_children();
                }
            }
        }
    }
    
    
    // for (auto &&c : clusters){
    //     if (c.selected){
    //         Cluster* parent = c.parent;

    //         if (parent != nullptr ){
    //             double current_weight = parent->get_cluster_weight();
    //             double children_weight = parent->get_children_cluster_weight();
    //             if(current_weight > children_weight){
    //                 parent->selected = true;
    //                 parent->child1->selected = false;#include <string>

    //         }
    //     }
    // }
    for (auto &&c : ordered_clusters){
        if (c->selected){
            selected_clusters.push_back(c);
        }
    }
    return selected_clusters;
}


void point_labels(std::vector<Cluster*> selected_clusters, int number_of_points, int* labels){

    for(int cluster_idx = 0; cluster_idx < selected_clusters.size(); cluster_idx++){
        for (auto &&component : selected_clusters[cluster_idx]->get_components()){
            labels[component] = cluster_idx+1;
            printf("Setting %d to %d\n",component,cluster_idx+1);
        }
    }
}