#!/bin/bash
. global_exports.sh

cd ${WD}
echo "Usage:" && echo " ./generate_profiling_data.sh folder_name " && echo "Info:" && echo " Will create (binary) wokload file under ./playground/metadata/folder_name/Profile.Map and copy  the
system map, kernel binary and Coverage.Map in folder_name (these files are used to lift the binary profile to LLVM friendly profile). Finally it will generate Decoded.Profiles and Decoded.Profiles.indirect unde the same directory."

[[ !  -z  $1  ]] || exit

echo ${WORKLOAD_DIR}/$1/Profile.Map

TIMES=(0 1 2 3 4 5 6 7 8 9 10)

cd lkm
make clean
make all
sudo insmod vuprofiler-module.ko
./mknod.sh
cd ..

cd tools/lmbench3

rm results/x86_64-linux-gnu/*


for TOKEN in ${TIMES[@]}; do
    echo "Benchmark number "${TOKEN}
    make rerun
done


cd ../../playground

[ ! -d ${WORKLOAD_DIR} ] && mkdir -p ${WORKLOAD_DIR}
[ ! -d ${WORKLOAD_DIR}/$1 ] && mkdir -p ${WORKLOAD_DIR}/$1

rm -f ${WORKLOAD_DIR}/$1/Profile.Map


../lkm/vuprof_ioctl check_overflow

../lkm/vuprof_ioctl get_data > ${WORKLOAD_DIR}/$1/Profile.Map

sudo rmmod vuprofiler_module

cd ..

./copy_kernel.sh $1

cp kernel/backport/Coverage.Map playground/${WORKLOAD_DIR}/$1

python3 include/profiler/vuprofiler_offline_analysis.py -f $1

python3 include/profiler/vuprofiler_offline_analysis.py -f $1 -i -o Decoded.Profiles.indirect

