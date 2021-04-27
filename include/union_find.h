// Inspired by:
//https://github.com/williamfiset/Algorithms/blob/master/src/main/java/com/williamfiset/algorithms/datastructures/unionfind/UnionFind.java
#include<stdlib.h>
#include<vector>

typedef float float_t;

class Cluster
{
private:
    float_t lambda_birth;
    float_t lambda_death;
    int number_of_components;
    Cluster* parent;
public:
    int root_id;
    Cluster(float_t lambda, int elements): lambda_birth(lambda), lambda_death(lambda), number_of_components(elements) {};
    ~Cluster();
    void finalize(Cluster* p, float_t lambda){
        lambda_death = lambda;
        parent = p;
    }
    double cluster_weight(){
        return (lambda_death - lambda_birth) * number_of_components;
    }
};

class Union_find {
private:
    int size;
    int* sz;
    int* id;
    int number_of_components;
    int minimum_cluster_size;
    std::vector<Cluster> open_clusters;
    std::vector<Cluster> finished_clusters;

public:
    Union_find(int size);
    ~Union_find();

    int find(int p);
    void unify(int p, int q, float_t distance);

    bool connected(int p, int q);
    int component_size(int p);
    int size() {return size};
    int number_of_components() {return number_of_components};
    void merge_clusters(int root1, int root2);
    std::vector<Cluster> get_clusters() {return finished_clusters};
};