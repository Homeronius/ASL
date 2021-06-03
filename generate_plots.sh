#!/bin/bash


# BASIC Implementation

python helper_scripts/plot_performance_alt.py --system intel  \
        --data-path measurements/martin/20210528_012132 \
        --files basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec.csv \
                basic_distvec_quickvec_primvec.csv \
        --save-path plots/presentation/basic_implementation_cycles.png \
        --metric=cycles \
        --x-scale=linear

python helper_scripts/plot_performance_alt.py --system intel  \
        --data-path measurements/martin/20210528_012132 \
        --files basic.csv \
                basic_distvec.csv \
                basic_distvec_quickvec.csv \
                basic_distvec_quickvec_primvec.csv \
        --save-path plots/presentation/basic_implementation_perf.png \
        --metric=fp/c \
        --x-scale=linear

# Advanced Implementation

python helper_scripts/plot_performance_alt.py --system intel  \
        --data-path measurements/martin/20210528_012132 \
        --files advprim_distvec_quickvec_primvec.csv \
                advprim_distvec_quickvec.csv \
                advprim_distvec.csv \
                advprim.csv \
        --save-path plots/presentation/advanced_perf.png \
        --metric=fp/c \
        --x-scale=linear

python helper_scripts/plot_performance_alt.py --system intel  \
        --data-path measurements/martin/20210528_012132 \
        --files advprim_distvec_quickvec_primvec.csv \
                advprim_distvec_quickvec.csv \
                advprim_distvec.csv \
                advprim.csv \
        --save-path plots/presentation/advanced_cycles.png \
        --metric=cycles \
        --x-scale=linear


# Intel v AMD

#TODO

# Distance Matrix Variations
# TODO Run on martins system
python helper_scripts/plot_performance_alt.py --system intel  \
        --data-path measurements/tom/20210603_141601 \
        --files distance_matrix_basic.csv \
                distance_matrix_blocked80.csv \
                distance_matrix_triangular.csv \
        --save-path plots/presentation/distance_matrix_cycles.png \
        --metric=cycles \
        --x-scale=linear


# Dimension Analysis

#TODO