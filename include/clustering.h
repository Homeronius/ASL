#ifndef _HDBSCAN_CLUSTERING_H_
#define _HDBSCAN_CLUSTERING_H_

#include<list>
#include"union_find.h"
/**
 * @brief builds condensed cluster tree
 * 
 * @param edgesA first vertex of edge in mst
 * @param edgesB second vertex of edge in mst
 * @param distances mutual_reachability distance between verticies
 * @param n number of nodes in mst
 * @param number_of_edges number of edges in mst
 * @return std::list<Cluster> returns list of clusters, which represent condensed cluster tree
 */
std::vector<Cluster*> clustering(int* edgesA, int* edgesB, float_t* distances, size_t n, size_t minimum_cluster_size);

std::vector<Cluster*> extract_clusters(std::vector<Cluster*> clusters);

int* point_labels(std::vector<Cluster*> selected_clusters, int number_of_points);

#endif //_HDBSCAN_CLUSTERING_H_vector<Cluster>