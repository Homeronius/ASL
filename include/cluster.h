#ifndef _HDBSCAN_CLUSTER_H_
#define _HDBSCAN_CLUSTER_H_


#include<stdlib.h>
#include<vector>
#include<list>
#include<algorithm>

typedef float float_t;

/**
 * @brief Pruned cluster class
 * 
 */
class Cluster {
private:
    
    float_t lambda_death;

    std::vector<int> components;
    std::vector<float_t> lambdas;

public:
    Cluster* child1;
    Cluster* child2;
    Cluster* parent;
    int root_id;
    bool selected;

    /**
     * @brief Construct a new Cluster object.
     * 
     * @param lambda inverse of the distance at time of creation.
     * @param root_id id of corresponding union_find root element
     * @param elements list of nodes contained in cluster
     */
    Cluster(float_t lambda, int root_id, std::vector<int> elements): components(elements), root_id(root_id) {
        lambdas.assign(components.size(), lambda);
        child1 = nullptr;
        child2 = nullptr;
        selected = true;
    };
    /**
     * @brief Default Construct a empty Cluster object
     * 
     */
    Cluster(){
        child1 = nullptr;
        child2 = nullptr;
        root_id = -1;
    };
    /**
     * @brief Construct a new Cluster object by merging to existing clusters
     * 
     * @param c1 first cluster
     * @param c2 second cluster.
     * @param lambda inverse of the distance at time of creation.
     */
    Cluster(Cluster c1, Cluster c2, float_t lambda){
        // TODO check correctness of copy
        // c.components = c1.components + c2.components
        std::copy(c1.components.begin(),c1.components.end(),components.begin());
        std::copy(c2.components.begin(),c2.components.end(),components.begin()+c1.components.size());
        lambdas.assign(components.size(), lambda);

        if(c1.components.size() < c2.components.size()){
            root_id = c2.root_id;
        } else {
            root_id = c1.root_id;
        }
        child1 = &c1;
        child2 = &c2;
        selected = false;
    };
    ~Cluster();
    /**
     * @brief Close cluster after merging.
     * 
     * @param p parent cluster into which it has been merged.
     * @param lambda inverse of the distance at time of completion.
     */
    void finalize(Cluster* p, float_t lambda){
        lambda_death = lambda;
        parent = p;
    }
    /**
     * @brief Used to calculate stability of cluster. 
     * 
     * @return double Sum of lifetimes of the original nodes.
     */
    double get_cluster_weight(){
        double sum = 0;
        for(int i = 0; i < lambdas.size(); i++){
            sum +=  lambdas[i] - lambda_death;
        }
    }

    double get_max_cluster_weight(){
        if (selected){
            return get_cluster_weight();
        } else {
            return get_children_cluster_weight();
        }
    }

    double get_children_cluster_weight(){
        if (child1 != nullptr && child2 != nullptr){
            return child1->get_cluster_weight() + child2->get_cluster_weight();
        } else {
            return 0;
        }
    }

    /**
     * @brief add single node to cluster
     * 
     * @param root_id id of leaf node
     * @param lambda 
     */
    void add_leaf(int root_id, float_t lambda){
        components.push_back(root_id);
        lambdas.push_back(lambda);
    }
    /**
     * @brief add group (< min cluster size) to cluster
     * 
     * @param root_ids ids of elements
     * @param lambda 
     */
    void add_leaf(std::vector<int> root_ids, float_t lambda){
        components.insert(components.end(), root_ids.begin(), root_ids.end()); //append root ids
        lambdas.insert(lambdas.end(),root_ids.size(),lambda); //append current lambda for of new elemennts
    }

    std::vector<int> get_components(){
        return components;
    }
};

bool operator==(const Cluster& A, const Cluster& B){
    return A.root_id == B.root_id;
}
#endif //_HDBSCAN_CLUSTER_H_