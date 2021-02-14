#!/bin/bash
echo "Usage:"
echo "./run_remote_artifact.sh @remote_username @remote_ip @remote_artifact_top_dir"
echo " @remote_username must be in sudoers"
[[ !  -z  $1  ]] || exit
[[ !  -z  $2  ]] || exit
[[ !  -z  $3  ]] || exit
#CONFIGS=(sizeimages.newest.baseline.noopt sizeimages.newest.+retpolines.noopt.old sizeimages.newest.+retpolines.pibe-opt.lmbench-work.999990.old sizeimages.newest.+retpolines.system.noopt sizeimages.newest.+retpolines+retretpolines.noopt sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.990000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999999 sizeimages.newest.alldefenses.noopt sizeimages.newest.alldefenses.pibe-opt.lmbench-work.990000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999999  sizeimages.newest.+lvi.noopt sizeimages.newest.+lvi.pibe-opt.lmbench-work.990000 sizeimages.newest.+lvi.pibe-opt.lmbench-work.999999 sizeimages.newest.+retretpolines.noopt sizeimages.newest.+retretpolines.pibe-opt.lmbench-work.999999 alldefenses.pibe-opt.apache-work.999999) 
CONFIGS=(sizeimages.newest.baseline.noopt sizeimages.newest.baseline.pibe-opt.lmbench-work.999900 sizeimages.newest.+retpolines.noopt.old sizeimages.newest.+retpolines.pibe-opt.lmbench-work.999990.old sizeimages.newest.+retpolines.noopt sizeimages.newest.+retpolines.pibe-opt.lmbench-work.999990 sizeimages.newest.alldefenses.noopt sizeimages.newest.alldefenses.pibe-opt.lmbench-work.990000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999999  sizeimages.newest.+lvi.noopt sizeimages.newest.+lvi.pibe-opt.lmbench-work.990000 sizeimages.newest.+lvi.pibe-opt.lmbench-work.999999 sizeimages.newest.+retretpolines.noopt sizeimages.newest.+retretpolines.pibe-opt.lmbench-work.999999 alldefenses.pibe-opt.apache-work.999999)  
#CONFIGS=(sizeimages.newest.baseline.noopt sizeimages.newest.+retpolines+retretpolines.noopt sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.990000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999999 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999999  sizeimages.newest.+lvi.noopt sizeimages.newest.+lvi.pibe-opt.lmbench-work.990000 sizeimages.newest.+lvi.pibe-opt.lmbench-work.999999 sizeimages.newest.+retretpolines.noopt sizeimages.newest.+retretpolines.pibe-opt.lmbench-work.999999 alldefenses.pibe-opt.apache-work.999999) 
DIR=$6
[[ !  -z  $6  ]] || DIR=default_apache_wrk_dir

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
       ssh -t -o ConnectTimeout=10 $1@$2 "cd $3 && ./startup_apache.sh"
       counter=$?
    done

    TIMES=1
    CONCURENCY=$4
    TIME=$5
    [[ !  -z  $4  ]] || CONCURENCY=100
    [[ !  -z  $5  ]] || TIME=5m
    IP1=10.0.0.3
    IP2=10.0.1.3
    echo "DIR :"${DIR} 
    echo "CONCURENCY :" ${CONCURENCY}
    echo "TIME :" ${TIME}
    
    mkdir -p playground/performance/${DIR}/${CONF/sizeimages.newest./}
    tries=1
    while [ $tries -le $TIMES ]
    do
          echo "Benchmark number :"${tries}" for config:"${CONF/sizeimages.newest./}
          #wrk -t4 -c${CONCURENCY} -d${TIME} --latency http://$IP1:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/wrk1.${tries} &
          #wrk -t4 -c${CONCURENCY} -d${TIME} --latency http://$IP2:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/wrk2.${tries} &
          taskset -c 0,4,1,5 wrk -t4 -c${CONCURENCY} -d${TIME} --latency http://$IP1:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/wrk1.${tries} &
          taskset -c 2,6,3,7 wrk -t4 -c${CONCURENCY} -d${TIME} --latency http://$IP2:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/wrk2.${tries} &
          wait
          ((tries++))
    done
    ssh -t -o ConnectTimeout=10 $1@$2 "cd $3 && ./stop_apache.sh"




  

done


