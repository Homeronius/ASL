#ifndef _HDBSCAN_PRIM_H_
#define _HDBSCAN_PRIM_H_
void prim(double *adjacency, int *result, int n);
void prim_advanced(double *X, double *core_distances, int *result, int n, int d);
#endif  // _HDBSCAN_PRIM_H