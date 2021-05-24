# Comparison of Skylake and Zen2 instruction latency and gap

| Intrinsic                | Skylake Latency   | Skylake Gap | Zen 2 Latency | Zen 2 Gap |
| ------------------------ | ----------------- | ----------- | ------------- | --------- |
| _mm256_set1_pd           | multi-instruction |             |               |           |
| _mm256_loadu_pd          | 7                 | 0.5         | 5 / 8         | 0.5       |
| _mm256_cmp_pd            | 4                 | 0.5         | 1             | 0.5       |
| _mm256_movemask_pd       | 2                 | 1           | 5 / 7         | 1         |
| _pdep_u32                | 3                 | 1           | 19 / 18       | 19        |
| _pext_u32                | 3                 | 1           | 19 / 18       | 19        |
| _mm256_set_epi32         | multi-instruction |             |               |           |
| _mm256_castps_pd         | not compiled      |             |               |           |
| _mm256_castpd_ps         | not compiled      |             |               |           |
| _mm256_permutevar8x32_ps | 3                 | 1           | 3/8           | 2         |
| _mm256_castsi256_pd      | not compiled      |             |               |           |
| _mm256_castpd_si256      | not compiled      |             |               |           |
| _mm256_maskstore_pd      | 6                 | 1           | 23 / 4        | 12 / 6    |


Information for Zen 2 was taken from [1] [Agner](https://www.agner.org/optimize/instruction_tables.pdf) and [2] [uops.info](https://uops.info). If they differed, both numbers were inserted here in the form [1]/[2].