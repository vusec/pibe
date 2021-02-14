#!/bin/bash
echo "Usage:"
echo "./run_remote_artifact.sh @remote_username @remote_ip @remote_artifact_top_dir"
echo " @remote_username must be in sudoers"
[[ !  -z  $1  ]] || exit
[[ !  -z  $2  ]] || exit
[[ !  -z  $3  ]] || exit
CONFIGS=(sizeimages.newest.baseline.noopt sizeimages.newest.+retpolines.noopt.old sizeimages.newest.+retpolines.pibe-opt.lmbench-work.999990.old sizeimages.newest.+retpolines.system.noopt sizeimages.newest.+retpolines+retretpolines.noopt sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.990000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999000 sizeimages.newest.+retpolines+retretpolines.pibe-opt.lmbench-work.999999 sizeimages.newest.alldefenses.noopt sizeimages.newest.alldefenses.pibe-opt.lmbench-work.990000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999000 sizeimages.newest.alldefenses.pibe-opt.lmbench-work.999999  sizeimages.newest.+lvi.noopt sizeimages.newest.+lvi.pibe-opt.lmbench-work.990000 sizeimages.newest.+lvi.pibe-opt.lmbench-work.999999) 


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
       ssh -t -o ConnectTimeout=10 $1@$2 "cd $3 && sudo mount -t tmpfs -o rw,size=50G tmpfs /mnt/ramdisk && mkdir -p playground/performance/dbench.1250clients.tmpfs &&  dbench -D /mnt/ramdisk -t 900 1250  > playground/performance/dbench.1250clients.tmpfs/dbench.${CONF/sizeimages.newest./}"
       counter=$?
    done
  

done


