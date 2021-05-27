#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $(basename $0) [intel|amd] [baseline_flags|all]"
    exit 1
fi

if [ ! -z $1 ] && [ $1 != "amd" ] && [ $1 != "intel" ]; then
    echo "Invalid argument: $1" >&2
    echo ""
    echo "Usage: $(basename $0) [intel|amd] [baseline_flags|all]"
    exit 1
fi

# extend here with different test names
if [ ! -z $2 ] && [ $2 != "baseline_flags" ] && [ $2 != "all" ]; then
    echo "Invalid argument: $2" >&2
    echo ""
    echo "Usage: $(basename $0) [intel|amd] [baseline_flags|all]"
    exit 1
fi

AMD=0
if [ $1 = "amd" ]; then
    AMD=1
fi

TIME=$(date +%Y%m%d_%H%M%S)
# data creation here first
python helper_scripts/generate_clusters.py data 3 2

# create build dir
mkdir -p build

##########################################################
#### Standard algorithm with different compiler flags ####
##########################################################
#### NOTE: this is basically just a toy comparison for now,
####       no contribution from our side

#if [ $2 = "baseline_flags" ] || [ $2 = "all" ]; then
    #N=3
    ## TODO: variable compiler here
    #for i in $(seq 0 ${N}); do
        #printf "building and running for O${i}...\n"
        #cd build && cmake -G Ninja .. \
            #-DCMAKE_C_COMPILER=clang-11 \
            #-DCMAKE_CXX_COMPILER=clang++-11 \
            #-DOPT_LEVEL=O${i} \
            #-DBENCHMARK_AMD=${AMD} &&
            #ninja build_bench &&
            #cd ..
        #./run_perf_measurements.sh O${i}_primadvanced_test hdbscan_benchmark perf_data_d2 12 ${TIME}
    #done
    #printf "building and running for O3, vectorized version...\n"
    #cd build && cmake -G Ninja .. \
        #-DCMAKE_C_COMPILER=clang-11 \
        #-DCMAKE_CXX_COMPILER=clang++-11 \
        #-DOPT_LEVEL=O3 \
        #-DBENCHMARK_AMD=${AMD} &&
        #ninja build_bench_vec &&
        #cd ..
    #./run_perf_measurements.sh O3_primadvanced_test_vec hdbscan_benchmark_vec perf_data_d2 12

    #printf "creating plot...\n"

    #python helper_scripts/plot_performance_alt.py --system $1 \
        #--data-path data/timings/${TIME} \
        #--files O0_primadvanced_test.csv O1_primadvanced_test.csv O2_primadvanced_test.csv O3_primadvanced_test.csv O3_primadvanced_test_vec.csv \
        #--save-path $1_flags_comparison.png

#fi

## TODO:

##########################################################
####   Compare hand vectorized with non-vectorized     ########
##########################################################

##########################################################
########### Precompute dist vs. non precompute, triangular? ###########
##########################################################

##########################################################
########### Locality, memory alignment ###########
##########################################################

##########################################################
#### comparison for different dimension sizes d, mpts, minimum cluster size? ###
##########################################################

##########################################################
######## different compilers?                     ########
##########################################################

##########################################################
######## final: baseline vs. medium vs. best code ########
##########################################################


##########################################################
######## Total Comparison of all optimizations    ########
##########################################################

# Total Comparison flops/cycles d=20
N=12

# Basic everything
python helper_scripts/generate_clusters.py data 6 20
cd build && cmake -G Ninja .. \
    -DCMAKE_C_COMPILER=clang-11 \
    -DCMAKE_CXX_COMPILER=clang++-11 \
    -DOPT_LEVEL=O3 \
    -DBENCHMARK_AMD=${AMD} &&
    ninja &&
    ninja build_bench &&
    ninja build_bench_vec &&
    cd ..

##########################################################
######## Algorithm with basic prim algorithm    ##########
##########################################################

# No optimizations
./run_perf_measurements.sh basic hdbscan_basic_benchmark perf_data_d20 ${N} ${TIME}
# Adding optimized distance computations (computation of adjacency matrix)
./run_perf_measurements.sh basic_distvec hdbscan_basic_benchmark_distvec perf_data_d20 ${N} ${TIME}
# Adding optimized quickselect algorithm
./run_perf_measurements.sh basic_distvec_quickvec hdbscan_basic_benchmark_distvec_quickvec perf_data_d20 ${N} ${TIME}
# Adding optimized prim algorithm
./run_perf_measurements.sh basic_distvec_quickvec_primvec hdbscan_basic_benchmark_distvec_quickvec_primvec perf_data_d20 ${N} ${TIME}

# Plot result
python helper_scripts/plot_performance_alt.py  \
    --data-path data/timings/${TIME} \
    --files basic.csv \
            basic_distvec.csv \
            basic_distvec_quickvec.csv \
            basic_distvec_quickvec.csv \
            basic_distvec_quickvec_primvec.csv  \
    --save-path plots/performance_basic.png

# Algorithm with advanced prim algorithm
##########################################################
######## Algorithm with advanced prim algorithm ##########
##########################################################

# No optimizations
./run_perf_measurements.sh advprim hdbscan_benchmark perf_data_d20 ${N} ${TIME}
# Adding optimized distance computation (computation of core_distances)
./run_perf_measurements.sh advprim_distvec hdbscan_benchmark_distvec perf_data_d20 ${N} ${TIME}
# Adding optimized quickselect algorithm
./run_perf_measurements.sh advprim_distvec_quickvec hdbscan_benchmark_distvec_quickvec perf_data_d20 ${N} ${TIME}
# Adding optimized prim algorithm
./run_perf_measurements.sh advprim_distvec_quickvec_primvec hdbscan_benchmark_distvec_quickvec_primvec perf_data_d20 ${N} ${TIME}

# Plot result
python helper_scripts/plot_performance_alt.py  \
    --data-path data/timings/${TIME} \
    --files advprim.csv \
            advprim_distvec.csv \
            advprim_distvec_quickvec.csv \
            advprim_distvec_quickvec.csv \
            advprim_distvec_quickvec_primvec.csv  \
    --save-path plots/performance_advprim.png


##########################################################
######## Plots showing cycles of each binary \  ##########
######## for basic & advanced prim              ##########
##########################################################

# Plot basic 
python helper_scripts/plot_performance_alt.py  \
    --data-path data/timings/${TIME} \
    --files basic.csv \
            basic_distvec.csv \
            basic_distvec_quickvec.csv \
            basic_distvec_quickvec.csv \
            basic_distvec_quickvec_primvec.csv  \
    --save-path plots/cycles_basic.png \
    --metric=cycles

# Plot advprim
python helper_scripts/plot_performance_alt.py  \
    --data-path data/timings/${TIME} \
    --files advprim.csv \
            advprim_distvec.csv \
            advprim_distvec_quickvec.csv \
            advprim_distvec_quickvec.csv \
            advprim_distvec_quickvec_primvec.csv  \
    --save-path plots/cycles_advprim.png \
    --metric=cycles

# Clean up datasets used for this part
rm ./data/perf_data_d20_*


