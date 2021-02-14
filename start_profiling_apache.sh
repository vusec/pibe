#!/bin/bash

sudo ./startup_apache.sh

cd lkm || exit
echo "Started apache"
make clean
make all
sudo insmod vuprofiler-module.ko
sudo ./mknod.sh
cd ..
