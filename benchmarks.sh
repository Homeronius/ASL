#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $(basename $0) [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|all]"
    exit 1
fi

if [ ! -z $1 ] && [ $1 != "amd" ] && [ $1 != "intel" ]; then
    echo "Invalid argument: $1" >&2
    echo ""
    echo "Usage: $(basename $0) [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|all]"
    exit 1
fi

# extend here with different test names
if [ ! -z $2 ] && [ $2 != "basic" ] && [ $2 != "advanced" ] && \
   [ $2 != "amd-v-intel" ] && [ $2 != "blocked-v-triangular" ] && \
   [ $2 != "reference" ] && [ $2 != "all" ]; then
    echo "Invalid argument: $2" >&2
    echo ""
    echo "Usage: $(basename $0) [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|all]"
    exit 1
fi

AMD=0
if [ $1 = "amd" ]; then
    AMD=1
fi

TIME=$(date +%Y%m%d_%H%M%S)
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
if [ $2 = "basic" ] || [ $2 = "advanced" ] ||  [ $2 = "reference" ] || [ $2 = "all" ]; then
    python helper_scripts/generate_clusters.py data 6 ${D}
fi

##########################################################
######## Reference    ##########
##########################################################
if [ $2 = "reference" ] || [ $2 = "all" ]; then
    cd references/hdbscan-cpp && make clean
    if [ $1 = "amd" ]; then
        make CXX=clang++-11 CPPFLAGS="-DBENCHMARK_AMD -march=native"
    else
        make CXX=clang++-11 CPPFLAGS="march=native"
    fi
    cd ../..
    # a bit hacky, but our script assumes binary is in build
    ./run_perf_measurements.sh reference_hdbscan ../../references/hdbscan-cpp/main perf_data_d${D} ${N} ${TIME}
fi

##########################################################
######## Baseline    ##########
##########################################################

cd build && cmake -G Ninja .. \
    -DCMAKE_C_COMPILER=clang-11 \
    -DCMAKE_CXX_COMPILER=clang++-11 \
    -DCMAKE_CXX_FLAGS="-O3" \
    -DPACKLEFT_WLOOKUP=1 \
    -DBENCHMARK_AMD=${AMD} &&
    ninja build_bench &&
    cd ..


if [ $2 = "basic" ] || [ $2 = "all" ]; then
    # No optimizations
    ./run_perf_measurements.sh basic_O3 hdbscan_basic_benchmark perf_data_d${D} ${N} ${TIME}

fi

cd build && cmake -G Ninja .. \
    -DCMAKE_C_COMPILER=clang-11 \
    -DCMAKE_CXX_COMPILER=clang++-11 \
    -DCMAKE_CXX_FLAGS="-O3 -march=native" \
    -DPACKLEFT_WLOOKUP=1 \
    -DBENCHMARK_AMD=${AMD} &&
    ninja build_bench &&
    ninja build_bench_vec &&
    cd ..


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

if [ $2 = "basic" ] || [ $2 = "advanced" ] || [ $2 != "reference" ] || [ $2 = "all" ]; then
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
        -DCMAKE_C_COMPILER=clang-11 \
        -DCMAKE_CXX_COMPILER=clang++-11 \
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
        -DCMAKE_C_COMPILER=clang-11 \
        -DCMAKE_CXX_COMPILER=clang++-11 \
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
exit 1
if [ $2 = "blocked-v-triangular" ] || [ $2 = "all" ]; then
    printf "Running blocked-v-triangular benchmarks. Creating data...\n"
    python helper_scripts/generate_clusters.py data 6 128
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=clang-11 \
        -DCMAKE_CXX_COMPILER=clang++-11 \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_TRIANG=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_BLOCKED=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        cd ..

    printf "Running blocked-v-triangular benchmarks. Run benchmark basic version...\n"
    # Basic
    ./run_perf_measurements.sh distance_matrix_basic hdbscan_basic_benchmark perf_data_d128 ${N} ${TIME}

    python helper_scripts/generate_clusters.py data 6 128
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=clang-11 \
        -DCMAKE_CXX_COMPILER=clang++-11 \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_TRIANG=1 \
        -DHDBSCAN_PRECOMPUTE_DIST_BLOCKED=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        cd ..

    printf "Running blocked-v-triangular benchmarks. Run benchmark triangular matrix version...\n"
    # Triangular
    ./run_perf_measurements.sh distance_matrix_triangular hdbscan_basic_benchmark perf_data_d128 ${N} ${TIME}
    
    python helper_scripts/generate_clusters.py data 6 128
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=clang-11 \
        -DCMAKE_CXX_COMPILER=clang++-11 \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_TRIANG=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_BLOCKED=1 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        cd ..

    printf "Running blocked-v-triangular benchmarks. Run benchmark blocked matrix version...\n"
    # Triangular
    ./run_perf_measurements.sh distance_matrix_blocked hdbscan_basic_benchmark perf_data_d128 ${N} ${TIME}

    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files distance_matrix_basic.csv \
                distance_matrix_triangular.csv \
                distance_matrix_blocked.csv \
        --save-path plots/triangular_v_blocked.png \
        --metric=cycles \
        --x-scale=linear

    # Remove data used for this experiment
    rm ./data/perf_data_d128_*
fi
