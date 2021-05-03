#include "distance.h"
#include "utils.h"
#include "clustering.h"
#include "cluster.h"

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <vector>

typedef double float_t;

int main() {

    float_t *dataset = NULL;
    int *labels = NULL;
    int shape[2];
    const char *filename = "../data/blobs_0.csv";

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
    compute_core_distances(dataset, core_dist, mpts, n, d);

    printf("Core distances computed.\n");

    // 2. Compute the MST -> mutual reachability graph
    //prim_advanced(dataset, core_dist, );

    // 3. Extend graph

    // 4. Build hierarchical tree

	int* edgesA;
	int* edgesB;
	float_t* distances;
	int minimum_cluster_size = 10;

	std::vector<Cluster> condensed_cluster_tree = clustering(edgesA,edgesB,distances,n,minimum_cluster_size);

	std::vector<Cluster> selected_clusters = extract_clusters(condensed_cluster_tree);

	int* our_labels = point_labels(selected_clusters,n);

    return 0;

}
