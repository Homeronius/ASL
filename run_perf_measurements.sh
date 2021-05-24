#!/bin/bash

# Run the benchmark binary on a number of '.csv' datasets and store the
# output to a log as well as .csv file
if [[ $# -ne 4 && $# -ne 5 ]]; then
    echo "Usage: $(basename $0) <benchm_name> <binary_name> <filebase> <n_datasets>"
    exit 1
fi


# take time if not given as arg
TIME=${5:-$(date +%Y%m%d_%H%M%S)}

mkdir -p data/timings/${TIME}

SUFFIX=$1
PREFIX="data/timings/${TIME}"
BINARY="./build/bin/$2"
OUTPUT_LOG="${PREFIX}/timings_${SUFFIX}.out"
OUTPUT_CSV="${PREFIX}/${SUFFIX}.csv"

# Substring input the datasets have in common
FILEBASE="data/$3"
# Number of datasets to be processed
N=$4

if [ -s ${OUTPUT_LOG} ]; then
    printf "File already exists. Appending OUTPUT_LOG ...\n"
    printf "======================\
            \nOUTPUT_LOG from new performance run:\
            \n======================\n" >>${OUTPUT_LOG}
else
    touch ${OUTPUT_LOG}
fi
printf "Running test at time %s\n" "$(date)" >>${OUTPUT_LOG}
for i in $(seq 0 ${N}); do
    echo "running benchmark \"${SUFFIX}\" for ${FILEBASE}_${i}.csv ..."
    printf "======================\
            \nMeasurement for ${FILEBASE}_${i}.csv\
            \n======================\n" >>${OUTPUT_LOG}
    ${BINARY} "${FILEBASE}_${i}.csv" "${OUTPUT_CSV}" >>${OUTPUT_LOG}
    printf "\n" >>${OUTPUT_LOG}
done
