#!/bin/bash
. global_exports.sh

[[ !  -z  $1  ]] && echo Supplied folder $1 || exit

#TIMES=(0 1 2 3 4 5 6 7 8 9 10) 
TIMES=$RUNS

echo ${BENCHMARK_DIR}
echo "We will do: "$TIMES runs

cd tools/lmbench3

rm results/x86_64-linux-gnu/*
cat bin/x86_64-linux-gnu/CONFIG.$(hostname) | awk '{if ($1 ~ "ENOUGH" ) {print "ENOUGH=5000";next;};if ($1 ~ "MB=") {print "MB=1024";next;};if ($1 ~ "BENCHMARK_HARDWARE=") {print "BENCHMARK_HARDWARE=NO";next;};if ($1 ~ "BENCHMARK_OS=") {print "BENCHMARK_OS=YES";next;};if ($1 ~ /^BENCHMARK/) {sub(/=[[:print:]]*$/,"=");print $0;next;}; {print $0}}' > bin/x86_64-linux-gnu/temp.conf
cat bin/x86_64-linux-gnu/temp.conf > bin/x86_64-linux-gnu/CONFIG.$(hostname)
tries=1
while [ $tries -le $TIMES ]
do
    echo "Benchmark number "${tries}
    make rerun
    ((tries++))
done

[ ! -d ../../playground/${BENCHMARK_DIR}/$1 ] && mkdir -p ../../playground/${BENCHMARK_DIR}/$1


counter=0
tries=1
while [ $tries -le $TIMES ]
do
    echo "Copying benchmark number "${counter}
    (set -x ; cp results/x86_64-linux-gnu/$(hostname).${counter} ../../playground/${BENCHMARK_DIR}/$1/LMBench.${counter} )
    ((counter++))
    ((tries++))
done

cd ../.. 
