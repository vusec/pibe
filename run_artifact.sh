#!/bin/bash
echo "Usage:"
echo "./run_remote_artifact.sh @remote_username @remote_ip @remote_artifact_top_dir"
echo " @remote_username must be in sudoers"
[[ !  -z  $1  ]] || exit
[[ !  -z  $2  ]] || exit
[[ !  -z  $3  ]] || exit
CONFIGS=(baseline.noopt baseline.pibe-opt.lmbench-work.999900 alldefenses.noopt alldefenses.pibe-opt.lmbench-work.999999 alldefenses.pibe-opt.apache-work.999999 +retpolines.noopt +retpolines.pibe-opt.lmbench-work.999990   +retretpolines.noopt +retretpolines.pibe-opt.lmbench-work.999999 +lvi.noopt +lvi.pibe-opt.lmbench-work.999999 ) 


for CONF in ${CONFIGS[@]}; do
    echo "Loading configuration "${CONF}
    counter=1
    while [ $counter != 0 ]; do
       echo "Attempting connection..."
       sleep 1
       ssh -t -o ConnectTimeout=10 $1@$2 "exit"
       counter=$?
    done
    ssh -t $1@$2 "cd $3 && sudo ./load_kernel.sh ${CONF}"

    counter=1
    while [ $counter != 0 ]; do
       echo "Attempting connection..."
       sleep 2
       #ssh -t -o ConnectTimeout=10 $1@$2 "exit"
       ssh -t -o ConnectTimeout=10 $1@$2 "cd $3 && ./benchmark_performance.sh ${CONF}"
       counter=$?
    done
  

done


