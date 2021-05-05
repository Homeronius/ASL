#ifndef _HDBSCAN_UTILS_H
#define _HDBSCAN_UTILS_H

#include<vector>
#include<cluster.h>

void read_csv(double **dataset, int **labels, int (*shape)[2], const char *fname);
void write_csv(double *dataset, int *labels, int shape[2], const char *fname);
void condensed_tree_to_csv(std::vector<Cluster*> tree, const char *fname);
#endif // _HDBSCAN_UTILS_H
