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
    int mpts = 3;
    float_t *core_dist = static_cast<float_t *>(malloc(n * n * sizeof(float_t)));
    compute_distance_matrix(dataset, core_dist, mpts, n, d);

    printf("Core distances computed.\n");

    // 2. Compute the MST -> mutual reachability graph
    int n_ext = 2*n - 1;
    edge *mst = static_cast<edge *>(malloc(n_ext * sizeof(edge)));
    prim(core_dist, mst, n);

    printf("MST computed. First edge in arr : (%i, %i) | weight : %f\n", mst[0].u, mst[0].v, mst[0].weight);

    // 3. Extend graph with self-edge -> MST_{ext}
    for(int i = 0; i < n; ++i){
        mst[(n-1) + i] = {core_dist[i*n + i], i, i};
    }

    printf("MST extended. Last self-edge in arr : (%i, %i) | weight : %f\n", mst[n_ext-1].u, mst[n_ext-1].v, mst[n_ext-1].weight);

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
        edges_A[i] = mst[i].u;
        edges_B[i] = mst[i].v;
        core_dist_ext[i] = mst[i].weight;
    }

    // 4.2 Build the hierarchical tree itself

	int minimum_cluster_size = 3;

	std::vector<Cluster> condensed_cluster_tree = clustering(edges_A, edges_B, core_dist_ext, n_ext, minimum_cluster_size);

    printf("Finished creating condensed cluster tree");

	std::vector<Cluster> selected_clusters = extract_clusters(condensed_cluster_tree);

    printf("Finished extracting clusters. Found %i clusters. \n", selected_clusters.size());

	int* our_labels = point_labels(selected_clusters, n_ext);

    return 0;

}
