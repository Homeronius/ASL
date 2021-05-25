# Comparison of Skylake and Zen2 instruction latency and gap

| Intrinsic                | Instruction                    | Skylake Latency | Skylake Gap | Zen 2 Latency | Zen 2 Gap |
| ------------------------ | ------------------------------ | --------------- | ----------- | ------------- | --------- |
| _mm256_set1_pd           | multi-instruction              |                 |             |               |           |
| _mm256_loadu_pd          | `vmovupd ymm, m256`            | 7               | 0.5         | 5 / 8         | 0.5       |
| _mm256_cmp_pd            | `vcmppd ymm, ymm, ymm, imm8`   | 4               | 0.5         | 1             | 0.5       |
| _mm256_movemask_pd       | `vmovmskpd r32, ymm `          | 2               | 1           | 5 / 7         | 1         |
| _pdep_u32                | `pdep r32, r32, r32 `          | 3               | 1           | 19 / 18       | 19        |
| _pext_u32                | `pext r32, r32, r32`           | 3               | 1           | 19 / 18       | 19        |
| _mm256_set_epi32         | multi-instruction              |                 |             |               |           |
| _mm256_castps_pd         | not compiled                   |                 |             |               |           |
| _mm256_castpd_ps         | not compiled                   |                 |             |               |           |
| _mm256_permutevar8x32_ps | `vpermps ymm, ymm, ymm`        | 3               | 1           | 3 or 8        | 2         |
| _mm256_castsi256_pd      | not compiled                   |                 |             |               |           |
| _mm256_castpd_si256      | not compiled                   |                 |             |               |           |
| _mm256_maskstore_pd      | `vmaskmovpd m256, ymm, ymm`    | 6               | 1           | 23 / 4        | 12 / 6    |
| _mm256_add_pd            | `vaddpd ymm, ymm, ymm`         | 4               | 0.5         |               |           |
| _mm256_hadd_pd           | `vhaddpd ymm, ymm, ymm`        | 7               | 2           |               |           |
| _mm256_permute_pd        | `vpermilpd ymm, ymm, imm8`     | 1               | 1           |               |           |
| _mm256_permute4x64_pd    | `vpermpd ymm, ymm, imm8`       | 3               | 1           |               |           |
| _mm256_blend_pd          | `vblendpd ymm, ymm, ymm, imm8` | 1               | 0.33        |               |           |

Information for Skylake was taken from the [Intel intrinsics guide](https://software.intel.com/sites/landingpage/IntrinsicsGuide/)

Information for Zen 2 was taken from [1] [Agner](https://www.agner.org/optimize/instruction_tables.pdf) and [2] [uops.info](https://uops.info). If they differed, both numbers were inserted here in the form [1]/[2].
