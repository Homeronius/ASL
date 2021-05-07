#!/bin/bash

# Run the benchmark binary on a number of '.csv' datasets and store the
# output to a log as well as .csv file

BINARY="./build/bin/hdbscan_benchmark"
OUTPUT_LOG="timings.out"
OUTPUT_CSV="data/timings.csv"
# Substring input the datasets have in common
FILEBASE="data/perf_data"
# Number of datasets to be processed
N=9

if [ -s ${OUTPUT_LOG} ]
then
    printf "File already exists. Appending OUTPUT_LOG ...\n"
    printf "======================\
            \nOUTPUT_LOG from new performance run:\
            \n======================\n" >> ${OUTPUT_LOG}
else
    touch ${OUTPUT_LOG}
fi

for i in $(seq 0 ${N});
do
    printf "======================\
            \nMeasurement for ${FILEBASE}_${i}.csv\
            \n======================\n" >> ${OUTPUT_LOG}
    ${BINARY} "${FILEBASE}_${i}.csv" "${OUTPUT_CSV}" >> ${OUTPUT_LOG}
    printf "\n" >> ${OUTPUT_LOG}
done
