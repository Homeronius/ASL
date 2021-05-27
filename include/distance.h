#ifndef _HDBSCAN_DISTANCE_H_
#define _HDBSCAN_DISTANCE_H_
#include <immintrin.h>

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
void euclidean_distance_4_opt_alt(double *base, double *p1, double *p2,
                                  double *p3, double *p4, double *res);
void euclidean_distance_2(double *pa1, double *pa2, double *pb1, double *pb2,
                          double *pc1, double *pc2, double *pd1, double *pd2,
                          double *res);
__m256d euclidean_distance_2_ret_simd(double *pa1, double *pa2, double *pb1,
                                      double *pb2, double *pc1, double *pc2,
                                      double *pd1, double *pd2);
__m256d euclidean_distance_4_opt_ret_simd(double *base, double *p1, double *p2,
                                          double *p3, double *p4);
double euclidean_distance(double *p1, double *p2, int d);
double manhattan_distance(double *p1, double *p2, int d);

#endif // _HDBSCAN_DISTANCE_H_
