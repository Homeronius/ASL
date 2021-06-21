#include "distance.h"
#include "quickselect.h"
#include <immintrin.h>
#include <math.h>
#include <stdint.h>

#ifdef FINE_GRAINED_BENCH
#include "benchmark_util.h"
#endif

#ifdef HDBSCAN_INSTRUMENT
extern long hdbscan_sqrt_counter;
#endif

void euclidean_distance_2(double *pa1, double *pa2, double *pb1, double *pb2,
                          double *pc1, double *pc2, double *pd1, double *pd2,
                          double *res) {
  __m256d sum_ab;
  __m256d sum_cd;
  {
    __m256d ld1 = _mm256_loadu2_m128d(pc1, pa1);
    __m256d ld2 = _mm256_loadu2_m128d(pc2, pa2);
    __m256d diff = _mm256_sub_pd(ld1, ld2);
    sum_ab = _mm256_mul_pd(diff, diff);
  }
  {
    __m256d ld1 = _mm256_loadu2_m128d(pd1, pb1);
    __m256d ld2 = _mm256_loadu2_m128d(pd2, pb2);
    __m256d diff = _mm256_sub_pd(ld1, ld2);
    sum_cd = _mm256_mul_pd(diff, diff);
  }

  __m256d add1 = _mm256_hadd_pd(sum_ab, sum_cd);

  // sqrt = | sqrt(a) | sqrt(b) | sqrt(c) | sqrt(d) |
  __m256d sqrt = _mm256_sqrt_pd(add1);

  _mm256_storeu_pd(res, sqrt);

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter += 4;
#endif
}

// Same function as above but avoiding a storeu by returning SIMD vector
// directly
__m256d euclidean_distance_2_ret_simd(double *pa1, double *pa2, double *pb1,
                                      double *pb2, double *pc1, double *pc2,
                                      double *pd1, double *pd2) {
  __m256d sum_ab;
  __m256d sum_cd;
  {
    __m256d ld1 = _mm256_loadu2_m128d(pc1, pa1);
    __m256d ld2 = _mm256_loadu2_m128d(pc2, pa2);
    __m256d diff = _mm256_sub_pd(ld1, ld2);
    sum_ab = _mm256_mul_pd(diff, diff);
  }
  {
    __m256d ld1 = _mm256_loadu2_m128d(pd1, pb1);
    __m256d ld2 = _mm256_loadu2_m128d(pd2, pb2);
    __m256d diff = _mm256_sub_pd(ld1, ld2);
    sum_cd = _mm256_mul_pd(diff, diff);
  }

  __m256d add1 = _mm256_hadd_pd(sum_ab, sum_cd);

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter += 4;
#endif

  // sqrt = | sqrt(a) | sqrt(b) | sqrt(c) | sqrt(d) |
  return _mm256_sqrt_pd(add1);
}

void euclidean_distance_4(double *pa1, double *pa2, double *pb1, double *pb2,
                          double *pc1, double *pc2, double *pd1, double *pd2,
                          double *res) {
  __m256d pva1 = _mm256_loadu_pd(pa1);
  __m256d pva2 = _mm256_loadu_pd(pa2);
  __m256d diffa = _mm256_sub_pd(pva1, pva2);
  __m256d suma = _mm256_mul_pd(diffa, diffa);

  __m256d pvb1 = _mm256_loadu_pd(pb1);
  __m256d pvb2 = _mm256_loadu_pd(pb2);
  __m256d diffb = _mm256_sub_pd(pvb1, pvb2);
  __m256d sumb = _mm256_mul_pd(diffb, diffb);

  __m256d pvc1 = _mm256_loadu_pd(pc1);
  __m256d pvc2 = _mm256_loadu_pd(pc2);
  __m256d diffc = _mm256_sub_pd(pvc1, pvc2);
  __m256d sumc = _mm256_mul_pd(diffc, diffc);

  __m256d pvd1 = _mm256_loadu_pd(pd1);
  __m256d pvd2 = _mm256_loadu_pd(pd2);
  __m256d diffd = _mm256_sub_pd(pvd1, pvd2);
  __m256d sumd = _mm256_mul_pd(diffd, diffd);

  __m256d add1 = _mm256_hadd_pd(suma, sumc);
  __m256d add2 = _mm256_hadd_pd(sumb, sumd);

  __m256d perm1 = _mm256_permute4x64_pd(add1, 0b11011000);
  __m256d perm2 = _mm256_permute4x64_pd(add2, 0b11011000);

  __m256d add3 = _mm256_hadd_pd(perm1, perm2);

  // sqrt = | sqrt(a) | sqrt(b) | sqrt(c) | sqrt(d) |
  __m256d sqrt = _mm256_sqrt_pd(add3);
  _mm256_storeu_pd(res, sqrt);

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter += 4;
#endif
}

void euclidean_distance_4_opt(double *base, double *p1, double *p2, double *p3,
                              double *p4, double *res) {
  __m256d base_vec = _mm256_loadu_pd(base);

  __m256d partial_sum1;
  __m256d partial_sum2;

  {
    __m256d p1_vec = _mm256_loadu_pd(p1);
    __m256d diff1 = _mm256_sub_pd(base_vec, p1_vec);
    __m256d sum1 = _mm256_mul_pd(diff1, diff1);

    __m256d p3_vec = _mm256_loadu_pd(p3);
    __m256d diff2 = _mm256_sub_pd(base_vec, p3_vec);
    __m256d sum2 = _mm256_mul_pd(diff2, diff2);

    partial_sum1 = _mm256_hadd_pd(sum1, sum2);
  }
  {
    __m256d p2_vec = _mm256_loadu_pd(p2);
    __m256d diff3 = _mm256_sub_pd(base_vec, p2_vec);
    __m256d sum3 = _mm256_mul_pd(diff3, diff3);

    __m256d p4_vec = _mm256_loadu_pd(p4);
    __m256d diff4 = _mm256_sub_pd(base_vec, p4_vec);
    __m256d sum4 = _mm256_mul_pd(diff4, diff4);

    partial_sum2 = _mm256_hadd_pd(sum3, sum4);
  }

  __m256d perm1 = _mm256_permute4x64_pd(partial_sum1, 0b11011000);
  __m256d perm2 = _mm256_permute4x64_pd(partial_sum2, 0b11011000);

  __m256d sum = _mm256_hadd_pd(perm1, perm2);

  // sqrt = | sqrt(p1) | sqrt(p2) | sqrt(p3) | sqrt(p4) |
  __m256d sqrt = _mm256_sqrt_pd(sum);
  _mm256_storeu_pd(res, sqrt);

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter += 4;
#endif
}

// Same function as above but avoiding a storeu by returning SIMD vector
// directly
__m256d euclidean_distance_4_opt_ret_simd(double *base, double *p1, double *p2,
                                          double *p3, double *p4) {
  __m256d base_vec = _mm256_loadu_pd(base);

  __m256d partial_sum1;
  __m256d partial_sum2;

  {
    __m256d p1_vec = _mm256_loadu_pd(p1);
    __m256d diff1 = _mm256_sub_pd(base_vec, p1_vec);
    __m256d sum1 = _mm256_mul_pd(diff1, diff1);

    __m256d p3_vec = _mm256_loadu_pd(p3);
    __m256d diff2 = _mm256_sub_pd(base_vec, p3_vec);
    __m256d sum2 = _mm256_mul_pd(diff2, diff2);

    partial_sum1 = _mm256_hadd_pd(sum1, sum2);
  }
  {
    __m256d p2_vec = _mm256_loadu_pd(p2);
    __m256d diff3 = _mm256_sub_pd(base_vec, p2_vec);
    __m256d sum3 = _mm256_mul_pd(diff3, diff3);

    __m256d p4_vec = _mm256_loadu_pd(p4);
    __m256d diff4 = _mm256_sub_pd(base_vec, p4_vec);
    __m256d sum4 = _mm256_mul_pd(diff4, diff4);

    partial_sum2 = _mm256_hadd_pd(sum3, sum4);
  }

  __m256d perm1 = _mm256_permute4x64_pd(partial_sum1, 0b11011000);
  __m256d perm2 = _mm256_permute4x64_pd(partial_sum2, 0b11011000);

  __m256d sum = _mm256_hadd_pd(perm1, perm2);

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter += 4;
#endif

  // sqrt = | sqrt(p1) | sqrt(p2) | sqrt(p3) | sqrt(p4) |
  return _mm256_sqrt_pd(sum);
}

// 44 double flops per 4 distances
void euclidean_distance_4_opt_alt(double *base, double *p1, double *p2,
                                  double *p3, double *p4, double *res) {
  __m256d base_vec = _mm256_loadu_pd(base);

  __m256d partial_sum1;
  __m256d partial_sum2;

  {
    __m256d p1_vec = _mm256_loadu_pd(p1);
    __m256d diff1 = _mm256_sub_pd(base_vec, p1_vec);
    __m256d sum1 = _mm256_mul_pd(diff1, diff1);

    __m256d pvb2 = _mm256_loadu_pd(p3);
    __m256d diff2 = _mm256_sub_pd(base_vec, pvb2);
    __m256d sum2 = _mm256_mul_pd(diff2, diff2);

    __m256d switched3 = _mm256_permute_pd(sum1, 1);
    __m256d switched4 = _mm256_permute_pd(sum2, 1);
    __m256d b1 = _mm256_blend_pd(sum1, sum2, 0b0101);
    __m256d b2 = _mm256_blend_pd(switched3, switched4, 0b0101);
    partial_sum1 = _mm256_add_pd(b1, b2);
  }
  {
    __m256d p3_vec = _mm256_loadu_pd(p2);
    __m256d diff3 = _mm256_sub_pd(base_vec, p3_vec);
    __m256d sum3 = _mm256_mul_pd(diff3, diff3);

    __m256d p4_vec = _mm256_loadu_pd(p4);
    __m256d diff4 = _mm256_sub_pd(base_vec, p4_vec);
    __m256d sum4 = _mm256_mul_pd(diff4, diff4);

    __m256d switched3 = _mm256_permute_pd(sum3, 1);
    __m256d switched4 = _mm256_permute_pd(sum4, 1);
    __m256d b1 = _mm256_blend_pd(sum3, sum4, 0b0101);
    __m256d b2 = _mm256_blend_pd(switched3, switched4, 0b0101);
    partial_sum2 = _mm256_add_pd(b1, b2);
  }

  __m256d perm1 = _mm256_permute4x64_pd(partial_sum1, 0b11011000);
  __m256d perm2 = _mm256_permute4x64_pd(partial_sum2, 0b11011000);

  __m256d switched3 = _mm256_permute_pd(perm1, 1);
  __m256d switched4 = _mm256_permute_pd(perm2, 1);
  __m256d b1 = _mm256_blend_pd(perm1, perm2, 0b0101);
  __m256d b2 = _mm256_blend_pd(switched3, switched4, 0b0101);
  __m256d sum = _mm256_add_pd(b1, b2);

  __m256d sqrt = _mm256_sqrt_pd(sum);
  _mm256_storeu_pd(res, sqrt);

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter += 4;
#endif
}

// This has better performance for smaller dimensions (e.g. d=4),
// but performes a lot worse for high dimensions (e.g. d=400)
// double euclidean_distance(double *p1, double *p2, int d) {
//   __m256d sum = _mm256_setzero_pd();

//   int i = 0;
//   for (; i < d - 3; i += 4) {
//     __m256d pv1 = _mm256_loadu_pd(p1 + i);
//     __m256d pv2 = _mm256_loadu_pd(p2 + i);
//     __m256d diff = _mm256_sub_pd(pv1, pv2);
//     sum = _mm256_fmadd_pd(diff, diff, sum);
//   }

//   __m128d lower = _mm256_castpd256_pd128(sum);
//   __m128d upper = _mm256_extractf128_pd(sum, 1);

//   __m128d s = _mm_add_pd(lower, upper);
//   __m128d p = _mm_permute_pd(s, 0b01);
//   __m128d res = _mm_add_sd(s,p);

//   for (; i < d; i++) {
//     __m128d d_rest = _mm_sub_sd(_mm_load_sd(p1+i), _mm_load_sd(p2+i));
//     res = _mm_fmadd_sd(d_rest, d_rest, res);
//   }

// #ifdef HDBSCAN_INSTRUMENT
//   hdbscan_sqrt_counter++;
// #endif

//   return _mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), res));
// }

double euclidean_distance(double *p1, double *p2, int d) {
  int i = 0;
  __m256d sum1 = _mm256_setzero_pd();
  __m256d sum2 = _mm256_setzero_pd();
  __m256d sum3 = _mm256_setzero_pd();
  __m256d sum4 = _mm256_setzero_pd();

  for (; i < d - 15; i += 16) {
    __m256d pv1 = _mm256_loadu_pd(p1 + i);
    __m256d pv2 = _mm256_loadu_pd(p2 + i);
    __m256d pv3 = _mm256_loadu_pd(p1 + i + 4);
    __m256d pv4 = _mm256_loadu_pd(p2 + i + 4);
    __m256d pv5 = _mm256_loadu_pd(p1 + i + 8);
    __m256d pv6 = _mm256_loadu_pd(p2 + i + 8);
    __m256d pv7 = _mm256_loadu_pd(p1 + i + 12);
    __m256d pv8 = _mm256_loadu_pd(p2 + i + 12);
    __m256d diff1 = _mm256_sub_pd(pv1, pv2);
    __m256d diff2 = _mm256_sub_pd(pv3, pv4);
    __m256d diff3 = _mm256_sub_pd(pv5, pv6);
    __m256d diff4 = _mm256_sub_pd(pv7, pv8);
    sum1 = _mm256_fmadd_pd(diff1, diff1, sum1);
    sum2 = _mm256_fmadd_pd(diff2, diff2, sum2);
    sum3 = _mm256_fmadd_pd(diff3, diff3, sum3);
    sum4 = _mm256_fmadd_pd(diff4, diff4, sum4);
  }

  __m256d suma = _mm256_add_pd(sum1, sum2);
  __m256d sumb = _mm256_add_pd(sum3, sum4);
  __m256d sum = _mm256_add_pd(suma, sumb);

  __m128d lower = _mm256_castpd256_pd128(sum);
  __m128d upper = _mm256_extractf128_pd(sum, 1);

  __m128d s = _mm_add_pd(upper, lower);
  __m128d p = _mm_permute_pd(s, 0b01);
  __m128d res = _mm_add_sd(p, s);

  for (; i < d; i++) {
    __m128d p1_vec = _mm_load_sd(p1 + i);
    __m128d p2_vec = _mm_load_sd(p2 + i);
    __m128d diff = _mm_sub_sd(p1_vec, p2_vec);
    res = _mm_fmadd_sd(diff, diff, res);
  }

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter++;
#endif

  return _mm_cvtsd_f64(_mm_sqrt_sd(res, res));
}

// This has better performance for smaller dimensions (e.g. d=4),
// but performes a lot worse for high dimensions (e.g. d=400)
// double manhattan_distance(double *p1, double *p2, int d) {
//   __m256d sum = _mm256_setzero_pd();
//   __m256d abs_mask =
//       _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFFFFFFFFFFFFFF));

//   // TODO: could load 4 at the same time to fully utilize the hadds.
//   int i = 0;
//   for (; i < d - 3; i += 4) {
//     __m256d p1_vec = _mm256_loadu_pd(p1 + i);
//     __m256d p2_vec = _mm256_loadu_pd(p2 + i);
//     __m256d diff = _mm256_sub_pd(p1_vec, p2_vec);
//     __m256d abs = _mm256_and_pd(diff, abs_mask);
//     sum = _mm256_add_pd(sum, abs);
//   }

//   __m128d lower = _mm256_castpd256_pd128(sum);
//   __m128d upper = _mm256_extractf128_pd(sum, 1);

//   __m128d s = _mm_add_pd(lower, upper);
//   __m128d p = _mm_permute_pd(s, 0b01);
//   double res = _mm_cvtsd_f64(_mm_add_pd(s, p));

//   for (; i < d; i++) {
//     double diff = p1[i] - p2[i];
//     res += fabs(diff);
//   }

//   return res;
// }

double manhattan_distance(double *p1, double *p2, int d) {

  int i = 0;
  __m256d sum1 = _mm256_setzero_pd();
  __m256d sum2 = _mm256_setzero_pd();
  __m256d sum3 = _mm256_setzero_pd();
  __m256d sum4 = _mm256_setzero_pd();
  __m256d abs_mask =
      _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFFFFFFFFFFFFFF));

  for (; i < d - 15; i += 16) {
    __m256d pv1 = _mm256_loadu_pd(p1 + i);
    __m256d pv2 = _mm256_loadu_pd(p2 + i);
    __m256d pv3 = _mm256_loadu_pd(p1 + i + 4);
    __m256d pv4 = _mm256_loadu_pd(p2 + i + 4);
    __m256d pv5 = _mm256_loadu_pd(p1 + i + 8);
    __m256d pv6 = _mm256_loadu_pd(p2 + i + 8);
    __m256d pv7 = _mm256_loadu_pd(p1 + i + 12);
    __m256d pv8 = _mm256_loadu_pd(p2 + i + 12);
    __m256d diff1 = _mm256_sub_pd(pv1, pv2);
    __m256d diff2 = _mm256_sub_pd(pv3, pv4);
    __m256d diff3 = _mm256_sub_pd(pv5, pv6);
    __m256d diff4 = _mm256_sub_pd(pv7, pv8);
    sum1 = _mm256_add_pd(_mm256_and_pd(diff1, abs_mask), sum1);
    sum2 = _mm256_add_pd(_mm256_and_pd(diff2, abs_mask), sum2);
    sum3 = _mm256_add_pd(_mm256_and_pd(diff3, abs_mask), sum3);
    sum4 = _mm256_add_pd(_mm256_and_pd(diff4, abs_mask), sum4);
  }

  __m256d suma = _mm256_add_pd(sum1, sum2);
  __m256d sumb = _mm256_add_pd(sum3, sum4);
  __m256d sum = _mm256_add_pd(suma, sumb);

  __m128d lower = _mm256_castpd256_pd128(sum);
  __m128d upper = _mm256_extractf128_pd(sum, 1);

  __m128d s = _mm_add_pd(lower, upper);
  __m128d p = _mm_permute_pd(s, 0b01);
  __m128d res = _mm_add_sd(s, p);

  for (; i < d; i++) {
    __m128d p1_vec = _mm_load_sd(p1 + i);
    __m128d p2_vec = _mm_load_sd(p2 + i);
    __m128d diff = _mm_sub_sd(p1_vec, p2_vec);
    res = _mm_add_sd(_mm_and_pd(diff, _mm256_castpd256_pd128(abs_mask)), res);
  }

  return _mm_cvtsd_f64(res);
}

void manhattan_distance_2(double *pa1, double *pa2, double *pb1, double *pb2,
                          double *pc1, double *pc2, double *pd1, double *pd2,
                          double *res) {
  __m256d sum_ab;
  __m256d sum_cd;
  __m256d abs_mask =
      _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFFFFFFFFFFFFFF));
  {
    __m256d ld1 = _mm256_loadu2_m128d(pc1, pa1);
    __m256d ld2 = _mm256_loadu2_m128d(pc2, pa2);
    __m256d diff = _mm256_sub_pd(ld1, ld2);
    sum_ab = _mm256_and_pd(diff, abs_mask);
  }
  {
    __m256d ld1 = _mm256_loadu2_m128d(pd1, pb1);
    __m256d ld2 = _mm256_loadu2_m128d(pd2, pb2);
    __m256d diff = _mm256_sub_pd(ld1, ld2);
    sum_cd = _mm256_and_pd(diff, abs_mask);
  }

  __m256d add = _mm256_hadd_pd(sum_ab, sum_cd);

  _mm256_storeu_pd(res, add);
}

void manhattan_distance_4(double *pa1, double *pa2, double *pb1, double *pb2,
                          double *pc1, double *pc2, double *pd1, double *pd2,
                          double *res) {
  __m256d abs_mask =
      _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFFFFFFFFFFFFFF));
  __m256d pa1_vec = _mm256_loadu_pd(pa1);
  __m256d pa2_vec = _mm256_loadu_pd(pa2);
  __m256d diffa = _mm256_sub_pd(pa1_vec, pa2_vec);
  __m256d absa = _mm256_and_pd(diffa, abs_mask);

  __m256d pb1_vec = _mm256_loadu_pd(pb1);
  __m256d pb2_vec = _mm256_loadu_pd(pb2);
  __m256d diffb = _mm256_sub_pd(pb1_vec, pb2_vec);
  __m256d absb = _mm256_and_pd(diffb, abs_mask);

  __m256d pc1_vec = _mm256_loadu_pd(pc1);
  __m256d pc2_vec = _mm256_loadu_pd(pc2);
  __m256d diffc = _mm256_sub_pd(pc1_vec, pc2_vec);
  __m256d absc = _mm256_and_pd(diffc, abs_mask);

  __m256d pd1_vec = _mm256_loadu_pd(pd1);
  __m256d pd2_vec = _mm256_loadu_pd(pd2);
  __m256d diffd = _mm256_sub_pd(pd1_vec, pd2_vec);
  __m256d absd = _mm256_and_pd(diffd, abs_mask);

  __m256d add1 = _mm256_hadd_pd(absa, absc);
  __m256d add2 = _mm256_hadd_pd(absb, absd);

  __m256d perm1 = _mm256_permute4x64_pd(add1, 0b11011000);
  __m256d perm2 = _mm256_permute4x64_pd(add2, 0b11011000);

  __m256d add3 = _mm256_hadd_pd(perm1, perm2);
  _mm256_storeu_pd(res, add3);
}

double get_kth_neighbor(double *input, int n, int k) {
  if (k <= 0) {
    return 0;
  }
  double knn_vals[k]; // Sorted nearest distances found so far
  for (int i = 0; i < k; i++) {
    knn_vals[i] = __DBL_MAX__;
  }
  for (int j = 0; j < n; j++) {
    double val = input[j];
    int neighborIndex = k;
    // Check at which position in the nearest distances the current distance
    // would fit:
    while (neighborIndex >= 1 && val < knn_vals[neighborIndex - 1]) {
      neighborIndex--;
    }
    // Shift elements in the array to make room for the current distance:
    if (neighborIndex < k) {
      for (int shiftIndex = k - 1; shiftIndex > neighborIndex; shiftIndex--) {
        knn_vals[shiftIndex] = knn_vals[shiftIndex - 1];
      }
      knn_vals[neighborIndex] = val;
    }
  }
  return knn_vals[k - 1];
}

void compute_core_distances(double *input, double *core_dist, int mpts, int n,
                            int d) {
  double distances[n];

#ifdef FINE_GRAINED_BENCH
  struct measurement_data_t mdata;
  init_measurement(&mdata);
#endif

  BuildPackMask();
  for (int k = 0; k < n; k++) {
#ifdef FINE_GRAINED_BENCH
    auto compute = [&]() {
#endif
      int i = 0;
#ifdef SPECIALIZED_DISTANCE
      for (; i < n - 3; i += 4) {
        if (d == 2) {
          euclidean_distance_2(input + k * d, input + i * d, input + k * d,
                               input + (i + 1) * d, input + k * d,
                               input + (i + 2) * d, input + k * d,
                               input + (i + 3) * d, distances + i);
        } else if (d == 4) {
          euclidean_distance_4_opt_alt(input + k * d, input + i * d,
                                       input + (i + 1) * d, input + (i + 2) * d,
                                       input + (i + 3) * d, distances + i);
        } else {
          break;
        }
      }
#endif
      for (; i < n; i++) {
        distances[i] = euclidean_distance(input + i * d, input + k * d, d);
      }
#ifdef FINE_GRAINED_BENCH
    };
    add_measurement(&mdata, compute);
#endif
#ifdef HDBSCAN_QUICKSELECT
    core_dist[k] = iterative_quickselect(distances, n, mpts - 1);
#else
    core_dist[k] = get_kth_neighbor(distances, n, mpts);
#endif
  }

#ifdef FINE_GRAINED_BENCH
  print_measurement(&mdata, "(distance)");
#endif
}

void compute_distance_matrix(double *input, double *core_dist, double *dist,
                             int mpts, int n, int d) {
  double tmp[n];
#ifdef FINE_GRAINED_BENCH
  struct measurement_data_t mdata;
  init_measurement(&mdata);
#endif

  for (int i = 0; i < n; i++) {
#ifdef FINE_GRAINED_BENCH
    auto compute = [&]() {
#endif
      int k = 0;
#ifdef SPECIALIZED_DISTANCE
      for (; k < n - 3; k += 4) {
        if (d == 2) {
          euclidean_distance_2(input + i * d, input + k * d, input + i * d,
                               input + (k + 1) * d, input + i * d,
                               input + (k + 2) * d, input + i * d,
                               input + (k + 3) * d, tmp + k);
        } else if (d == 4) {
          euclidean_distance_4_opt_alt(input + i * d, input + k * d,
                                       input + (k + 1) * d, input + (k + 2) * d,
                                       input + (k + 3) * d, tmp + k);
        } else {
          break;
        }

        __m256d t = _mm256_loadu_pd(tmp + k);
        _mm256_storeu_pd(dist + i * n + k, t);
      }
#endif
      for (; k < n; k++) {
        tmp[k] = euclidean_distance(input + i * d, input + k * d, d);
        dist[i * n + k] = tmp[k];
      }
#ifdef FINE_GRAINED_BENCH
    };
    add_measurement(&mdata, compute);
#endif
#ifdef HDBSCAN_QUICKSELECT
    core_dist[i] = iterative_quickselect(tmp, n, mpts - 1);
#else
    core_dist[i] = get_kth_neighbor(tmp, n, mpts);
#endif
  }

#ifdef FINE_GRAINED_BENCH
  print_measurement(&mdata, "(distance)");
#endif

  for (int i = 0; i < n; i++) {
    int k = 0;
    __m256d cdi = _mm256_loadu_pd(core_dist + i);
    for (; k < n - 3; k += 4) {
      __m256d cdk = _mm256_loadu_pd(core_dist + k);
      __m256d dist_vec = _mm256_loadu_pd(dist + i * n + k);
      __m256d max1 = _mm256_max_pd(cdi, cdk);
      __m256d max2 = _mm256_max_pd(max1, dist_vec);
      _mm256_storeu_pd(dist + i * n + k, max2);
    }
    for (; k < n; k++) {
      dist[i * n + k] = fmax(fmax(core_dist[i], core_dist[k]), dist[i * n + k]);
    }
  }
}

void compute_distance_matrix_triang(double *input, double *core_dist,
                                    double *dist, int mpts, int n, int d) {
  double tmp[n];
  // idea: dist matrix is of size (n*1)*n / 2 (upper triangular)
  // - the translation then is
  //   pairwise_dist[i][k] = dist[(n * i - (i * (i + 1)/2)) + k]
  // - invariant: i <= k
  // !! verify this yourself !!
  for (int i = 0; i < n; i++) {
    int k;
    for (k = 0; k < i; k++)
      tmp[k] = dist[(n * k - (k * (k + 1) / 2)) + i];
    // dist to self is zero
    dist[(n * i - (i * (i + 1) / 2)) + i] = 0;
    tmp[i] = 0;
    for (k = i + 1; k < n; k++) {
      tmp[k] = euclidean_distance(input + i * d, input + k * d, d);
      dist[(n * i - (i * (i + 1) / 2)) + k] = tmp[k];
    }
    core_dist[i] = iterative_quickselect(tmp, n, mpts - 1);
  }

  for (int i = 0; i < n; i++) {
    for (int k = i; k < n; k++) {
      dist[(n * i - (i * (i + 1) / 2)) + k] =
          fmax(fmax(core_dist[i], core_dist[k]),
               dist[(n * i - (i * (i + 1) / 2)) + k]);
    }
  }
}
