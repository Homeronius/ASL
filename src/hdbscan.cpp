#include "hdbscan.h"

typedef double float_t;

HDBSCAN::HDBSCAN(const int mpts, const int minimum_cluster_size):
                mpts(mpts), minimum_cluster_size(minimum_cluster_size) {

}

HDBSCAN::~HDBSCAN(){
    free(dataset);
    free(labels);
    free(predicted_labels);
    free(mst);
    free(core_dist);
    for (size_t i = 0; i < condensed_cluster_tree.size(); i++){
        delete condensed_cluster_tree[i];
    }
};

const char *HDBSCAN::get_path(std::string& fname) const {
    return fname.c_str();
}

void HDBSCAN::load_dataset(const char *fname){
    dataset_path = std::string(fname);

    int shape[2];

    read_csv(&dataset, &labels, &shape, fname);

    n = shape[0];
    d = shape[1];

    n_ext = 2*n - 1;

	printf("Loaded dataset of size [%d, %d]\n\n", n, d);

}

void HDBSCAN::show_dataset_head() const {
    assert(dataset != nullptr && labels != nullptr);

    printf("DATA\n");
    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < d; ++j){
            printf("%f, ", dataset[i*d + j]);
        }
        printf("\n");
    }
    printf("...\n\n");

    printf("LABELS\n");
    for(int i = 0; i < 3; ++i){
        printf("%d\n", labels[i]);
    }
    printf("...\n\n");
}


void HDBSCAN::build_mst() {

     // 1. Compute core distances
     core_dist = static_cast<float_t *>(malloc(n * sizeof(float_t)));
     compute_core_distances(dataset, core_dist, mpts, n, d);

     printf("Core distances computed.\n");

     // 2. Compute the MST -> mutual reachability graph
     mst = static_cast<edge *>(malloc(n_ext * sizeof(edge)));
     prim_advanced(dataset, core_dist, mst, n, d);

     printf("Minimum-Spanning-Tree constructed.\n");
}


void HDBSCAN::build_condensed_cluster_tree(){

    // 3. Extend graph with self-edge -> MST_{ext}
    for(int i = 0; i < n; ++i){
        mst[(n-1) + i] = {core_dist[i], i, i};
    }

    // 3.1 Sort edges by increasing weight
    qsort(mst, n_ext, sizeof(edge), compare_edges);

    printf("MST computed and extended. First edge in arr : (%i, %i) | weight : %f\n", mst[0].u, mst[0].v, mst[0].weight);
    printf("Last self-edge in arr : (%i, %i) | weight : %f\n", mst[n_ext-1].u, mst[n_ext-1].v, mst[n_ext-1].weight);

    // 4. Build hierarchical tree

    // 4.1 Build the hierarchical tree itself

    //condensed_cluster_tree = clustering(edges_A, edges_B, core_dist_ext, n_ext, minimum_cluster_size);
    condensed_cluster_tree = clustering(mst, n_ext, minimum_cluster_size);

    printf("Finished creating condensed cluster tree\n");

}

void HDBSCAN::select_clusters(){

     get_preorder(ordered_cluster_tree, condensed_cluster_tree.back());

     selected_clusters = extract_clusters(ordered_cluster_tree);

     printf("Finished extracting clusters. Found %i clusters. \n", static_cast<int>(selected_clusters.size()));
}

void HDBSCAN::extract_labels(){

    predicted_labels = (int*) calloc(n_ext,sizeof(int));
    point_labels(selected_clusters, n_ext, predicted_labels);
}

void HDBSCAN::store_predicted_labels(const char *fname) {

    prediction_path = std::string(fname);
    int shape[2] = {n, d};

    // Store the clustered data
    write_csv(dataset, predicted_labels, shape, fname);
}

void HDBSCAN::store_condensed_cluster_tree(const char *fname) {
    condensed_cluster_tree_path = std::string(fname);

    condensed_tree_to_csv(ordered_cluster_tree, fname);

}
