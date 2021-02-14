#!/bin/bash
arg=notfull
[[ ! -z  $1  ]] && arg=$1

if [[ "$arg" == "-full" ]]; then
    echo "Full configuration"
    sudo apt update
    # We require stdc++ for C++ headers (used when compiling LLVM passes)
    sudo apt install build-essential linux-libc-dev

    # Fix for Ubuntu 16.04 running on 64-bit system
    [ ! -d /usr/include/asm ] && sudo ln -s /usr/include/x86_64-linux-gnu/asm /usr/include/asm

    # CMake is used when compiling the LLVM source
    sudo apt-get install cmake
    # Flex and Bison may be needed when compiling binutils
    sudo apt-get install flex
    sudo apt-get install bison
    # SSL is required when compiling the kernel (else you will get a missing bio.h error)
    sudo apt-get install libssl-dev 


    # Install binutils and llvm_10
    make -C tools all

    # Remove extra folders after tools are built.
    sudo rm -r tools/build_llvm_10
    sudo rm -r tools/build_binutils

    # Compile all LLVM passes
    make -C passes all

    # Install and configure Apache2
    sudo ./install_apache.sh
fi
# Install latex to output results in a pdf.
sudo apt-get install texlive-latex-base
# Python3 is the base python interpreter used by our .py scripts
sudo apt-get install python3

sudo apt install python3-pip

sudo pip3 install --upgrade pip3 || sudo pip3 install --upgrade pip

sudo pip3 install numpy


# Configure LMBench. Note that this will also run LMBench so CTRL+C after you see the "Latency measurements" message
cd tools/lmbench3/ && cat lmbench.config | make results
