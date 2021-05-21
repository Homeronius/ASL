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

# data creation here first
python helper_scripts/generate_clusters_alt.py data 3 2

# create build dir
mkdir -p build

##########################################################
#### Standard algorithm with different compiler flags ####
##########################################################
#### NOTE: this is basically just a toy comparison for now,
####       no contribution from our side

if [ $2 = "baseline_flags" ] || [ $2 = "all" ]; then
    N=3
    # TODO: variable compiler here
    for i in $(seq 0 ${N}); do
        printf "building and running for O${i}...\n"
        cd build && cmake -G Ninja .. \
            -DCMAKE_C_COMPILER=clang-11 \
            -DCMAKE_CXX_COMPILER=clang++-11 \
            -DOPT_LEVEL=O${i} \
            -DBENCHMARK_AMD=${AMD} &&
            ninja build_bench &&
            cd ..
        ./run_perf_measurements.sh O${i}_primadvanced_test hdbscan_benchmark perf_data_d2 12
    done

    printf "creating plot...\n"

    python helper_scripts/plot_performance_alt.py --system $1 \
        --files O0_primadvanced_test.csv O1_primadvanced_test.csv O2_primadvanced_test.csv O3_primadvanced_test.csv \
        --save-path $1_flags_comparison.png

fi

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
