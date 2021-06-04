#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $(basename $0) [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|gcc-v-clang|mpts|all]"
    exit 1
fi

if [ ! -z $1 ] && [ $1 != "amd" ] && [ $1 != "intel" ]; then
    echo "Invalid argument: $1" >&2
    echo ""
    echo "Usage: $(basename $0) [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|gcc-v-clang|mpts|all]"
    exit 1
fi

# extend here with different test names
if [ ! -z $2 ] && [ $2 != "basic" ] && [ $2 != "advanced" ] && \
   [ $2 != "amd-v-intel" ] && [ $2 != "blocked-v-triangular" ] && \
   [ $2 != "reference" ] && [ $2 != "gcc-v-clang" ] && [ $2 != "mpts" ] && \
   [ $2 != "dimensions" ] && [ $2 != "all" ]; then
    echo "Invalid argument: $2" >&2
    echo ""
    echo "Usage: $(basename $0) [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|gcc-v-clang|mpts|all]"
    exit 1
fi

AMD=0
if [ $1 = "amd" ]; then
    AMD=1
fi

TIME=$(date +%Y%m%d_%H%M%S)
# C_COMPILER=/usr/bin/clang
# CXX_COMPILER=/usr/bin/clang++
C_COMPILER=clang-11
CXX_COMPILER=clang++-11
# data creation here first

# create build dir
mkdir -p build
mkdir -p plots/${TIME}

##########################################################
######## Total Comparison of all optimizations    ########
##########################################################

# Total Comparison flops/cycles d=4
N=12
D=4

# Basic everything
if [ $2 = "basic" ] || [ $2 = "advanced" ] ||  [ $2 = "reference" ] || [ $2 = "mpts" ] || [ $2 = "all" ]; then
    python helper_scripts/generate_clusters.py data 6 ${D}
fi

##########################################################
######## Reference    ##########
##########################################################
if [ $2 = "reference" ] || [ $2 = "all" ]; then
    cd references/hdbscan-cpp && make clean
    if [ $1 = "amd" ]; then
        make CXX=${CXX_COMPILER} CPPFLAGS="-DBENCHMARK_AMD -march=native"
    else
        make CXX=${CXX_COMPILER} CPPFLAGS="march=native"
    fi
    cd ../..
    # a bit hacky, but our script assumes binary is in build
    ./run_perf_measurements.sh reference_hdbscan ../../references/hdbscan-cpp/main perf_data_d${D} ${N} ${TIME}
fi

##########################################################
######## Baseline    ##########
##########################################################

if [ $2 = "basic" ] || [ $2 = "all" ]; then
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3" \
        -DPACKLEFT_WLOOKUP=1 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        cd ..
    # No optimizations
    ./run_perf_measurements.sh basic_O3 hdbscan_basic_benchmark perf_data_d${D} ${N} ${TIME}

fi


################################################################################
### Plot with mpts on the x axis
################################################################################

if [ $2 = "mpts" ] || [ $2 = "all" ]; then
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DBENCHMARK_AMD=${AMD} \
        -DHDBSCAN_QUICKSELECT=0 &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

  ./run_perf_measurements.sh quickselect_baseline hdbscan_benchmark perf_data_d${D} 7 ${TIME} 4
  ./run_perf_measurements.sh bubbleselect hdbscan_benchmark_distvec_quickvec perf_data_d${D} 7 ${TIME} 4
  for i in $(seq 20 40 80); do
    ./run_perf_measurements.sh quickselect_baseline hdbscan_benchmark perf_data_d${D} 7 ${TIME} ${i}
    ./run_perf_measurements.sh bubbleselect hdbscan_benchmark_distvec_quickvec perf_data_d${D} 7 ${TIME} ${i}
  done

    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DBENCHMARK_AMD=${AMD} \
        -DHDBSCAN_QUICKSELECT=1 &&
        ninja build_bench_vec &&
        cd ..

  ./run_perf_measurements.sh quickselect_vectorized hdbscan_benchmark_distvec_quickvec perf_data_d${D} 7 ${TIME} 4
  for i in $(seq 20 40 80); do
    ./run_perf_measurements.sh quickselect_vectorized hdbscan_benchmark_distvec_quickvec perf_data_d${D} 7 ${TIME} ${i}
  done

  python helper_scripts/plot_performance_alt.py --system $1  \
      --data-path data/timings/${TIME} \
      --files quickselect_baseline.csv \
              bubbleselect.csv \
              quickselect_vectorized.csv \
      --save-path plots/${TIME}/cycles_mpts.png \
      --metric=cycles \
      --x-scale=linear
fi

if [ $2 != "mpts" ]; then

cd build && cmake -G Ninja .. \
    -DCMAKE_C_COMPILER=${C_COMPILER} \
    -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
    -DCMAKE_CXX_FLAGS="-O3 -march=native" \
    -DPACKLEFT_WLOOKUP=1 \
    -DBENCHMARK_AMD=${AMD} &&
    ninja build_bench &&
    ninja build_bench_vec &&
    cd ..

fi


##########################################################
######## Algorithm with basic prim algorithm    ##########
##########################################################
if [ $2 = "basic" ] || [ $2 = "all" ]; then
    # No optimizations
    ./run_perf_measurements.sh basic hdbscan_basic_benchmark perf_data_d${D} ${N} ${TIME}
    # Adding optimized distance computations (computation of adjacency matrix)
    ./run_perf_measurements.sh basic_distvec hdbscan_basic_benchmark_distvec perf_data_d${D} ${N} ${TIME}
    # Adding optimized quickselect algorithm
    ./run_perf_measurements.sh basic_distvec_quickvec hdbscan_basic_benchmark_distvec_quickvec perf_data_d${D} ${N} ${TIME}
    # Adding optimized prim algorithm
    ./run_perf_measurements.sh basic_distvec_quickvec_primvec hdbscan_basic_benchmark_distvec_quickvec_primvec perf_data_d${D} ${N} ${TIME}

    # Plot result
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic_O3.csv \
                basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec.csv \
                basic_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/performance_basic.png
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic_O3.csv \
                basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec.csv \
                basic_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/performance_basic_linear.png \
        --x-scale=linear
fi

##########################################################
######## Algorithm with advanced prim algorithm ##########
##########################################################
if [ $2 = "advanced" ] || [ $2 = "all" ]; then
    # No optimizations
    ./run_perf_measurements.sh advprim hdbscan_benchmark perf_data_d${D} ${N} ${TIME}
    # Adding optimized distance computation (computation of core_distances)
    ./run_perf_measurements.sh advprim_distvec hdbscan_benchmark_distvec perf_data_d${D} ${N} ${TIME}
    # Adding optimized quickselect algorithm
    ./run_perf_measurements.sh advprim_distvec_quickvec hdbscan_benchmark_distvec_quickvec perf_data_d${D} ${N} ${TIME}
    # Adding optimized prim algorithm
    ./run_perf_measurements.sh advprim_distvec_quickvec_primvec hdbscan_benchmark_distvec_quickvec_primvec perf_data_d${D} ${N} ${TIME}

    # Plot result
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim.csv \
                advprim_distvec.csv \
                advprim_distvec_quickvec.csv \
                advprim_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/performance_advprim.png
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim.csv \
                advprim_distvec.csv \
                advprim_distvec_quickvec.csv \
                advprim_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/performance_advprim_linear.png \
        --x-scale=linear
fi

##########################################################
######## Plots showing cycles of each binary \  ##########
######## for basic & advanced prim              ##########
##########################################################
if [ $2 = "basic" ] || [ $2 = "all" ]; then
    # Plot basic 
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec.csv \
                basic_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/cycles_basic.png \
        --metric=cycles \
        --x-scale=linear
fi

if [ $2 = "advanced" ] || [ $2 = "all" ]; then
    # Plot advprim
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim.csv \
                advprim_distvec.csv \
                advprim_distvec_quickvec.csv \
                advprim_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/cycles_advprim.png \
        --metric=cycles \
        --x-scale=linear
fi

if [ $2 = "all" ]; then
    # Plot basic vs advprim
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic.csv \
                basic_distvec_quickvec.csv \
                advprim.csv \
                advprim_distvec_quickvec.csv  \
        --save-path plots/${TIME}/performance_basic_vs_advanced.png \
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic.csv \
                basic_distvec_quickvec.csv \
                advprim.csv \
                advprim_distvec_quickvec.csv  \
        --save-path plots/${TIME}/performance_basic_vs_advanced_linear.png \
        --x-scale=linear 

    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic.csv \
                basic_distvec_quickvec.csv \
                advprim.csv \
                advprim_distvec_quickvec.csv  \
        --save-path plots/${TIME}/cycles_basic_vs_advanced.png \
        --metric=cycles \
        --x-scale=linear
    
    # Plot reference vs ours
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files reference_hdbscan.csv \
                basic.csv basic_distvec_quickvec_primvec.csv \
                advprim_distvec.csv \
        --save-path plots/${TIME}/cycles_ref_vs_ours.png --x-scale=linear --metric=cycles
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files reference_hdbscan.csv \
                basic.csv basic_distvec_quickvec_primvec.csv \
                advprim_distvec.csv \
        --save-path plots/${TIME}/performance_ref_vs_ours.png --x-scale=linear
fi

if [ $2 = "basic" ] || [ $2 = "advanced" ] || [ $2 == "reference" ] || [ $2 = "mpts" ] || [ $2 = "all" ]; then
    # Clean up datasets used for this part
    rm ./data/perf_data_d${D}_*
fi



##########################################################
######## Comparison between Intel and AMD         ########
######## with focus on the vectorization          ########
######## of the partition function                ########
##########################################################

# Total Comparison flops/cycles d=128
N=12
D=8
if [ $2 = "amd-v-intel" ] || [ $2 = "all" ]; then
    printf "Running amd-v-intel benchmarks. Creating data...\n"
    # First with the version where `pext` and `pdep` are used
    python helper_scripts/generate_clusters.py data 6 ${D}
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

    printf "Running amd-v-intel benchmarks. Run benchmark with pext/pdep partitioning...\n"
    # Best performing 
    ./run_perf_measurements.sh $1_pext_partition hdbscan_benchmark_distvec_quickvec perf_data_d8 ${N} ${TIME}

    # Then the optimized version for AMD, using a lookup table
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

    printf "Running amd-v-intel benchmarks. Run benchmark with LUT partitioning...\n"
    # Best performing 
    ./run_perf_measurements.sh $1_lut_partition hdbscan_benchmark_distvec_quickvec perf_data_d8 ${N} ${TIME}

    printf "Running amd-v-intel benchmarks. Run benchmark without vectorization...\n"
    ./run_perf_measurements.sh $1_no_vec hdbscan_benchmark perf_data_d8 ${N} ${TIME}

    # Plot like this here (should be done with second machine, i.e.
    # benchmarks have been copied) 
    : ' python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files amd_pext_partition.csv \
                amd_lut_partition.csv \
                intel_pext_partition.csv \
                intel_lut_partition.csv \
        --save-path plots/perf_amd_vs_intel.png \
        --metric=cycles
        --x-scale=linear
    '
    # Remove data used for this experiment
    rm ./data/perf_data_d8_*
fi


#######################################################################################
######## Comparison between basic, triangular and blocked distance computation ########
#######################################################################################
# Total Comparison flops/cycles d=128
N=12

if [ $2 = "blocked-v-triangular" ] || [ $2 = "all" ]; then
    printf "Running blocked-v-triangular benchmarks. Creating data...\n"
    python helper_scripts/generate_clusters.py data 6 20
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -DHDBSCAN_BLOCK_SIZE=80" \
        -DPACKLEFT_WLOOKUP=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_TRIANG=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_BLOCKED=0 \
        -DHDBSCAN_VERBOSE=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        cd ..

    printf "Running blocked-v-triangular benchmarks. Run benchmark basic version...\n"
    # Basic
    ./run_perf_measurements.sh distance_matrix_basic hdbscan_basic_benchmark perf_data_d20 ${N} ${TIME}

    ./run_perf_measurements.sh distance_matrix_triangular hdbscan_basic_benchmark_triang perf_data_d20 ${N} ${TIME}

    ./run_perf_measurements.sh distance_matrix_blocked hdbscan_basic_benchmark_blocked perf_data_d20 ${N} ${TIME}

    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files distance_matrix_basic.csv \
                distance_matrix_triangular.csv \
                distance_matrix_blocked.csv \

        --save-path plots/triangular_v_blocked.png \
        --metric=cycles \
        --x-scale=linear

    # Remove data used for this experiment
    rm -f ./data/perf_data_d20_*
fi

########################################################################
######## Comparison of gcc vs. clang on most optimized versions ########
########################################################################

if [ $2 = "gcc-v-clang" ] || [ $2 = "all" ]; then
    N=12
    D=4
    # Generate data
    python helper_scripts/generate_clusters.py data 6 ${D}

    # clang-11

    ./run_perf_measurements.sh clang_basic_best hdbscan_basic_benchmark_distvec_quickvec_primvec perf_data_d4 ${N} ${TIME}
    ./run_perf_measurements.sh clang_advprim_best hdbscan_benchmark_distvec_quickvec perf_data_d4 ${N} ${TIME}


    # gcc
    # Needed, otherwise problems with compiling with new compiler
    rm -rf build
    mkdir -p build

    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=cc \
        -DCMAKE_CXX_COMPILER=c++ \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

    ./run_perf_measurements.sh gcc_basic_best hdbscan_basic_benchmark_distvec_quickvec_primvec  perf_data_d4 ${N} ${TIME}
    ./run_perf_measurements.sh gcc_advprim_best hdbscan_benchmark_distvec_quickvec  perf_data_d4 ${N} ${TIME}

    # Plotting
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files gcc_basic_best.csv \
                gcc_advprim_best.csv \
                clang_basic_best.csv \
                clang_advprim_best.csv  \
        --save-path plots/${TIME}/gcc_v_clang.png
fi

#####################################################
######## Comparison for different dimensions ########
#####################################################

if [ $2 = "dimensions" ] || [ $2 = "all" ]; then
    N=12
    D=8
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench_vec &&
        cd ..
    for i in $(seq 1 ${D}); do
        d=$((2 ** ${i}))
        python helper_scripts/generate_clusters.py data 6 ${d}
        printf "running for d = ${d}...\n"
        ./run_perf_measurements.sh advprim_distvec_quickvec_dims${d} hdbscan_benchmark_distvec_quickvec perf_data_d${d} ${N} ${TIME}
        rm ./data/perf_data_d${d}*
    done
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_distvec_quickvec_dims2.csv \
                advprim_distvec_quickvec_dims4.csv \
                advprim_distvec_quickvec_dims8.csv \
                advprim_distvec_quickvec_dims16.csv \
                advprim_distvec_quickvec_dims32.csv \
                advprim_distvec_quickvec_dims64.csv \
                advprim_distvec_quickvec_dims128.csv \
        --save-path cycles_dims.png \
        --metric=cycles \
        --x-scale=linear
    
    python helper_scripts/plot_performance_dims.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_distvec_quickvec_dims2.csv \
                advprim_distvec_quickvec_dims4.csv \
                advprim_distvec_quickvec_dims8.csv \
                advprim_distvec_quickvec_dims16.csv \
                advprim_distvec_quickvec_dims32.csv \
                advprim_distvec_quickvec_dims64.csv \
                advprim_distvec_quickvec_dims128.csv \
        --save-path perf_dims_heatmap.png

fi
