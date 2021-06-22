#!/bin/bash

USAGE="Usage: plot_all [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|gcc-v-clang|mpts|data_variance|all] [time] [file type]"

if [ $# -ne 4 ]; then
    echo $USAGE
    exit 1
fi

if [ ! -z $1 ] && [ $1 != "amd" ] && [ $1 != "intel" ]; then
    echo "Invalid argument: $1" >&2
    echo ""
    echo $USAGE
    exit 1
fi

# extend here with different test names
if [ ! -z $2 ] && [ $2 != "basic" ] && [ $2 != "advanced" ] && \
   [ $2 != "amd-v-intel" ] && [ $2 != "blocked-v-triangular" ] && \
   [ $2 != "reference" ] && [ $2 != "gcc-v-clang" ] && [ $2 != "mpts" ] && \
   [ $2 != "dimensions" ] && [ $2 != "data_variance" ] && \
   [ $2 != "all" ]; then
    echo "Invalid argument: $2" >&2
    echo ""
    echo $USAGE
    exit 1
fi

TIME=$3
PATH_IN=./measurements/martin/${TIME}
PATH_OUT=./plots/martin/${TIME}

EXT=$4

AMD=0
if [ $1 = "amd" ]; then
    AMD=1
    PATH_IN=./measurements/alex/${TIME}
    PATH_OUT=./plots/alex/${TIME}
fi


################################################################################
### Plot with mpts on the x axis
################################################################################
if [ $2 = "mpts" ] || [ $2 = "all" ]; then
  python helper_scripts/plot_performance_alt.py --system $1  \
      --data-path ${PATH_IN} \
      --files quickselect_baseline.csv \
              bubbleselect.csv \
              quickselect_vectorized.csv \
      --save-path ${PATH_OUT}/runtime_mpts.${EXT} \
      --metric=cycles \
      --x-scale=linear

  python helper_scripts/plot_performance_alt.py --system $1  \
      --data-path ${PATH_IN} \
      --files quickselect_baseline.csv \
              bubbleselect.csv \
              quickselect_vectorized.csv \
      --save-path ${PATH_OUT}/perf_mpts.${EXT} \
      --x-scale=linear
fi

################################################################################
### Algorithm with basic prim algorithm
################################################################################
if [ $2 = "basic" ] || [ $2 = "all" ]; then
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec.csv \
                basic_distvec_quickvec_primvec.csv  \
        --save-path ${PATH_OUT}/performance_basic.${EXT}

    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec.csv \
                basic_distvec_quickvec_primvec.csv  \
        --save-path ${PATH_OUT}/performance_basic_linear.${EXT} \
        --x-scale=linear
fi

################################################################################
### Algorithm with advanced prim algorithm
################################################################################
if [ $2 = "advanced" ] || [ $2 = "all" ]; then
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files advprim.csv \
                advprim_distvec.csv \
                advprim_distvec_quickvec.csv \
                advprim_distvec_quickvec_primvec.csv  \
        --save-path ${PATH_OUT}/performance_advprim.${EXT}
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files advprim.csv \
                advprim_distvec.csv \
                advprim_distvec_quickvec.csv \
                advprim_distvec_quickvec_primvec.csv  \
        --save-path ${PATH_OUT}/performance_advprim_linear.${EXT} \
        --x-scale=linear
fi

################################################################################
### Plots showing cycles of each binary for basic & advanced prim
################################################################################
if [ $2 = "basic" ] || [ $2 = "all" ]; then
    # Plot basic 
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec.csv \
                basic_distvec_quickvec_primvec.csv  \
        --save-path ${PATH_OUT}/cycles_basic.${EXT} \
        --metric=cycles \
        --x-scale=linear
fi

if [ $2 = "advanced" ] || [ $2 = "all" ]; then
    # Plot advprim
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files advprim.csv \
                advprim_distvec.csv \
                advprim_distvec_quickvec.csv \
                advprim_distvec_quickvec_primvec.csv  \
        --save-path ${PATH_OUT}/cycles_advprim.${EXT} \
        --metric=cycles \
        --x-scale=linear
fi

if [ $2 = "all" ]; then
    # Plot basic vs advprim
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files basic.csv \
                basic_distvec_quickvec.csv \
                advprim.csv \
                advprim_distvec_quickvec.csv  \
        --save-path ${PATH_OUT}/performance_basic_vs_advanced.${EXT} \
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files basic.csv \
                basic_distvec_quickvec.csv \
                advprim.csv \
                advprim_distvec_quickvec.csv  \
        --save-path ${PATH_OUT}/performance_basic_vs_advanced_linear.${EXT} \
        --x-scale=linear 

    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files basic.csv \
                basic_distvec_quickvec.csv \
                advprim.csv \
                advprim_distvec_quickvec.csv  \
        --save-path ${PATH_OUT}/cycles_basic_vs_advanced.${EXT} \
        --metric=cycles \
        --x-scale=linear
    
    # Plot reference vs ours
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files reference_hdbscan.csv \
                basic.csv basic_distvec_quickvec_primvec.csv \
                advprim_distvec.csv \
        --save-path ${PATH_OUT}/cycles_ref_vs_ours.${EXT} --x-scale=linear --metric=cycles

    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files reference_hdbscan.csv \
                basic.csv basic_distvec_quickvec_primvec.csv \
                advprim_distvec.csv \
        --save-path ${PATH_OUT}/performance_ref_vs_ours.${EXT} --x-scale=linear
fi

################################################################################
### Comparison between Intel and AMD with focus on the vectorization
### of the partition function
################################################################################
if [ $2 = "amd-v-intel" ] || [ $2 = "all" ]; then
    # Plot like this here (should be done with second machine, i.e.
    # benchmarks have been copied) 
    : ' python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files amd_pext_partition.csv \
                amd_lut_partition.csv \
                intel_pext_partition.csv \
                intel_lut_partition.csv \
        --save-path plots/perf_amd_vs_intel.${EXT} \
        --metric=cycles
        --x-scale=linear
    '
fi


################################################################################
### Comparison between basic, triangular and blocked distance computation
################################################################################
# Total Comparison flops/cycles d=128
if [ $2 = "blocked-v-triangular" ] || [ $2 = "all" ]; then
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files distance_matrix_basic.csv \
                distance_matrix_triangular.csv \
                distance_matrix_blocked_20.csv \
                distance_matrix_blocked_40.csv \
                distance_matrix_blocked_80.csv \
                distance_matrix_blocked_100.csv \
        --save-path=${PATH_OUT}/triangular_v_blocked.${EXT} \
        --metric=cycles \
        --x-scale=linear
fi

################################################################################
### Comparison of gcc vs. clang on most optimized versions
################################################################################
if [ $2 = "gcc-v-clang" ] || [ $2 = "all" ]; then
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files gcc_basic_best.csv \
                gcc_advprim_best.csv \
                clang_basic_best.csv \
                clang_advprim_best.csv  \
        --save-path ${PATH_OUT}/gcc_v_clang.${EXT}
fi


################################################################################
### Comparison for different dimensions
################################################################################
if [ $2 = "dimensions" ] || [ $2 = "all" ]; then
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files advprim_distvec_quickvec_dims2.csv \
                advprim_distvec_quickvec_dims4.csv \
                advprim_distvec_quickvec_dims8.csv \
                advprim_distvec_quickvec_dims16.csv \
                advprim_distvec_quickvec_dims32.csv \
                advprim_distvec_quickvec_dims64.csv \
                advprim_distvec_quickvec_dims128.csv \
                advprim_distvec_quickvec_dims256.csv \
        --save-path ${PATH_OUT}/cycles_dims.${EXT} \
        --metric=cycles \
        --x-scale=linear
    
    python helper_scripts/plot_performance_dims.py --system $1  \
        --data-path ${PATH_IN} \
        --files advprim_distvec_quickvec_dims2.csv \
                advprim_distvec_quickvec_dims4.csv \
                advprim_distvec_quickvec_dims8.csv \
                advprim_distvec_quickvec_dims16.csv \
                advprim_distvec_quickvec_dims32.csv \
                advprim_distvec_quickvec_dims64.csv \
                advprim_distvec_quickvec_dims128.csv \
                advprim_distvec_quickvec_dims256.csv \
        --save-path ${PATH_OUT}/perf_dims_heatmap.${EXT}
fi

################################################################################
### Comparison for different data
################################################################################
if [ $2 = "data_variance" ] || [ $2 = "all" ]; then
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path ${PATH_IN} \
        --files advprim_vec_blobs1.csv \
                advprim_vec_blobs2.csv \
                advprim_vec_blobs3.csv \
                advprim_vec_blobs4.csv \
                basic_best_blobs1.csv \
                basic_best_blobs2.csv \
                basic_best_blobs3.csv \
                basic_best_blobs4.csv \
        --save-path ${PATH_OUT}/performance_data_variance.${EXT} \
        --x-scale=linear
fi
