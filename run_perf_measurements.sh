#!/bin/bash

# Run the benchmark binary on a number of '.csv' datasets and store the
# output to a log as well as .csv file
if [ $# -eq 0 ]; then
    echo "Usage: $(basename $0) <benchm_name>"
    exit 1
fi

# make log directories if not existing
mkdir -p logs
mkdir -p data/timings

SUFFIX=$1
BINARY="./build/bin/hdbscan_benchmark"
OUTPUT_LOG="logs/timings_${SUFFIX}.out"
OUTPUT_CSV="data/timings/${SUFFIX}.csv"
# Substring input the datasets have in common
FILEBASE="data/perf_data"
# Number of datasets to be processed
N=9

if [ -s ${OUTPUT_LOG} ]; then
    printf "File already exists. Appending OUTPUT_LOG ...\n"
    printf "======================\
            \nOUTPUT_LOG from new performance run:\
            \n======================\n" >>${OUTPUT_LOG}
else
    touch ${OUTPUT_LOG}
fi

for i in $(seq 0 ${N}); do
    echo "running benchmark for ${FILEBASE}_${i}.csv ..."
    printf "======================\
            \nMeasurement for ${FILEBASE}_${i}.csv\
            \n======================\n" >>${OUTPUT_LOG}
    ${BINARY} "${FILEBASE}_${i}.csv" "${OUTPUT_CSV}" >>${OUTPUT_LOG}
    printf "\n" >>${OUTPUT_LOG}
done
