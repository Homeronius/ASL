#ifndef _HDBSCAN_UNION_FIND_H_
#define _HDBSCAN_UNION_FIND_H_
// Inspired by:
//https://github.com/williamfiset/Algorithms/blob/master/src/main/java/com/williamfiset/algorithms/datastructures/unionfind/UnionFind.java
#include<stdlib.h>
#include<list>
#include<algorithm>

#include"cluster.h"

typedef float float_t;

class Union_find {
private:
    int size;
    int* sz;
    int* id;
    int number_of_components;
    int minimum_cluster_size;
    std::list<Cluster> open_clusters;
    std::list<Cluster> finished_clusters;

public:
    Union_find(int set_size, int cluster_size);
    ~Union_find();

    int find(int p);
    void unify(int p, int q, float_t distance);
    bool connected(int p, int q);
    int component_size(int p);
    int get_size() {return size;};
    int get_number_of_components() {return number_of_components;};
    void merge_clusters(int root1, int root2, float_t distance);
    std::list<Cluster> get_clusters() {return finished_clusters;};
};

#endif //_HDBSCAN_UNION_FIND_H_