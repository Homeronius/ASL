#ifndef _HDBSCAN_DISTANCE_H_
#define _HDBSCAN_DISTANCE_H_

void compute_core_distances(double *input, double *core_dist, int mpts, int n,
                            int d);
void compute_distance_matrix(double *input, double *core_dist, double *dist,
                             int mpts, int n, int d);
void compute_distance_matrix_triang(double *input, double *core_dist,
                                    double *dist, int mpts, int n, int d);
void compute_distance_matrix_blocked(double *input, double *core_dist,
                                     double *dist, int mpts, int n, int d);

void euclidean_distance_4_opt(double *base, double *p1, double *p2, double *p3,
                              double *p4, double *res);
void euclidean_distance_2(double *pa1, double *pa2, double *pb1, double *pb2,
                          double *pc1, double *pc2, double *pd1, double *pd2,
                          double *res);
double euclidean_distance(double *p1, double *p2, int d);
double manhattan_distance(double *p1, double *p2, int d);
double quickselect(double *list, int left, int right, int k);
double iterative_quickselect(double *list, int left, int right, int k);

#endif // _HDBSCAN_DISTANCE_H_
