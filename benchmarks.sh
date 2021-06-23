#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $(basename $0) [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|gcc-v-clang|mpts|data_variance|cache_analysis|manhattan|all]"
    exit 1
fi

if [ ! -z $1 ] && [ $1 != "amd" ] && [ $1 != "intel" ]; then
    echo "Invalid argument: $1" >&2
    echo ""
    echo "Usage: $(basename $0) [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|gcc-v-clang|mpts|data_variance|cache_analysis|manhattan|all]"
    exit 1
fi

# extend here with different test names
if [ ! -z $2 ] && [ $2 != "basic" ] && [ $2 != "advanced" ] && \
   [ $2 != "amd-v-intel" ] && [ $2 != "blocked-v-triangular" ] && \
   [ $2 != "reference" ] && [ $2 != "gcc-v-clang" ] && [ $2 != "mpts" ] && \
   [ $2 != "dimensions" ] && [ $2 != "data_variance" ] && [ $2 != "cache_analysis" ] && \
   [ $2 != "manhattan" ] && [ $2 != "all" ]; then
    echo "Invalid argument: $2" >&2
    echo ""
    echo "Usage: $(basename $0) [intel|amd] [basic|advanced|amd-v-intel|blocked-v-triangular|reference|gcc-v-clang|mpts|data_variance|cache_analysis|manhattan|all]"
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

rm -rf build

# create build dir
mkdir -p build
mkdir -p plots/${TIME}

##########################################################
######## Total Comparison of all optimizations    ########
##########################################################

# Total Comparison flops/cycles

# TEST
# N=3
# NFIX=3
# D=32

# BENCHMARKING
N=12
NFIX=7
D=64

# Basic everything
if [ $2 = "basic" ] || [ $2 = "advanced" ] ||  [ $2 = "reference" ] || [ $2 = "mpts" ] || [ $2 = "amd-v-intel" ] || [ $2 = "all" ]; then
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
        make CXX=${CXX_COMPILER} CPPFLAGS="-march=native"
    fi
    cd ../..
    # a bit hacky, but our script assumes binary is in build
    ./run_perf_measurements.sh reference_hdbscan ./references/hdbscan-cpp/main perf_data_d${D} ${N} ${TIME}
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
        -DHDBSCAN_TEST=0 \
        -DHDBSCAN_QUICKSELECT=0 &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

  ./run_perf_measurements.sh quickselect_baseline ./build/bin/hdbscan_benchmark perf_data_d${D} ${NFIX} ${TIME} 5
  ./run_perf_measurements.sh bubbleselect ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${D} ${NFIX} ${TIME} 5
  for i in $(seq 10 10 100); do
    ./run_perf_measurements.sh quickselect_baseline ./build/bin/hdbscan_benchmark perf_data_d${D} ${NFIX} ${TIME} ${i}
    ./run_perf_measurements.sh bubbleselect ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${D} ${NFIX} ${TIME} ${i}
  done

    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DBENCHMARK_AMD=${AMD} \
        -DHDBSCAN_TEST=0 \
        -DHDBSCAN_QUICKSELECT=1 &&
        ninja build_bench_vec &&
        cd ..

  ./run_perf_measurements.sh quickselect_vectorized ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${D} ${NFIX} ${TIME} 5
  for i in $(seq 10 10 100); do
    ./run_perf_measurements.sh quickselect_vectorized ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${D} ${NFIX} ${TIME} ${i}
  done

  python helper_scripts/plot_performance_alt.py --system $1  \
      --data-path data/timings/${TIME} \
      --files quickselect_baseline.csv \
              bubbleselect.csv \
              quickselect_vectorized.csv \
      --save-path plots/${TIME}/cycles_mpts.pdf \
      --metric=cycles \
      --x-scale=linear
fi


if [ $2 != "mpts" ]; then
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
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
    ./run_perf_measurements.sh basic ./build/bin/hdbscan_basic_benchmark perf_data_d${D} ${N} ${TIME}
    # Adding optimized distance computations (computation of adjacency matrix)
    ./run_perf_measurements.sh basic_distvec ./build/bin/hdbscan_basic_benchmark_distvec perf_data_d${D} ${N} ${TIME}
    # Adding optimized quickselect algorithm
    ./run_perf_measurements.sh basic_distvec_quickvec ./build/bin/hdbscan_basic_benchmark_distvec_quickvec perf_data_d${D} ${N} ${TIME}
    # Adding optimized prim algorithm
    ./run_perf_measurements.sh basic_distvec_quickvec_primvec ./build/bin/hdbscan_basic_benchmark_distvec_quickvec_primvec perf_data_d${D} ${N} ${TIME}

    # Plot result
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/performance_basic.pdf
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/performance_basic_linear.pdf \
        --x-scale=linear
fi

##########################################################
######## Algorithm with advanced prim algorithm ##########
##########################################################
if [ $2 = "advanced" ] || [ $2 = "all" ]; then
    # No optimizations
    ./run_perf_measurements.sh advprim ./build/bin/hdbscan_benchmark perf_data_d${D} ${N} ${TIME}
    # Adding optimized distance computation (computation of core_distances)
    ./run_perf_measurements.sh advprim_distvec ./build/bin/hdbscan_benchmark_distvec perf_data_d${D} ${N} ${TIME}
    # Adding optimized quickselect algorithm
    ./run_perf_measurements.sh advprim_distvec_quickvec ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${D} ${N} ${TIME}
    # Adding optimized prim algorithm
    ./run_perf_measurements.sh advprim_distvec_quickvec_primvec ./build/bin/hdbscan_benchmark_distvec_quickvec_primvec perf_data_d${D} ${N} ${TIME}

    # Plot result
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim.csv \
                advprim_distvec.csv \
                advprim_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/performance_advprim.pdf
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim.csv \
                advprim_distvec.csv \
                advprim_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/performance_advprim_linear.pdf \
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
                basic_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/cycles_basic.pdf \
        --metric=cycles \
        --x-scale=linear
fi

if [ $2 = "advanced" ] || [ $2 = "all" ]; then
    # Plot advprim
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim.csv \
                advprim_distvec.csv \
                advprim_distvec_quickvec_primvec.csv  \
        --save-path plots/${TIME}/cycles_advprim.pdf \
        --metric=cycles \
        --x-scale=linear
fi

if [ $2 = "all" ]; then
    # Plot basic vs advprim
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic.csv \
                basic_distvec_quickvec_primvec.csv \
                advprim.csv \
                advprim_distvec.csv  \
        --save-path plots/${TIME}/performance_basic_vs_advanced.pdf \
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic.csv \
                basic_distvec_quickvec_primvec.csv \
                advprim.csv \
                advprim_distvec.csv  \
        --save-path plots/${TIME}/performance_basic_vs_advanced_linear.pdf \
        --x-scale=linear 

    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files basic.csv \
                basic_distvec_quickvec_primvec.csv \
                advprim.csv \
                advprim_distvec.csv  \
        --save-path plots/${TIME}/cycles_basic_vs_advanced.pdf \
        --metric=cycles \
        --x-scale=linear
    
    # Plot reference vs ours
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files reference_hdbscan.csv \
                basic.csv basic_distvec_quickvec_primvec.csv \
                basic_distvec.csv \
                advprim.csv \
                advprim_distvec.csv \
        --save-path plots/${TIME}/cycles_ref_vs_ours.pdf --x-scale=linear --metric=cycles
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files reference_hdbscan.csv \
                basic.csv basic_distvec_quickvec_primvec.csv \
                basic_distvec.csv \
                advprim.csv \
                advprim_distvec.csv \
        --save-path plots/${TIME}/performance_ref_vs_ours.pdf --x-scale=linear
fi

##########################################################
######## Comparison between Intel and AMD         ########
######## with focus on the vectorization          ########
######## of the partition function                ########
##########################################################

if [ $2 = "amd-v-intel" ] || [ $2 = "all" ]; then
    printf "Running amd-v-intel benchmarks. Creating data...\n"
    # First with the version where `pext` and `pdep` are used
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=0 \
        -DHDBSCAN_QUICKSELECT=1 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

    printf "Running amd-v-intel benchmarks. Run benchmark with pext/pdep partitioning...\n"
    # Best performing 
    ./run_perf_measurements.sh $1_pext_partition ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d4 ${N} ${TIME}

    # Then the optimized version for AMD, using a lookup table
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=1 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

    printf "Running amd-v-intel benchmarks. Run benchmark with LUT partitioning...\n"
    # Best performing 
    ./run_perf_measurements.sh $1_lut_partition ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d4 ${N} ${TIME}

    printf "Running amd-v-intel benchmarks. Run benchmark without vectorization...\n"
    ./run_perf_measurements.sh $1_no_vec ./build/bin/hdbscan_benchmark perf_data_d4 ${N} ${TIME}

    # Plot like this here (should be done with second machine, i.e.
    # benchmarks have been copied) 
    : ' python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files amd_pext_partition.csv \
                amd_lut_partition.csv \
                intel_pext_partition.csv \
                intel_lut_partition.csv \
        --save-path plots/perf_amd_vs_intel.pdf \
        --metric=cycles
        --x-scale=linear
    '
fi

if [ $2 = "basic" ] || [ $2 = "advanced" ] || [ $2 == "reference" ] || [ $2 = "mpts" ] || [ $2 = "amd-v-intel" ] || [ $2 = "all" ]; then
    # Clean up datasets used for this part
    rm ./data/perf_data_d${D}_*
fi


#######################################################################################
######## Comparison between basic, triangular and blocked distance computation ########
#######################################################################################
# Total Comparison flops/cycles d=128

if [ $2 = "blocked-v-triangular" ] || [ $2 = "all" ]; then
    printf "Running blocked-v-triangular benchmarks. Creating data...\n"
    python helper_scripts/generate_clusters.py data 6 20
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -DHDBSCAN_BLOCK_SIZE=20" \
        -DPACKLEFT_WLOOKUP=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_TRIANG=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_BLOCKED=0 \
        -DHDBSCAN_VERBOSE=0 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        cd ..

    printf "Running blocked-v-triangular benchmarks. Run benchmark basic version...\n"
    # Basic
    ./run_perf_measurements.sh distance_matrix_basic ./build/bin/hdbscan_basic_benchmark perf_data_d20 ${N} ${TIME}

    ./run_perf_measurements.sh distance_matrix_triangular ./build/bin/hdbscan_basic_benchmark_triang perf_data_d20 ${N} ${TIME}

    ./run_perf_measurements.sh distance_matrix_blocked_20 ./build/bin/hdbscan_basic_benchmark_blocked perf_data_d20 ${N} ${TIME}

    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -DHDBSCAN_BLOCK_SIZE=40" \
        -DPACKLEFT_WLOOKUP=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_TRIANG=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_BLOCKED=0 \
        -DHDBSCAN_VERBOSE=0 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        cd ..

    ./run_perf_measurements.sh distance_matrix_blocked_40 ./build/bin/hdbscan_basic_benchmark_blocked perf_data_d20 ${N} ${TIME}
    
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -DHDBSCAN_BLOCK_SIZE=80" \
        -DPACKLEFT_WLOOKUP=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_TRIANG=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_BLOCKED=0 \
        -DHDBSCAN_VERBOSE=0 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        cd ..

    ./run_perf_measurements.sh distance_matrix_blocked_80 ./build/bin/hdbscan_basic_benchmark_blocked perf_data_d20 ${N} ${TIME}

    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -DHDBSCAN_BLOCK_SIZE=100" \
        -DPACKLEFT_WLOOKUP=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_TRIANG=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_BLOCKED=0 \
        -DHDBSCAN_VERBOSE=0 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        cd ..

    ./run_perf_measurements.sh distance_matrix_blocked_100 ./build/bin/hdbscan_basic_benchmark_blocked perf_data_d20 ${N} ${TIME}

    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files distance_matrix_basic.csv \
                distance_matrix_triangular.csv \
                distance_matrix_blocked_20.csv \
                distance_matrix_blocked_40.csv \
                distance_matrix_blocked_80.csv \
                distance_matrix_blocked_100.csv \
        --save-path=plots/triangular_v_blocked.pdf \
        --metric=cycles \
        --x-scale=linear

    # Remove data used for this experiment
    rm -f ./data/perf_data_d20_*
fi

########################################################################
######## Comparison of gcc vs. clang on most optimized versions ########
########################################################################

if [ $2 = "gcc-v-clang" ] || [ $2 = "all" ]; then
    # Generate data
    python helper_scripts/generate_clusters.py data 6 ${D}

    # clang-11
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -ffast-math" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

    ./run_perf_measurements.sh clang_basic_autovec ./build/bin/hdbscan_basic_benchmark perf_data_d${D} ${N} ${TIME}
    ./run_perf_measurements.sh clang_advprim_autovec ./build/bin/hdbscan_benchmark perf_data_d${D} ${N} ${TIME}

    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

    ./run_perf_measurements.sh clang_basic_best ./build/bin/hdbscan_basic_benchmark_distvec_quickvec_primvec perf_data_d${D} ${N} ${TIME}
    ./run_perf_measurements.sh clang_advprim_best ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${D} ${N} ${TIME}

    # gcc
    # Needed, otherwise problems with compiling with new compiler
    rm -rf build
    mkdir -p build

    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=gcc-11 \
        -DCMAKE_CXX_COMPILER=g++-11 \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -ffast-math" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

    ./run_perf_measurements.sh gcc_basic_autovec ./build/bin/hdbscan_basic_benchmark  perf_data_d${D} ${N} ${TIME}
    ./run_perf_measurements.sh gcc_advprim_autovec ./build/bin/hdbscan_benchmark  perf_data_d${D} ${N} ${TIME}

    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=gcc-11 \
        -DCMAKE_CXX_COMPILER=g++-11 \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench &&
        ninja build_bench_vec &&
        cd ..

    ./run_perf_measurements.sh gcc_basic_best ./build/bin/hdbscan_basic_benchmark_distvec_quickvec_primvec  perf_data_d${D} ${N} ${TIME}
    ./run_perf_measurements.sh gcc_advprim_best ./build/bin/hdbscan_benchmark_distvec_quickvec  perf_data_d${D} ${N} ${TIME}

    # Plotting
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files gcc_basic_best.csv \
                gcc_advprim_best.csv \
                gcc_basic_autovec.csv \
                gcc_advprim_autovec.csv \
                clang_basic_best.csv \
                clang_advprim_best.csv  \
                clang_basic_autovec.csv \
                clang_advprim_autovec.csv  \
        --save-path plots/${TIME}/gcc_v_clang.pdf
    
    rm -rf build
    mkdir -p build
fi


#####################################################
######## Comparison for different dimensions ########
#####################################################

if [ $2 = "dimensions" ] || [ $2 = "all" ]; then
    D=8
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DSPECIALIZED_DISTANCE=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench_vec &&
        cd ..
    for i in $(seq 1 ${D}); do
        d=$((2 ** ${i}))
        python helper_scripts/generate_clusters.py data 6 ${d}
        printf "running for d = ${d}...\n"
        ./run_perf_measurements.sh advprim_distvec_quickvec_dims ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${d} ${NFIX} ${TIME} 4
    done
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DSPECIALIZED_DISTANCE=1 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench_vec &&
        cd ..
    for i in $(seq 1 ${D}); do
        d=$((2 ** ${i}))
        printf "running for d = ${d}...\n"
        ./run_perf_measurements.sh advprim_distvec_quickvec_dims_spec ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${d} ${NFIX} ${TIME} 4
        ./run_perf_measurements.sh advprim_distvec_quickvec_dims${d} ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${d} ${N} ${TIME}
        rm ./data/perf_data_d${d}_*
    done
    
    python helper_scripts/plot_performance_dims.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_distvec_quickvec_dims2.csv \
                advprim_distvec_quickvec_dims4.csv \
                advprim_distvec_quickvec_dims8.csv \
                advprim_distvec_quickvec_dims16.csv \
                advprim_distvec_quickvec_dims32.csv \
                advprim_distvec_quickvec_dims64.csv \
                advprim_distvec_quickvec_dims128.csv \
                advprim_distvec_quickvec_dims256.csv \
        --save-path plots/${TIME}/perf_dims_heatmap.pdf

    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_distvec_quickvec_dims.csv \
                advprim_distvec_quickvec_dims_spec.csv \
        --save-path plots/${TIME}/cycles_dims.pdf \
        --metric=cycles
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_distvec_quickvec_dims.csv \
                advprim_distvec_quickvec_dims_spec.csv \
        --save-path plots/${TIME}/perf_dims.pdf

fi

if [ $2 = "manhattan" ] || [ $2 = "all" ]; then
    D=8
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DSPECIALIZED_DISTANCE=0 \
        -DHDBSCAN_MANHATTAN=1 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench_vec &&
        cd ..
    for i in $(seq 1 ${D}); do
        d=$((2 ** ${i}))
        python helper_scripts/generate_clusters.py data 6 ${d}
        printf "running for d = ${d}...\n"
        ./run_perf_measurements.sh advprim_distvec_quickvec_dims_manhattan ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${d} ${NFIX} ${TIME} 4
    done
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DHDBSCAN_MANHATTAN=1 \
        -DSPECIALIZED_DISTANCE=1 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench_vec &&
        cd ..
    for i in $(seq 1 ${D}); do
        d=$((2 ** ${i}))
        printf "running for d = ${d}...\n"
        ./run_perf_measurements.sh advprim_distvec_quickvec_dims_manhattan_spec ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${d} ${NFIX} ${TIME} 4
        rm ./data/perf_data_d${d}_*
    done

    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_distvec_quickvec_dims_manhattan.csv \
                advprim_distvec_quickvec_dims_manhattan_spec.csv \
        --save-path plots/${TIME}/cycles_dims_manhattan.pdf \
        --metric=cycles
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_distvec_quickvec_dims_manhattan.csv \
                advprim_distvec_quickvec_dims_manhattan_spec.csv \
        --save-path plots/${TIME}/perf_dims_manhattan.pdf

fi

if [ $2 = "all" ]; then
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_distvec_quickvec_dims.csv \
                advprim_distvec_quickvec_dims_spec.csv \
                advprim_distvec_quickvec_dims_manhattan.csv \
                advprim_distvec_quickvec_dims_manhattan_spec.csv \
        --save-path plots/${TIME}/cycles_dims_euclid_manhattan.pdf \
        --metric=cycles
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_distvec_quickvec_dims.csv \
                advprim_distvec_quickvec_dims_spec.csv \
                advprim_distvec_quickvec_dims_manhattan.csv \
                advprim_distvec_quickvec_dims_manhattan_spec.csv \
        --save-path plots/${TIME}/perf_dims_euclid_manhattan.pdf
fi

#####################################################
######## Comparison for different data ########
#####################################################

if [ $2 = "data_variance" ] || [ $2 = "all" ]; then
    K=4
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_TEST=0 \
        -DSPECIALIZED_DISTANCE=0 \
        -DHDBSCAN_MANHATTAN=0 \
        -DBENCHMARK_AMD=${AMD} &&
        ninja build_bench_vec &&
        cd ..
    for i in $(seq 1 ${K}); do
        python helper_scripts/generate_clusters.py data ${i} ${D}
        printf "running for i = ${i} blobs...\n"
        ./run_perf_measurements.sh advprim_vec_blobs${i} ./build/bin/hdbscan_benchmark_distvec_quickvec perf_data_d${D} ${N} ${TIME}
        ./run_perf_measurements.sh basic_best_blobs${i} ./build/bin/hdbscan_basic_benchmark_distvec_quickvec_primvec perf_data_d${D} ${N} ${TIME}
    done
    rm ./data/perf_data_d${D}*
    
    python helper_scripts/plot_performance_alt.py --system $1  \
        --data-path data/timings/${TIME} \
        --files advprim_vec_blobs1.csv \
                advprim_vec_blobs2.csv \
                advprim_vec_blobs3.csv \
                advprim_vec_blobs4.csv \
                basic_best_blobs1.csv \
                basic_best_blobs2.csv \
                basic_best_blobs3.csv \
                basic_best_blobs4.csv \
        --save-path plots/${TIME}/performance_data_variance.pdf \
        --x-scale=linear
fi


if [ $2 = "cache_analysis" ] || [ $2 = "all" ]; then

    rm -rf build
    mkdir -p build
    cd build && cmake -G Ninja .. \
        -DCMAKE_C_COMPILER=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
        -DCMAKE_CXX_FLAGS="-O3 -march=native -DHDBSCAN_BLOCK_SIZE=80" \
        -DPACKLEFT_WLOOKUP=1 \
        -DHDBSCAN_QUICKSELECT=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_TRIANG=0 \
        -DHDBSCAN_PRECOMPUTE_DIST_BLOCKED=0 \
        -DHDBSCAN_VERBOSE=0 \
        -DHDBSCAN_TEST=0 &&
        ninja &&
        cd ..
    #cd build && ninja && cd ..

    # build reference binary separately

    cd references/hdbscan-cpp && make clean
    if [ $1 = "amd" ]; then
        make CXX=${CXX_COMPILER} CPPFLAGS="-DBENCHMARK_AMD -march=native"
    else
        make CXX=${CXX_COMPILER} CPPFLAGS="-march=native"
    fi
    cd ../..

    declare -a basic_arr=("hdbscan_basic"
                           "hdbscan_basic_distvec"
                           "hdbscan_basic_distvec_quickvec_primvec"
    )

    declare -a advprim_arr=("hdbscan"
                            "hdbscan_distvec"
                            "hdbscan_distvec_quickvec_primvec"
    )

    declare -a blocked_v_distvec_arr=("hdbscan_basic_blocked"
                                      "hdbscan_basic_triang"
    )

    save_dir="./data/timings/${TIME}/"
    mkdir -p ${save_dir}
    python helper_scripts/generate_clusters.py data 3 4
    python helper_scripts/generate_clusters.py data 6 20

    echo ===============
    echo Basic binaries:
    echo ===============
    for D in 64; do
        outfile="${save_dir}basic_cg_N${N}_d${D}.out"
        touch ${outfile}
        for i in "${basic_arr[@]}"; do
           printf "Running Cache analysis for $i\n\n" >> $outfile
           valgrind --tool=cachegrind --cachegrind-out-file=cg.out ./build/bin/$i data/perf_data_d${D}_${N}.csv 2>> $outfile
           printf "\n////////////////////////////////////////////////////////////////////////////////////\n\n" >> $outfile
        done
    done

    echo =======================
    echo Advanced Prim binaries:
    echo =======================
    for D in 64; do
        outfile="${save_dir}advprim_cg_N${N}_d${D}.out"
        touch $outfile
        for i in "${advprim_arr[@]}"; do
           printf "Running Cache analysis for $i\n\n" >> $outfile
           valgrind --tool=cachegrind --cachegrind-out-file="cg.out" ./build/bin/${i} data/perf_data_d${D}_${N}.csv 2>> $outfile
           printf "\n////////////////////////////////////////////////////////////////////////////////////\n\n" >> $outfile
        done
    done


    echo ===============
    echo Blocked-v-Triang binaries:
    echo ===============
    for D in 64; do
        outfile="${save_dir}blocked_v_triang_cg_N${N}_d${D}.out"
        touch $outfile
        for i in "${blocked_v_distvec_arr[@]}"; do
           printf "Running Cache analysis for $i\n\n" >> $outfile
           valgrind --tool=cachegrind --cachegrind-out-file="cg.out" ./build/bin/$i data/perf_data_d${D}_${N}.csv 2>> $outfile
           printf "\n////////////////////////////////////////////////////////////////////////////////////\n\n" >> $outfile
        done
    done


    echo ===============
    echo Reference binary:
    echo ===============
    # a bit hacky, but our script assumes binary is in build
    for D in 64; do
        outfile="${save_dir}reference_cg_N${N}_d${D}.out"
        touch $outfile
        printf "Running Cache analysis for reference implementation\n\n" >> $outfile
        valgrind --tool=cachegrind --cachegrind-out-file="cg.out" ./references/hdbscan-cpp/main data/perf_data_d${D}_${N}.csv 2>> $outfile
        printf "\n////////////////////////////////////////////////////////////////////////////////////\n\n" >> $outfile
    done

fi
