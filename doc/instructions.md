# Comparison of Skylake and Zen2 instruction latency and gap

| Intrinsic                | Skylake Latency   | Skylake Gap | Zen 2 Latency | Zen 2 Gap |
| ------------------------ | ----------------- | ----------- | ------------- | --------- |
| _mm256_set1_pd           | multi-instruction |             |               |           |
| _mm256_loadu_pd          | 7                 | 0.5         |               |           |
| _mm256_cmp_pd            | 4                 | 0.5         |               |           |
| _mm256_movemask_pd       | 2                 | 1           |               |           |
| _pdep_u32                | 3                 | 1           |               |           |
| _pext_u32                | 3                 | 1           |               |           |
| _mm256_set_epi32         | multi-instruction |             |               |           |
| _mm256_castps_pd         | not compiled      |             |               |           |
| _mm256_castpd_ps         | not compiled      |             |               |           |
| _mm256_permutevar8x32_ps | 3                 | 1           |               |           |
| _mm256_castsi256_pd      | not compiled      |             |               |           |
| _mm256_castpd_si256      | not compiled      |             |               |           |
| _mm256_maskstore_pd      | 6                 | 1           |               |           |
