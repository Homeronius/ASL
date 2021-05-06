#!/bin/bash

# Run the benchmark binary on a number of '.csv' datasets and store output to a file

BINARY="./build/bin/hdbscan_benchmark"
OUTPUT="timings.out"
# Substring input the datasets have in common
FILEBASE="data/perf_data"
# Number of datasets to be processed
N=9

if [ -s ${OUTPUT} ]
then
    printf "File already exists. Appending output ...\n"
    printf "======================\
            \nOutput from new performance run:\
            \n======================\n" >> ${OUTPUT}
else
    touch ${OUTPUT}
fi

for i in $(seq 0 ${N});
do
    printf "======================\
            \nMeasurement for ${FILEBASE}_${i}.csv\
            \n======================\n" >> ${OUTPUT}
    ${BINARY} "${FILEBASE}_${i}.csv" >> ${OUTPUT}
    printf "\n" >> ${OUTPUT}
done
