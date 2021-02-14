#!/bin/bash

[[ !  -z  $1  ]] || exit
[[ !  -z  $2  ]] || exit


if [[ "$2" == "profiler" ]]; then
    echo "Profiler configuration"
    CONFIG_ARR=( HAVE_VUPROFILER=True )
    BUILD_ARR+=( HAVE_VUPROFILER=True )
    MODULE_ARR+=( HAVE_VUPROFILER=True )
fi 

if [[ "$2" == "baseline" ]]; then
    [[ !  -z  $3  ]] || exit
    echo "Baseline configuration"
fi

RETPOLINES_ONLY=False
if [[ "$2" == "+retpolines" ]]; then
    [[ !  -z  $3  ]] || exit
    echo "+retpolines only configuration"
    RETPOLINES_ONLY=True
    BUILD_ARR+=( ENABLE_RETPOLINES=True )
else
  if [[ "$2" == *"+retpolines"* ]]; then
    [[ !  -z  $3  ]] || exit
    echo "+retpolines configuration"
    BUILD_ARR+=( ENABLE_RETPOLINES=True )
  fi
  if [[ "$2" == *"+lvi"* ]]; then
    [[ !  -z  $3  ]] || exit
    echo "+lvi configuration"
    BUILD_ARR+=( ENABLE_LVI=True )
  fi

  if [[ "$2" == *"+retretpolines"* ]]; then
    [[ !  -z  $3  ]] || exit
    echo "+retretpolines configuration"
    BUILD_ARR+=( ENABLE_RETSPEC=True )
  fi

fi

BUILD_ARR+=( DEFENSE_CONFIGURATION=$2 )

if [[ "$3" == "nooptimization" ]]; then
    echo "Disable optimizations"
fi

if [[ "$3" == "optimizations" ]]; then
    [[ !  -z  $4  ]] || exit
    [[ !  -z  $5  ]] || exit
    BUILD_ARR+=( HAVE_ICP=LLVM )
    if [[ "$RETPOLINES_ONLY" == "True" ]]; then
       echo "Retpoline only optimization."
       BUILD_ARR+=( WORKLOAD_FILE=Decoded.Profiles.indirect )
       BUILD_ARR+=( PIBE_OPTIMIZATIONS=Promote )
    fi
    BUILD_ARR+=( WORKLOAD_FOLDER=$4 )
    BUILD_ARR+=( PIBE_BUDGET=$5 )
   
    
fi


if [[ "$1" == "hard-compile" ]]; then
    echo "Full kernel compilation."
    echo "${CONFIG_ARR[@]}"
    set -x ; ./clean_kernel.sh && cd kernel && make config_backport "${CONFIG_ARR[@]}" && make build_kernel_backport "${BUILD_ARR[@]}" && make build_modules_backport "${MODULE_ARR[@]}" && make install_modules_backport  "${MODULE_ARR[@]}" && make install_image_backport "${MODULE_ARR[@]}" && cd .. 
fi 

if [[ "$1" == "soft-compile" ]]; then
    echo "Soft kernel compilation (only link-time changes)"
    set -x ; ./clean_kernel.sh && cd kernel && rm -f backport/vmlinux && make build_kernel_backport "${BUILD_ARR[@]}" && make build_modules_backport "${MODULE_ARR[@]}" && make install_modules_backport  "${MODULE_ARR[@]}" && make install_image_backport "${MODULE_ARR[@]}" && cd ..
fi 
