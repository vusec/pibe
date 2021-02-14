#!/bin/bash

[[ !  -z  $1  ]] && echo Specify a folder to save the backup $1 || exit

ssh victor@bench 'mkdir /projects/icp_kernel/playground/images/'$1''
scp -r $1/*-5.1.0 victor@bench:/projects/icp_kernel/playground/images/$1
