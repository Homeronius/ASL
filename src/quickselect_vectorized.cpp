#include "distance.h"
#include <immintrin.h>
#include <math.h>
#include <stdint.h>

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

/* LOOKUP TABLE implementation for
   storing a vector based on a mask contigously
   in memory (pack-left with mask).
   The idea and code is adapted from
   https://stackoverflow.com/questions/36932240/avx2-what-is-the-most-efficient-way-to-pack-left-based-on-a-mask
*/

uint8_t g_pack_left_table_u8x3[256 * 3 + 1];

uint32_t get_nth_bits(int a) {
  uint32_t out = 0;
  int c = 0;
  for (int i = 0; i < 8; ++i) {
    auto set = (a >> i) & 1;
    if (set) {
      out |= (i << (c * 3));
      c++;
    }
  }
  return out;
}

// Generate Move mask via: _mm256_movemask_ps(_mm256_castsi256_ps(mask)); etc
__m256i MoveMaskToIndices(uint32_t moveMask) {
  uint8_t *adr = g_pack_left_table_u8x3 + moveMask * 3;
  __m256i indices = _mm256_set1_epi32(
      *reinterpret_cast<uint32_t *>(adr)); // lower 24 bits has our LUT
  __m256i shufmask =
      _mm256_srlv_epi32(indices, _mm256_setr_epi32(0, 3, 6, 9, 12, 15, 18, 21));
  return shufmask;
}

void BuildPackMask() {
  for (int i = 0; i < 256; ++i) {
    *reinterpret_cast<uint32_t *>(&g_pack_left_table_u8x3[i * 3]) =
        get_nth_bits(i);
  }
}

void partition_vec_lut(double *list, double *lower, int *idx_lower,
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
      int mask = _mm256_movemask_ps(_mm256_castpd_ps(less_mask));
      // popcount is always divisible by 2
      int count = __builtin_popcount(mask) / 2;
      __m256i shufmask = MoveMaskToIndices(mask);

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
      int mask = _mm256_movemask_ps(_mm256_castpd_ps(less_mask));
      // popcount is always divisible by 2
      int count = __builtin_popcount(mask) / 2;
      __m256i shufmask = MoveMaskToIndices(mask);

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

// only called with iterative_quickselect(list, n, mpts - 1);
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
#ifndef PACKLEFT_WLOOKUP
    partition_vec2(list, lower, &store_idx_lower, &store_idx_higher, n,
                   pivot_idx);
#else
    partition_vec_lut(list, lower, &store_idx_lower, &store_idx_higher, n,
                      pivot_idx);
#endif

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
