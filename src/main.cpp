#include "hdbscan.h"

typedef double float_t;

#ifdef HDBSCAN_INSTRUMENT
extern long hdbscan_sqrt_counter;
#endif

int main() {

  const char *dataset_path = "../../data/blobs_0.csv";
  const char *prediction_path = "../../data/blobs_0_prediction.csv";
  const char *condensed_cluster_tree_path = "../../data/blobs_0_tree.csv";

  const int mpts = 4;
  const int minimum_cluster_size = 5;
  int n, d;
  double *dataset;
  int *labels;

  load_dataset(dataset_path, &dataset, &labels, &n, &d);
  // clusterer.show_dataset_head();
  HDBSCAN clusterer(mpts, minimum_cluster_size, dataset, labels, n, d);

  clusterer.build_mst();

  clusterer.build_condensed_cluster_tree();

  clusterer.select_clusters();

  clusterer.extract_labels();

  clusterer.store_predicted_labels(prediction_path);

  clusterer.store_condensed_cluster_tree(condensed_cluster_tree_path);

  free(dataset);
  free(labels);

#ifdef HDBSCAN_INSTRUMENT
  printf("Number of sqrts: %ld\n", hdbscan_sqrt_counter);
#endif

  return 0;
}
