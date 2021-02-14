This file describes the internal workings of our artifact's experiment.

In our discussion $PATH_TO_ARTIFACT refers to the absolute path to the root directory of 
the artifact source tree.

## How to run the artifact experiment?

From a remote machine that has access to the test machine (on which the artifact source tree is saved and on which benchmarking will happen) 
run the following command:

```sh
$ ./run_artifact.sh @user @ip /path/to/artifact/source/tree
```

@user must be a user from the test machine with **sudo** privileges.

@ip is the ip address of the remote machine.

The path points to the root directory of the artifact source tree on the test machine.

Example execution:

```sh
$ ./run_artifact.sh victor 192.168.0.104  /projects/clones/pibe-reproduce
```

## What does ./run_artifact.sh do?

Internally, the script, connects via ssh to the test machine and executes two actions for each kernel image provided
with the artifact:

1. Loads the kernel image on the test machine and reboots the machine.
2. After the image reboots it benchmarks the kernel using LMBench and saves the results in a separate directory 
   from "$PATH_TO_ARTIFACT/playground/performance" on the test machine.

## Prerequisites

- The ip address of the test machine must be persistent during reboots.

## How to make the experiment passwordless?

You must have a sudo user on the test machine or create one via the following shell commands:

```sh
$ adduser victor // To create user victor
$ sudo usermod -aG sudo victor // To add victor to sudoers
```
Then from the remote machine you from which you will run the experiment, first input the following command:
```sh
$ ./passwordless.sh victor ip // where ip is the ip address of the test machine.
```
Only **passwordless.sh** and **run_artifact.sh** must be copied from the root directory of the artifact tree,
to the remote machine.

## What loadable kernel configurations are provided with the artifact?

All kernel binaries are in "$PATH_TO_ARTIFACT/playground/images/", each in its separate directory. The directory
name and configuration for each kernel image (in the order in which ./run_artifact loads the kernels) are:
- **baseline.noopt**: LTO vanilla kernel (refered in the paper as the "LTO Baseline").
  - configuration: 
     - retpolines: **disabled**
     - lvi-cfi: **disabled**
     - return retpolines: **disabled**
     - PIBE optimizations: **disabled**

- **baseline.pibe-opt.lmbench-work.999900**: LTO vanilla kernel optimized via PIBE's inlining and indirect call promotion (refered in the paper as the "PIBE Baseline").
  - configuration: 
     - retpolines: **disabled**
     - lvi-cfi: **disabled**
     - return retpolines: **disabled**
     - PIBE optimizations: **enabled**
     - workload: **LMBench**
     - optimization budget: **99.99%**

- **alldefenses.noopt**: 
  - configuration: 
     - retpolines: **enabled**
     - lvi-cfi: **enabled**
     - return retpolines: **enabled**
     - pibe optimizations: **disabled**

- **alldefenses.pibe-opt.lmbench-work.999999**: 
  - configuration: 
     - retpolines: **enabled**
     - lvi-cfi: **enabled**
     - return retpolines: **enabled**
     - pibe optimizations: **enabled**
     - workload: **LMBench**
     - optimization budget: **99.9999%**

- **alldefenses.pibe-opt.apache-work.999999**: 
  - configuration: 
     - retpolines: **enabled**
     - lvi-cfi: **enabled**
     - return retpolines: **enabled**
     - PIBE optimizations: **enabled** 
     - workload: **Apache**
     - optimization budget: **99.9999%**

- **+retpolines.noopt**: 
  - configuration: 
     - retpolines: **enabled**
     - lvi-cfi: **disabled**
     - return retpolines: **disabled**
     - pibe optimizations: **disabled**

- **+retpolines.pibe-opt.lmbench-work.999990**: 
  - configuration: 
     - retpolines: **enabled**
     - lvi-cfi: **disabled**
     - return retpolines: **disabled**
     - PIBE optimizations: **just indirect call promotion enabled** 
     - workload: **LMBench**
     - optimization budget: **99.999%**

- **+retretpolines.noopt**: 
  - configuration: 
     - retpolines: **disabled**
     - lvi-cfi: **disabled**
     - return retpolines: **enabled**
     - pibe optimizations: **disabled**

- **+retretpolines.pibe-opt.lmbench-work.999999**: 
  - configuration: 
     - retpolines: **disabled**
     - lvi-cfi: **disabled**
     - return retpolines: **enabled**
     - PIBE optimizations: **enabled** 
     - workload: **LMBench**
     - optimization budget: **99.9999%**
- **+lvi.noopt**: 
  - configuration: 
     - retpolines: **disabled**
     - lvi-cfi: **enabled**
     - return retpolines: **disabled**
     - pibe optimizations: **disabled**

- **+lvi.pibe-opt.lmbench-work.999999**: 
  - configuration: 
     - retpolines: **disabled**
     - lvi-cfi: **enabled**
     - return retpolines: **disabled**
     - PIBE optimizations: **enabled** 
     - workload: **LMBench**
     - optimization budget: **99.9999%**

## How to manually load and benchmark each kernel image on the test machine?

Example: we want to load and benchmark "alldefenses.noopt".

From the root directory of the artifact (on the test machine), run the following shell commands:
```sh
$ ./load_kernel.sh alldefenses.noopt // also reboots the machine
$ ./benchmark_performance.sh alldefenses.noopt // after reboot
```
 This first command loads the image from "$PATH_TO_ARTIFACT/playground/images/alldefenses.noopt"
in the /boot directory and calls **update-grub2**. Additionally it reboots the machine.
The second command runs **LMBench** for a configurable number of iterations and then saves the benchmarking
results in "$PATH_TO_ARTIFACT/playground/performance/alldefenses.noopt". If you want to save
benchmarking results to a custom directory from "$PATH_TO_ARTIFACT/playground/performance", change the input
parameter supplied to the **benchmark_performance.sh** shell script. This is usefull when
creating your own custom kernel configuration and want to benchmark it.

## How to customize the number of benchmarking iterations?

By default, the experiment will run 5 LMBench iterations for each kernel configuration.
In file **global_exports.sh** from the root directory of our evaluation source tree change the
following line:
```sh
export RUNS=5
```

## How to remove/add new kernel configurations to be run by **run_artifact** ?

In **run_artifact.sh** modify the CONFIGS bash array.


