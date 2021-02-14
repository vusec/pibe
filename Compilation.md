This file provides information on how to regenerate kernel binaries
discussed in the [experiments](Experiments.md) tutorial or create new 
custom made kernel builds.

## Prerequisites

First, one must run the dependencies install script with the -full
triger to compile binutils, the LLVM 10 framework and our optimization
passes:
```sh
$ ./install_tools_and_deps.sh -full
```
The command should be run from the root directory of our provided artifact 
(either obtained from github or Zotero).

## Compiling a new kernel (via our shell script)

To compile a new kernel one must make use of the **compile_install_kernel.sh**
(placed in the root directory of our source tree).

The general format of this command is as follows:

```sh
$ ./compile_install_kernel.sh compilation-trigger configuration-trigger optimization-trigger
```
The script will compile the kernel 5.1.0 provided with our evaluation source tree in 
"$PATH_TO_ARTIFACT/kernel/backport". This is a public kernel (version 5.1.0) modified
by us to integrate our optimization passes in the kernel build process.

The values acceptable for the triggers are as follows:
 - compilation-trigger:
   - hard-compile:
        - compiles the kernel from scratch (all previous compiled source files are rebuilt).
        - must be used if we are about to compile a kernel with a different configuration trigger
          than the previous build.
   - soft-compile:
        - only performs re-linking of kernel archives.
        - used if we are about to compile the kernel using the same configuration trigger as the
          previous build but with a different optimization trigger.
 - configuration-trigger:
   - profiler:
        - will build the kernel with profiling support (check tips on [profiling](Profiling.md) for an example use-case).
   - baseline:
        - will build the kernel with retpolines, lvi and return retpolines disabled.

   - +feature1..+featureN:
        - will build the kernel with the supplied features enabled. Possible values for featureX
           are: retpolines, lvi and retretpolines (eg., to compile a kernel with lvi and return
           retpolines enabled supply +lvi+retretpolines as configuration trigger).
 - optimization-trigger: only relevant for baseline and +feature.. configuration triggers.
   - nooptimization:
        - will build the kernel with the configuration specified by the configuration-trigger
           but without PIBE's optimization passes.
   - optimizations @workload_folder @budget:
        - will build the kernel under the configuration specified by the configuration-trigger
           using PIBE's indirect call promotion and inlining PGO passes with the given workload
           and the given optimization budget.
        - @workload_folder is a folder that points to a profiling workload (we already provide
           lmbench3 and apache2 folders to be used for this parameter).
        - @budget is a user supplied optimization budget (if you want to profile at an optimization
           budget of 97.1234% for example supply as budget the number 971234). 

**NOTES**: For a recommended alternative approach into compiling kernel configurations please refer to the
last topic in this section.
## Regenerating kernel configurations

In order to regenerate one of the kernel images discussed in [Experiments](Experiments.md) you must 
check the configuration for that specific kernel image and its configuration name.

To regenerate **+lvi.pibe-opt.lmbench-work.999999** image (i.e., only LVI-CFI enabled, optimized
with lmbench at 99.9999% budget) one must execute the following commands:
```sh
$ ./compile_install_kernel.sh hard-compile +lvi optimizations lmbench3 999999 
$ cd playground/images && ./make_backup.sh +lvi.pibe-opt.lmbench-work.999999 && cd ../..
```
The first command compiles that certain configuration and installs it in /boot directory.
The second command will make a backup with the name **+lvi.pibe-opt.lmbench-work.999999** in
"$PATH_TO_ARTIFACT/playground/images". The second command will essentially overwrite 
the specific configuration already supplied with the artifact. Next time **./run_artifact.sh**
will load that specific configuration it will use the freshly compiled kernel.

Assuming we previously compiled **+lvi.pibe-opt.lmbench-work.999999** then in order to
regenerate an image without optimizations but with the same **+lvi** configuration one
can execute:
```sh
$ ./compile_install_kernel.sh soft-compile +lvi nooptimization
$ cd playground/images && ./make_backup.sh +lvi.noopt && cd ../..
```
Assuming we previously compiled the **+lvi.noopt** and now we want to compile an image with
all transient defenses enabled but no optimization we are forced to use the hard-compile trigger
as the configuration changed.

To compile the LTO baseline and then overwrite the one that we supplied in "$PATH_TO_ARTIFACT/playground/images"
one must execute:
```sh
$ ./compile_install_kernel.sh hard-compile baseline nooptimization
$ cd playground/images && ./make_backup.sh baseline.noopt && cd ../..
```

## Some examples of creating new configurations

The user is not limited to the configurations supplied with the [artifact](Experiments.md) and can create his
own custom configurations and save them in a folder of his choosing within  
"$PATH_TO_ARTIFACT/playground/images".

For example if we want to create an image that uses only retpolines and return retpolines but no
optimization one can write:
```sh
$ ./compile_install_kernel.sh hard-compile +retpolines+retretpolines nooptimization
```
If he plans to benchmark it standalone (without using **run_artifact.sh**) it's not
necessary to save a backup.

After the previous command was run he can boot in the new kernel configuration and benchmark it
by using:
```sh
$ sudo reboot // reboots machine
$ ./benchmark_performance.sh folderabcd
```
This command will save the LMBench results obtained when benchmarking the new configuration in 
"$PATH_TO_ARTIFACT/playground/performance/folderabcd". In order to compare the results obtained
on this configuration with other results (either obtained with **run_artifact.sh** or from
other custom builds) you can use the general format of **visualise_results.sh** script (check
[here](Results.md) under "Customizing the comparison").

If the user wants to create a new configuration and then include it in the **run_artifact.sh**
workflow, after calling **compile_install_kernel.sh** he must save the configuration in 
playground/images/ then include the configuration name in the CONFIGS array defined in
**run_artifact.sh**.

Example:
```sh
$ ./compile_install_kernel.sh ...
$ cd playground/images && ./make_backup.sh dummyname && cd ../..
```
Then modify the CONFIGS array in **run_artifact.sh** to something like:
```sh
CONFIGS=(baseline.noopt .. dummyname )
```

#### Using workloads.

One can make use of our predefined workloads for Apache2 and LMBench3 or recreate his own
profiling workloads as discussed [here](Profiling.md). The workload names for the predefined
workloads are apache2 and lmbench3 respectively.

For example, if we want to create an image with lvi and return retpolines enabled and optimized with
Apache workload at an optimization budget of 99.9% one has to input:
```sh
$ ./compile_install_kernel.sh hard-compile +lvi+retretpolines optimizations apache2 999000
```
If the user created his own workload and wants to optimize with it he must specify the
name in place of "apache2", in the previous command.

### Compiling a new kernel (via our python script)
When changing from a debug to a release version of the LLVM framework we noticed that the kernel compilation
may hang indefinetly (happened 2 out of 35 compilations and it does not seem to be related to our compiler
changes). The fix for the issue is simply restarting the kernel compilation from where it previous hanged
(via the soft-compile compilation trigger). 

Provided you want to compile your own kernel config we recommend an alternative approach. Use our python
script as follows (from the root of our provided artifact):
```sh
$ python3 compile_install_kernel.py @configuration
```
@configuration is a configuration string similar to those discussed in [Experiments](Experiments.md).
Internally the script will call the same **compile_install_kernel.sh** kernel but will also install a daemon
that verifies if the compilation hanged. In the unfortunate case in which the compilation hangs (might
never happen) our provided script will restart the compilation using the soft-compile trigger.

For example, to compile a configuration that enables all defenses, optimized using lmbench as workload at 
a budget of 99.97% type in:

```sh
$ python3 compile_install_kernel.py alldefenses.pibe-opt.lmbench-work.999700
```

To compile a kernel that only enables lvi and optimizes using an apache workload at a budget of 99.98%
type in:

```sh
$ python3 compile_install_kernel.py +lvi.pibe-opt.apache-work.999800
```
 




   
           
