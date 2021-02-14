#!/bin/bash
echo "Usage:"
echo "./run_remote_artifact.sh @remote_username @remote_ip @remote_artifact_top_dir"
echo " @remote_username must be in sudoers"
[[ !  -z  $1  ]] || exit
[[ !  -z  $2  ]] || exit
[[ !  -z  $3  ]] || exit
CONFIGS=(sizeimages.newest.baseline.noopt sizeimages.newest.+retpolines.noopt.old sizeimages.newest.+retpolines.pibe-opt.lmbench-work.999990.old sizeimages.newest.+retpolines.system.noopt sizeimages.newest.+retpolines+retretpolines.noopt sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.990000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999999 sizeimages.newest.alldefenses.noopt sizeimages.newest.alldefenses.pibe-opt.lmbench-work.990000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999999  sizeimages.newest.+lvi.noopt sizeimages.newest.+lvi.pibe-opt.lmbench-work.990000 sizeimages.newest.+lvi.pibe-opt.lmbench-work.999999 sizeimages.newest.+retretpolines.noopt sizeimages.newest.+retretpolines.pibe-opt.lmbench-work.999999 alldefenses.pibe-opt.apache-work.999999)
CONFIGS=(sizeimages.newest.baseline.noopt sizeimages.newest.+retpolines.noopt.old sizeimages.newest.+retpolines.pibe-opt.lmbench-work.999990.old sizeimages.newest.+retpolines.noopt sizeimages.newest.+retpolines.pibe-opt.lmbench-work.999990 sizeimages.newest.+retpolines.system.noopt sizeimages.newest.alldefenses.noopt sizeimages.newest.alldefenses.pibe-opt.lmbench-work.990000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999999  sizeimages.newest.+lvi.noopt sizeimages.newest.+lvi.pibe-opt.lmbench-work.990000 sizeimages.newest.+lvi.pibe-opt.lmbench-work.999999 sizeimages.newest.+retretpolines.noopt sizeimages.newest.+retretpolines.pibe-opt.lmbench-work.999999 alldefenses.pibe-opt.apache-work.999999) 
#CONFIGS=(sizeimages.newest.baseline.noopt sizeimages.newest.+retpolines.noopt sizeimages.newest.+retpolines.pibe-opt.lmbench-work.999990)
#CONFIGS=(sizeimages.newest.baseline.noopt sizeimages.newest.+retpolines.noopt.old sizeimages.newest.+retpolines.pibe-opt.lmbench-work.999990.old sizeimages.newest.+retpolines.system.noopt sizeimages.newest.alldefenses.noopt sizeimages.newest.alldefenses.pibe-opt.lmbench-work.990000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999999  sizeimages.newest.+lvi.noopt sizeimages.newest.+lvi.pibe-opt.lmbench-work.990000 sizeimages.newest.+lvi.pibe-opt.lmbench-work.999999 sizeimages.newest.+retretpolines.noopt sizeimages.newest.+retretpolines.pibe-opt.lmbench-work.999999 alldefenses.pibe-opt.apache-work.999999) 
#CONFIGS=(sizeimages.newest.baseline.noopt sizeimages.newest.+retpolines+retretpolines.noopt sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.990000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999999 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999999  sizeimages.newest.+lvi.noopt sizeimages.newest.+lvi.pibe-opt.lmbench-work.990000 sizeimages.newest.+lvi.pibe-opt.lmbench-work.999999 sizeimages.newest.+retretpolines.noopt sizeimages.newest.+retretpolines.pibe-opt.lmbench-work.999999 alldefenses.pibe-opt.apache-work.999999) 
DIR=nginx_remote_default_1000_4clients

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
       ssh -t -o ConnectTimeout=10 $1@$2 "cd $3 && ./startup_nginx.sh"
       counter=$?
    done

    TIMES=11
    
    mkdir -p playground/performance/${DIR}/${CONF/sizeimages.newest./}
    tries=1
    while [ $tries -le $TIMES ]
    do
          echo "Benchmark number :"${tries}" for config:"${CONF/sizeimages.newest./}
          taskset 0x80 ab -c100 -n1000000 http://$2:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/apache1.${tries} &
          taskset 0x20 ab -c100 -n1000000 http://$2:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/apache2.${tries} &
          taskset 0x40 ab -c100 -n1000000 http://$2:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/apache3.${tries} &
          taskset 0x10 ab -c100 -n1000000 http://$2:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/apache4.${tries} &
          #taskset 0x10 ab -c100 -n1000000 http://$2:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/apache5.${tries} &
          #taskset 0x20 ab -c100 -n1000000 http://$2:80/index.html > playground/performance/${DIR}/${CONF/sizeimages.newest./}/apache6.${tries} &
          wait
          ((tries++))
    done
    ssh -t -o ConnectTimeout=10 $1@$2 "cd $3 && ./stop_nginx.sh"




  

done




