#!/bin/bash
. global_exports.sh

sudo ./stop_apache.sh

[[ !  -z  $1  ]] || exit

lkm/vuprof_ioctl check_overflow || exit

echo "Stopping apache profiling"

[ ! -d playground/${WORKLOAD_DIR} ] && mkdir -p ${WORKLOAD_DIR}
[ ! -d playground/${WORKLOAD_DIR}/$1 ] && mkdir -p playground/${WORKLOAD_DIR}/$1

lkm/vuprof_ioctl get_data > playground/${WORKLOAD_DIR}/$1/Profile.Map

sudo rmmod vuprofiler_module

./copy_kernel.sh $1

cp kernel/backport/Coverage.Map playground/${WORKLOAD_DIR}/$1

python3 include/profiler/vuprofiler_offline_analysis.py -f $1

python3 include/profiler/vuprofiler_offline_analysis.py -f $1 -i -o Decoded.Profiles.indirect
