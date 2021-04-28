#ifndef _HDBSCAN_CLUSTER_H_
#define _HDBSCAN_CLUSTER_H_


#include<stdlib.h>
#include<vector>
#include<list>
#include<algorithm>

typedef float float_t;

class Cluster
{
private:
    float_t lambda_death;

    std::vector<int> components;
    std::vector<float_t> lambdas;

    Cluster* parent;
public:
    int root_id;
    Cluster(float_t lambda, int root_id, std::vector<int> elements): components(elements), root_id(root_id) {
        lambdas.assign(components.size(), lambda);
    };
    Cluster(){
        root_id = -1;
    };
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
    };
    ~Cluster();
    
    void finalize(Cluster* p, float_t lambda){
        lambda_death = lambda;
        parent = p;
    }
    double cluster_weight();
    void add_leaf(int root_id, float_t lambda){
        components.push_back(root_id);
        lambdas.push_back(lambda);
    }
    void add_leaf(std::vector<int> root_ids, float_t lambda){
        components.insert(components.end(), root_ids.begin(), root_ids.end()); //append root ids
        lambdas.insert(lambdas.end(),root_ids.size(),lambda); //append current lambda for of new elemennts
    }
};

bool operator==(const Cluster& A, const Cluster& B){
    return A.root_id == B.root_id;
}
#endif //_HDBSCAN_CLUSTER_H_