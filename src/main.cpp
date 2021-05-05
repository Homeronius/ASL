#include "distance.h"
#include "utils.h"
#include "clustering.h"
#include "cluster.h"
#include "prim.h"
#include "hdbscan.h"

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <vector>

typedef double float_t;

int main() {

    const char *dataset_path = "../../data/blobs_0.csv";
    const char *prediction_path = "../../data/blobs_0_prediction.csv";
    const char *condensed_cluster_tree_path = "../../data/blobs_0_tree.csv";

    const int mpts = 4;
    const int minimum_cluster_size = 5;
    HDBSCAN clusterer(mpts, minimum_cluster_size);

    clusterer.load_dataset(dataset_path);
    //clusterer.show_dataset_head();

    clusterer.build_mst();

    clusterer.build_condensed_cluster_tree();

    clusterer.select_clusters();

    clusterer.extract_labels();

    clusterer.store_predicted_labels(prediction_path);

    clusterer.store_condensed_cluster_tree(condensed_cluster_tree_path);

    return 0;

}
