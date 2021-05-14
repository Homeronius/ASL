#ifndef _HDBSCAN_UTILS_H
#define _HDBSCAN_UTILS_H

#include <cluster.h>
#include <vector>

void read_csv(double **dataset, int **labels, int (*shape)[2],
              const char *fname);
void write_csv(double *dataset, int *labels, int shape[2], const char *fname);
void condensed_tree_to_csv(std::vector<Cluster *> tree, const char *fname);
void print_list(double *list, int left, int right);

#endif // _HDBSCAN_UTILS_H
