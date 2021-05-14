#include "distance.h"
#include <immintrin.h>
#include <math.h>
#include <stdlib.h>

#include <stdio.h>

#ifdef HDBSCAN_INSTRUMENT
extern long hdbscan_sqrt_counter;
#endif

void euclidean_distance_2(double *pa1, double *pa2, double *pb1, double *pb2,
                          double *pc1, double *pc2, double *pd1, double *pd2,
                          double *res) {
  __m256d sum_ab;
  __m256d sum_cd;
  {
    __m256d ld1 = _mm256_loadu2_m128d(pa1, pc1);
    __m256d ld2 = _mm256_loadu2_m128d(pa2, pc2);
    __m256d diff = _mm256_sub_pd(ld1, ld2);
    sum_ab = _mm256_mul_pd(diff, diff);
  }
  {
    __m256d ld1 = _mm256_loadu2_m128d(pb1, pd1);
    __m256d ld2 = _mm256_loadu2_m128d(pb2, pd2);
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

void euclidean_distance_4(double *pa1, double *pa2, double *pb1, double *pb2,
                          double *pc1, double *pc2, double *pd1, double *pd2,
                          double *res) {
  __m256d pva1 = _mm256_load_pd(pa1);
  __m256d pva2 = _mm256_load_pd(pa2);
  __m256d diffa = _mm256_sub_pd(pva1, pva2);
  __m256d suma = _mm256_mul_pd(diffa, diffa);

  __m256d pvb1 = _mm256_load_pd(pb1);
  __m256d pvb2 = _mm256_load_pd(pb2);
  __m256d diffb = _mm256_sub_pd(pvb1, pvb2);
  __m256d sumb = _mm256_mul_pd(diffb, diffb);

  __m256d pvc1 = _mm256_load_pd(pc1);
  __m256d pvc2 = _mm256_load_pd(pc2);
  __m256d diffc = _mm256_sub_pd(pvc1, pvc2);
  __m256d sumc = _mm256_mul_pd(diffc, diffc);

  __m256d pvd1 = _mm256_load_pd(pd1);
  __m256d pvd2 = _mm256_load_pd(pd2);
  __m256d diffd = _mm256_sub_pd(pvd1, pvd2);
  __m256d sumd = _mm256_mul_pd(diffd, diffd);

  __m256d add1 = _mm256_hadd_pd(suma, sumb);
  __m256d add2 = _mm256_hadd_pd(sumc, sumd);

  __m256d perm1 = _mm256_permute4x64_pd(add1, 0b11011000);
  __m256d perm2 = _mm256_permute4x64_pd(add2, 0b11011000);

  __m256d add3 = _mm256_hadd_pd(perm1, perm2);

  __m256d sqrt = _mm256_sqrt_pd(add3);
  _mm256_storeu_pd(res, sqrt);

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter += 4;
#endif
}

void euclidean_distance_4_opt(double *base, double *p1, double *p2, double *p3,
                              double *p4, double *res) {
  __m256d base_vec = _mm256_load_pd(base);

  __m256d partial_sum1;
  __m256d partial_sum2;

  {
    __m256d p1_vec = _mm256_load_pd(p1);
    __m256d diff1 = _mm256_sub_pd(base_vec, p1_vec);
    __m256d sum1 = _mm256_mul_pd(diff1, diff1);

    __m256d pvb2 = _mm256_load_pd(p2);
    __m256d diff2 = _mm256_sub_pd(base_vec, pvb2);
    __m256d sum2 = _mm256_mul_pd(diff2, diff2);

    partial_sum1 = _mm256_hadd_pd(sum1, sum2);
  }
  {
    __m256d p3_vec = _mm256_load_pd(p3);
    __m256d diff3 = _mm256_sub_pd(base_vec, p3_vec);
    __m256d sum3 = _mm256_mul_pd(diff3, diff3);

    __m256d p4_vec = _mm256_load_pd(p4);
    __m256d diff4 = _mm256_sub_pd(base_vec, p4_vec);
    __m256d sum4 = _mm256_mul_pd(diff4, diff4);

    __m256d partial_sum2 = _mm256_hadd_pd(sum3, sum4);
  }

  __m256d perm1 = _mm256_permute4x64_pd(partial_sum1, 0b11011000);
  __m256d perm2 = _mm256_permute4x64_pd(partial_sum2, 0b11011000);

  __m256d sum = _mm256_hadd_pd(perm1, perm2);

  __m256d sqrt = _mm256_sqrt_pd(sum);
  _mm256_storeu_pd(res, sqrt);

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter += 4;
#endif
}

double euclidean_distance(double *p1, double *p2, int d) {
  __m256d sum = _mm256_setzero_pd();

  int i = 0;
  for (; i < d - 3; i += 4) {
    __m256d pv1 = _mm256_loadu_pd(p1 + i);
    __m256d pv2 = _mm256_loadu_pd(p2 + i);
    __m256d diff = _mm256_sub_pd(pv1, pv2);
    sum = _mm256_fmadd_pd(diff, diff, sum);
  }

  double buff[4];
  _mm256_storeu_pd(buff, sum);

  double res = buff[0] + buff[1] + buff[2] + buff[3];

  double d_rest;
  for (; i < d; i++) {
    d_rest = p1[i] - p2[i];
    res += d_rest * d_rest;
  }

#ifdef HDBSCAN_INSTRUMENT
  hdbscan_sqrt_counter++;
#endif

  return sqrt(res);
}

double manhattan_distance(double *p1, double *p2, int d) {
  __m256d sum = _mm256_setzero_pd();
  __m256d abs_mask =
      _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFFFFFFFFFFFFFF));

  // TODO: could load 4 at the same time to fully utilize the hadds.
  int i = 0;
  for (; i < d - 3; i += 4) {
    __m256d p1_vec = _mm256_loadu_pd(p1 + i);
    __m256d p2_vec = _mm256_loadu_pd(p2 + i);
    __m256d diff = _mm256_sub_pd(p1_vec, p2_vec);
    __m256d abs = _mm256_and_pd(diff, abs_mask);
    sum = _mm256_add_pd(sum, abs);
  }

  __m256d add = _mm256_hadd_pd(sum, sum);
  __m256d perm = _mm256_permute4x64_pd(add, 0b11011000);
  double res = _mm256_cvtsd_f64(_mm256_hadd_pd(perm, perm));

  for (; i < d; i++) {
    double d = p1[i] - p2[i];
    res += fabs(d);
  }

  return res;
}

void manhattan_distance_2(double *pa1, double *pa2, double *pb1, double *pb2,
                          double *pc1, double *pc2, double *pd1, double *pd2,
                          double *res) {
  __m256d sum_ab;
  __m256d sum_cd;
  __m256d abs_mask =
      _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFFFFFFFFFFFFFF));
  {
    __m256d ld1 = _mm256_loadu2_m128d(pa1, pc1);
    __m256d ld2 = _mm256_loadu2_m128d(pa2, pc2);
    __m256d diff = _mm256_sub_pd(ld1, ld2);
    sum_ab = _mm256_and_pd(diff, abs_mask);
  }
  {
    __m256d ld1 = _mm256_loadu2_m128d(pb1, pd1);
    __m256d ld2 = _mm256_loadu2_m128d(pb2, pd2);
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
  __m256d pa1_vec = _mm256_load_pd(pa1);
  __m256d pa2_vec = _mm256_load_pd(pa2);
  __m256d diffa = _mm256_sub_pd(pa1_vec, pa2_vec);
  __m256d absa = _mm256_and_pd(diffa, abs_mask);

  __m256d pb1_vec = _mm256_load_pd(pb1);
  __m256d pb2_vec = _mm256_load_pd(pb2);
  __m256d diffb = _mm256_sub_pd(pb1_vec, pb2_vec);
  __m256d absb = _mm256_and_pd(diffb, abs_mask);

  __m256d pc1_vec = _mm256_load_pd(pc1);
  __m256d pc2_vec = _mm256_load_pd(pc2);
  __m256d diffc = _mm256_sub_pd(pc1_vec, pc2_vec);
  __m256d absc = _mm256_and_pd(diffc, abs_mask);

  __m256d pd1_vec = _mm256_load_pd(pd1);
  __m256d pd2_vec = _mm256_load_pd(pd2);
  __m256d diffd = _mm256_sub_pd(pd1_vec, pd2_vec);
  __m256d absd = _mm256_and_pd(diffd, abs_mask);

  __m256d add1 = _mm256_hadd_pd(absa, absb);
  __m256d add2 = _mm256_hadd_pd(absc, absd);

  __m256d perm1 = _mm256_permute4x64_pd(add1, 0b11011000);
  __m256d perm2 = _mm256_permute4x64_pd(add2, 0b11011000);

  __m256d add3 = _mm256_hadd_pd(perm1, perm2);
  _mm256_storeu_pd(res, add3);
}

inline void swap(double *a, double *b) {
  double tmp = *b;
  *b = *a;
  *a = tmp;
}

int partition(double *list, int left, int right, int pivot) {
  double pivot_val = list[pivot];
  swap(list + pivot, list + right);

  int store_idx = left;

  for (int i = left; i < right; i++) {
    if (list[i] < pivot_val) {
      swap(list + store_idx, list + i);
      store_idx++;
    }
  }

  swap(list + right, list + store_idx);

  return store_idx;
}

// WARNING: this version is not entirely correct right now
int partition_vec(double *list, int left, int right, int pivot) {
  double pivot_val = list[pivot];
  swap(list + pivot, list + right);

  int store_idx = left;
  int i = left;

  __m256d pivotvec = _mm256_set1_pd(pivot_val);

  for (; i < store_idx + 4 && i < right; i++) {
    if (list[i] < pivot_val) {
      swap(list + store_idx, list + i);
      store_idx++;
    }
  }

  for (; i < right - 4; i += 4) {
    __m256d listvec = _mm256_loadu_pd(list + i);
    __m256d less = _mm256_cmp_pd(listvec, pivotvec, _CMP_LT_OQ);

    int mask = _mm256_movemask_pd(less);

    int count = __builtin_popcount(mask);

    // Unpack each bit to a byte
    unsigned int expanded_mask = _pdep_u32(mask, 0x01010101);
    // Replicate bit to fill entire byte
    expanded_mask *= 0xFF;

    const unsigned int identity_indices = 0x76543210;
    unsigned int wanted_indices = _pext_u32(identity_indices, expanded_mask);

    // TODO: is there a more elegant way to do this?
    __m256i shufmask = _mm256_set_epi32(
        (wanted_indices >> 28) & 0xF, (wanted_indices >> 24) & 0xF,
        (wanted_indices >> 20) & 0xF, (wanted_indices >> 16) & 0xF,
        (wanted_indices >> 12) & 0xF, (wanted_indices >> 8) & 0xF,
        (wanted_indices >> 4) & 0xF, wanted_indices & 0xF);

    // Unfortunately, there is no intrinsic in AVX2 to do double permutevar
    __m256d res = _mm256_castps_pd(
        _mm256_permutevar8x32_ps(_mm256_castpd_ps(listvec), shufmask));

    __m256i sm = _mm256_castpd_si256(_mm256_cmp_pd(
        _mm256_castsi256_pd(shufmask), _mm256_set1_pd(0), _CMP_NEQ_OQ));

    __m256d toswap = _mm256_maskload_pd(list + store_idx, sm);
    _mm256_maskstore_pd(list + store_idx, sm, res);

    unsigned int reverse_indices = _pdep_u32(identity_indices, expanded_mask);
    __m256i shufmask_rev = _mm256_set_epi32(
        (reverse_indices >> 28) & 0xF, (reverse_indices >> 24) & 0xF,
        (reverse_indices >> 20) & 0xF, (reverse_indices >> 16) & 0xF,
        (reverse_indices >> 12) & 0xF, (reverse_indices >> 8) & 0xF,
        (reverse_indices >> 4) & 0xF, reverse_indices & 0xF);

    __m256d rev = _mm256_castps_pd(
        _mm256_permutevar8x32_ps(_mm256_castpd_ps(toswap), shufmask_rev));

    _mm256_maskstore_pd(list + i, _mm256_castpd_si256(less), rev);

    store_idx += count;
  }

  for (; i < right; i++) {
    if (list[i] < pivot_val) {
      swap(list + store_idx, list + i);
      store_idx++;
    }
  }

  swap(list + right, list + store_idx);

  return store_idx;
}

void partition_vec2(double *list, double *lower, int *idx_lower,
                    int *idx_higher, int n, int pivot) {
  double pivot_val = list[pivot];
  int store_idx_lower = 0;
  int store_idx_higher = 0;
  int i = 0;

  swap(list + pivot, list + n - 1);

  __m256d pivotvec = _mm256_set1_pd(pivot_val);

  for (; i < n - 4; i += 4) {
    __m256d listvec = _mm256_loadu_pd(list + i);
    {
      __m256d less_mask = _mm256_cmp_pd(listvec, pivotvec, _CMP_LE_OQ);
      int mask = _mm256_movemask_pd(less_mask);
      int count = __builtin_popcount(mask);

      // Unpack each bit to a byte
      unsigned int expanded_mask = _pdep_u32(mask, 0x01010101);
      // Replicate bit to fill entire byte
      expanded_mask *= 0xFF;

      const unsigned int identity_indices = 0x76543210;
      unsigned int wanted_indices = _pext_u32(identity_indices, expanded_mask);

      // TODO: is there a more elegant way to do this?
      __m256i shufmask = _mm256_set_epi32(
          (wanted_indices >> 28) & 0xF, (wanted_indices >> 24) & 0xF,
          (wanted_indices >> 20) & 0xF, (wanted_indices >> 16) & 0xF,
          (wanted_indices >> 12) & 0xF, (wanted_indices >> 8) & 0xF,
          (wanted_indices >> 4) & 0xF, wanted_indices & 0xF);

      // Unfortunately, there is no intrinsic in AVX2 to do double permutevar
      __m256d res = _mm256_castps_pd(
          _mm256_permutevar8x32_ps(_mm256_castpd_ps(listvec), shufmask));

      __m256i sm = _mm256_castpd_si256(_mm256_cmp_pd(
          _mm256_castsi256_pd(shufmask), _mm256_set1_pd(0), _CMP_NEQ_OQ));

      _mm256_maskstore_pd(lower + store_idx_lower, sm, res);

      store_idx_lower += count;
    }
    {
      __m256d less_mask = _mm256_cmp_pd(listvec, pivotvec, _CMP_GT_OQ);
      int mask = _mm256_movemask_pd(less_mask);
      int count = __builtin_popcount(mask);

      // Unpack each bit to a byte
      unsigned int expanded_mask = _pdep_u32(mask, 0x01010101);
      // Replicate bit to fill entire byte
      expanded_mask *= 0xFF;

      const unsigned int identity_indices = 0x76543210;
      unsigned int wanted_indices = _pext_u32(identity_indices, expanded_mask);

      // TODO: is there a more elegant way to do this?
      __m256i shufmask = _mm256_set_epi32(
          (wanted_indices >> 28) & 0xF, (wanted_indices >> 24) & 0xF,
          (wanted_indices >> 20) & 0xF, (wanted_indices >> 16) & 0xF,
          (wanted_indices >> 12) & 0xF, (wanted_indices >> 8) & 0xF,
          (wanted_indices >> 4) & 0xF, wanted_indices & 0xF);

      // Unfortunately, there is no intrinsic in AVX2 to do double permutevar
      __m256d res = _mm256_castps_pd(
          _mm256_permutevar8x32_ps(_mm256_castpd_ps(listvec), shufmask));

      __m256i sm = _mm256_castpd_si256(_mm256_cmp_pd(
          _mm256_castsi256_pd(shufmask), _mm256_set1_pd(0), _CMP_NEQ_OQ));

      _mm256_maskstore_pd(list + store_idx_higher, sm, res);
      store_idx_higher += count;
    }
  }

  for (; i < n - 1; i++) {
    if (list[i] <= pivot_val) {
      lower[store_idx_lower] = list[i];
      store_idx_lower++;
    }
    if (list[i] > pivot_val) {
      list[store_idx_higher] = list[i];
      store_idx_higher++;
    }
  }

  *idx_higher = store_idx_higher;
  *idx_lower = store_idx_lower;
}

double iterative_quickselect(double *list, int n, int k) {
  double lo[n];
  double *lower = lo;
  int pivot_idx = n / 2;

  if (k >= n) {
    return NAN;
  }

  while (true) {
    if (n == 1) {
      return list[0];
    }

    // Take element in the middle as pivot
    double pivot_val = list[pivot_idx];
    int store_idx_lower;
    int store_idx_higher;
    partition_vec2(list, lower, &store_idx_lower, &store_idx_higher, n,
                   pivot_idx);

    if (store_idx_lower == k) {
      return pivot_val;
    } else if (store_idx_lower > k) {
      double *tmp = list;
      list = lower;
      lower = tmp;
      n = store_idx_lower;
    } else {
      k -= store_idx_lower + 1;
      n = store_idx_higher;
    }
    pivot_idx = n / 2;
  }
}

void compute_core_distances(double *input, double *core_dist, int mpts, int n,
                            int d) {
  double distances[n];

  for (int k = 0; k < n; k++) {
    int i = 0;
    for (; i < n - 3; i += 4) {
      if (d == 2) {
        euclidean_distance_2(input + k * d, input + i * d, input + k * d,
                             input + (i + 1) * d, input + k * d,
                             input + (i + 2) * d, input + k * d,
                             input + (i + 3) * d, distances + i);
      } else if (d == 4) {
        euclidean_distance_4_opt(input + k * d, input + i * d,
                                 input + (i + 1) * d, input + (i + 2) * d,
                                 input + (i + 3) * d, distances + i);
      } else {
        break;
      }
    }
    for (; i < n; i++) {
      distances[i] = euclidean_distance(input + i * d, input + k * d, d);
    }

    core_dist[k] = iterative_quickselect(distances, n, mpts - 1);
  }
}

void compute_distance_matrix(double *input, double *core_dist, double *dist,
                             int mpts, int n, int d) {
  double tmp[n];

  for (int i = 0; i < n; i++) {
    int k = 0;
    for (; k < n - 3; k += 4) {
      if (d == 2) {
        euclidean_distance_2(input + i * d, input + k * d, input + i * d,
                             input + (k + 1) * d, input + i * d,
                             input + (k + 2) * d, input + i * d,
                             input + (k + 3) * d, tmp + k);
      } else if (d == 4) {
        euclidean_distance_4_opt(input + i * d, input + k * d,
                                 input + (k + 1) * d, input + (k + 2) * d,
                                 input + (k + 3) * d, tmp + k);
      } else {
        break;
      }

      __m256d d = _mm256_loadu_pd(tmp + k);
      _mm256_storeu_pd(dist + i * n + k, d);
    }
    for (; k < n; k++) {
      tmp[k] = euclidean_distance(input + i * d, input + k * d, d);
      dist[i * n + k] = tmp[k];
    }
    core_dist[i] = iterative_quickselect(tmp, n, mpts - 1);
  }

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
