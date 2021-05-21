#include "config.h"
#include "hdbscan.h"

typedef double float_t;

#ifdef HDBSCAN_INSTRUMENT
extern long hdbscan_sqrt_counter;
#endif

#ifdef OUTPUT_COMPILER_INFO
void output_compiler_info() {
  printf("Compiled with\n");
  printf("- Compiler ID:\t\t %s\n", COMPILER_ID);
  printf("- Compiler version:\t %s\n", COMPILER_VERSION);
  printf("- Compiler flags:\t %s\n", COMPILER_FLAGS);
  // printf("Optimization flag: -%s\n", OPT_LEVEL);
}
#endif

int main(int argc, char **argv) {

#ifdef OUTPUT_COMPILER_INFO
  output_compiler_info();
#endif

  if (argc < 2 || argc > 3) {
    printf("Usage: hdbscan_benchmark <input_path> [<output-file>]\n");
    return -1;
  }

  const char *dataset_path = argv[1];
  // const char *condensed_cluster_tree_path = "../../data/blobs_0_tree.csv";

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

  if (argc == 3) {
    const char *prediction_path = argv[2];
    clusterer.store_predicted_labels(prediction_path);
  }

  // clusterer.store_condensed_cluster_tree(condensed_cluster_tree_path);

#ifdef HDBSCAN_INSTRUMENT
  printf("Number of sqrts: %ld\n", hdbscan_sqrt_counter);
#endif

  free(dataset);
  free(labels);
  return 0;
}
