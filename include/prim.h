#ifndef _HDBSCAN_PRIM_H_
#define _HDBSCAN_PRIM_H_

struct edge {
  double weight;
  int u;
  int v;
};

typedef struct edge edge;

void prim(double *adjacency, edge *result, int n);
void prim_diag(double *adjacency, edge *result, int n);
void prim_advanced(double *X, double *core_distances, edge *result, int n,
                   int d);
int compare_edges(const void *e1, const void *e2);
#endif // _HDBSCAN_PRIM_H
