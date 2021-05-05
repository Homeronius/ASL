#ifndef _HDBSCAN_HDBSCAN_H_
#define _HDBSCAN_HDBSCAN_H_
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

class HDBSCAN {
    private:
        std::string dataset_path;
        std::string prediction_path;
        std::string condensed_cluster_tree_path;

        const int mpts;
        const int minimum_cluster_size;

        float_t *dataset = nullptr;
        int *labels = nullptr;
        int *predicted_labels = nullptr;

        // Algorithm related buffer pointers
        float_t *core_dist = nullptr;     // core distance for each datapoint
        edge *mst = nullptr;           // minimum-spanning-tree
        std::vector<Cluster*> condensed_cluster_tree;
        std::vector<Cluster*> ordered_cluster_tree;
        std::vector<Cluster*> selected_clusters;

        int n;
        int n_ext;
        int d;

        const char *get_path(std::string& fname) const;

    public:
        HDBSCAN(const int mpts, const int minimum_cluster_size);
        ~HDBSCAN();

        void load_dataset(const char *fname);
        void show_dataset_head() const;
        void build_mst();
        void build_condensed_cluster_tree();
        void select_clusters();
        void extract_labels();
        void store_predicted_labels(const char *fname);
        void store_condensed_cluster_tree(const char *fname);
};
#endif //__HDBSCAN_HDBSCAN_H_
