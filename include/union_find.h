#ifndef _HDBSCAN_UNION_FIND_H_
#define _HDBSCAN_UNION_FIND_H_
// Inspired by:
//https://github.com/williamfiset/Algorithms/blob/master/src/main/java/com/williamfiset/algorithms/datastructures/unionfind/UnionFind.java
#include<stdlib.h>
#include<list>
#include<algorithm>

#include"cluster.h"

typedef double float_t;

/**
 * @brief Modified union_find structure which tracks cluster creation
 * 
 */
class Union_find {
private:
    int size;
    int* sz;
    int* id;
    int number_of_components;
    int minimum_cluster_size;
    std::list<Cluster*> open_clusters;
    std::vector<Cluster*> finished_clusters;

public:
    /**
     * @brief Construct a new Union_find object
     * 
     * @param set_size number of datapoints
     * @param cluster_size minimum cluster size
     */
    Union_find(int set_size, int cluster_size);
    ~Union_find();

    int find(int p);
    void unify(int p, int q, float_t distance);
    bool connected(int p, int q);
    int component_size(int p);
    int get_size() {return size;};
    int get_number_of_components() {return number_of_components;};
    /**
     * @brief merge two clusters
     * 
     * @param root1 root of first cluster
     * @param root2 root of second cluster
     * @param distance mutual reachability distance between two original node in mst 
     */
    void merge_clusters(int root1, int root2, float_t distance);

    /**
     * @brief Get the clusters finished list of clusters, which represent the condensed cluster tree.
     * 
     * @return std::list<Cluster> 
     */
    std::vector<Cluster*> get_clusters() {return finished_clusters;};

    void finalize();
};

#endif //_HDBSCAN_UNION_FIND_H_