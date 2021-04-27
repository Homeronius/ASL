#include <assert.h>
#include<vector>

#include "union_find.h"
Union_find::Union_find(int set_size, int cluster_size){
    minimum_cluster_size = cluster_size;
    assert(minimum_cluster_size > 1 && "Smallest clusters must be greater than 1");

    size = set_size;
    number_of_components = size;
    sz = malloc(size*sizeof(int));
    id = malloc(size*sizeof(int));

    for (size_t i = 0; i < size; i++){
        sz[i]=1;
        id[i]=i;
    }
}

Union_find::~Union_find(){
    free(sz);
    free(id);
}

int Union_find::find() {
    int root = p;
    //find root
    while (root != id[root]) root = id[root];
    
    //path compression
    while (p != root) {
      int next = id[p];
      id[p] = root;
      p = next;
    }
    
    return root;
}

void Union_find::merge_clusters(int root1, int root2){

    Cluster c1;
    Cluster c2;
    Cluster c_new;

    
    float_t lambda =  1 / distance;
    c_new(lambda,sz[root1]+sz[root2]);
    c1.finalize(&c_new,lambda);
    c2.finalize(&c_new,lambda);
    open_clusters.erase(c1);
    open_clusters.erase(c2);
    finished_clusters.push_back(c1);
    finished_clusters.push_back(c2);
}

void Union_find::unify(int p, int q, float_t distance) {

    if (connected(p, q)) return; //may be redundant for our use?

    int root1 = find(p);
    int root2 = find(q);
    
    if (sz[root1]+sz[root2] > minimum_cluster_size){ //union will be valid cluster
        merge_clusters(root1,root2);    
    }



    if (sz[root1] < sz[root2]) {
      sz[root2] += sz[root1];
      id[root1] = root2;
    } else {
      sz[root1] += sz[root2];
      id[root2] = root1;
    }

    numComponents--;
}

bool Union_find::connected(int p, int q){
    return find(p) == find(q);
}

int Union_find::componentSize(int p) {
    return sz[find(p)];
}

