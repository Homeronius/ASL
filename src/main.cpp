#include "distance.h"
#include "utils.h"
#include "clustering.h"
#include "cluster.h"
#include "prim.h"

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <vector>

typedef double float_t;

int main() {

    float_t *dataset = NULL;
    int *labels = NULL;
    int shape[2];
    const char *filename = "../../data/blobs_0.csv";

    read_csv(&dataset, &labels, &shape, filename);

    int n = shape[0];
    int d = shape[1];
	printf("Loaded dataset of size [%d, %d]\n\n", n, d);

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

    // 1. Compute core distances
    int mpts = 4;
    float_t *core_dist = static_cast<float_t *>(malloc(n * sizeof(float_t)));
    float_t *mutual_reachability_matrix = static_cast<float_t *>(malloc(n * n * sizeof(float_t)));
    compute_core_distances(dataset, core_dist, mpts, n, d);

    printf("Core distances computed.\n");

    // 2. Compute the MST -> mutual reachability graph
    int n_ext = 2*n - 1;
    edge *mst = static_cast<edge *>(malloc(n_ext * sizeof(edge)));
    prim_advanced(dataset, core_dist, mst, n, d);


    // 3. Extend graph with self-edge -> MST_{ext}
    for(int i = 0; i < n; ++i){
        mst[(n-1) + i] = {core_dist[i], i, i};
    }

    // 3.1 Sort edges by increasing weight
    qsort(mst, n_ext, sizeof(edge), compare_edges);

    printf("MST computed and extended. First edge in arr : (%i, %i) | weight : %f\n", mst[0].u, mst[0].v, mst[0].weight);
    printf("Last self-edge in arr : (%i, %i) | weight : %f\n", mst[n_ext-1].u, mst[n_ext-1].v, mst[n_ext-1].weight);

    // 4. Build hierarchical tree

    // 4.1 Convert AoS mst into SoA
    // (optimization easily done with SIMD instructions, or rather unify the interface :D)
	int* edges_A;
	int* edges_B;
	float_t* core_dist_ext;

    edges_A = static_cast<int *>(malloc(n_ext * sizeof(int)));
    edges_B = static_cast<int *>(malloc(n_ext * sizeof(int)));
    core_dist_ext = static_cast<float_t *>(malloc(n_ext * sizeof(float_t)));

    // Do conversion (quick and dirty)
    for(int i = 0; i < n_ext; ++i){
        core_dist_ext[i] = mst[i].weight;
        edges_A[i] = mst[i].u;
        edges_B[i] = mst[i].v;
    }

    // 4.2 Build the hierarchical tree itself

	int minimum_cluster_size = 5;

	std::vector<Cluster*> condensed_cluster_tree = clustering(edges_A, edges_B, core_dist_ext, n_ext, minimum_cluster_size);
    printf("Finished creating condensed cluster tree\n");

    std::vector<Cluster*> ordered_clusters;
    get_preorder(ordered_clusters, condensed_cluster_tree.back());

	std::vector<Cluster*> selected_clusters = extract_clusters(ordered_clusters);

    printf("Finished extracting clusters. Found %i clusters. \n", selected_clusters.size());

	int* our_labels = point_labels(selected_clusters, n_ext);

    
    
    // Store the clustered data
    const char *tree_filename = "../../data/tree.csv";
    condensed_tree_to_csv(ordered_clusters,tree_filename);

    // Store the clustered data
    const char *out_filename = "../../data/blobs_0_predicted.csv";
    write_csv(dataset, our_labels, shape, out_filename);

    for (size_t i = 0; i < condensed_cluster_tree.size(); i++){
        free(condensed_cluster_tree[i]);
    }
    return 0;

}
