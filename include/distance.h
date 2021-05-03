#ifndef _HDBSCAN_DISTANCE_H_
#define _HDBSCAN_DISTANCE_H_

void compute_core_distances(double *input, double *core_dist, int mpts, int n, int d);
void compute_distance_matrix(double *input, double *dist, int mpts, int n, int d);
double euclidean_distance(double *p1, double *p2, int d);
double manhattan_distance(double *p1, double *p2, int d);

#endif // _HDBSCAN_DISTANCE_H_
