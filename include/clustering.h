#ifndef _HDBSCAN_CLUSTERING_H_
#define _HDBSCAN_CLUSTERING_H_

#include<list>
#include"union_find.h"
std::list<Cluster> clustering(int* edgesA, int* edgesB, float_t distances, size_t n, size_t number_of_edges);

#endif //_HDBSCAN_CLUSTERING_H_