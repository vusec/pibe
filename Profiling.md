
This file describes the steps needed in order to automatically generate PIBE's profiling workloads:
 - LMBench profiling workload, discussed in Section 8 of the main paper.
 - Apache2 remote workload discussed in Section 8.4 of the main paper.

## What do you need?
An 64-bit Intel CPU equipped with the Last Branch Recording profiling feature. You can
check if your system has this feature if you type in **dmesg | grep -i lbr** in the Ubuntu
terminal. This command should return a message containing a string of the form "N-deep LBR".
We suggest testing on Intel CPU's with at least 16 LBR entries (eg., Nehalem, Haswell) or
32 LBR entries (eg., Skylake). 

## Steps to generate workloads.

#### 1. Compile the Linux Kernel as a profiler.
First one must compile the 5.1.0 Linux Kernel provided with the artifact 
(in $PATH_TO_ARTIFACT/kernel/backport) with support for profiling function calls. 

To compile the kernel as a profiler type in the following command from the root 
of our provided artifact source tree:
```sh
$ ./compile_install_kernel.sh hard-compile profiler
$ sudo reboot // reboot the machine afterwards 
```
This will add instrumentation to the kernel that retrieves branch statistics 
using Intel's LBR feature (as well as a way to remap the statistics back 
to the LLVM IR of the kernel). The command will also install the profiling 
kernel on the machine, alongside all other kernels already installed on the machine. 

If you were previously running a kernel newer than version 5.1.0, during a reboot
you must manually select the 5.1.0 kernel when the Grub2 boot screen apears.
First select "Advanced options for Ubuntu" menu option after which you
select kernel 5.1.0 from the list of available kernels. If the bootloader menu 
screen does not appear automatically hold either SHIFT or ESC pressed while the 
system is rebooting. If you are running kernel versions older than 5.1.0 the 
profiling kernel will be first in boot order (ideal if testing the artifact via a 
remote ssh connection to the test machine).

#### 2. Generate the workloads.

##### 2.1. Generate LMBench3 Workload:
From within the freshly booted profiling image type the following command in the console:
```sh
$ ./generate_profiling_data.sh @workload_folder // To generate LMBench 3 workload

```
##### 2.2. Generate Apache2 Workload:
From a remote machine (that has ssh access to the test machine) type in the following 
command:
```sh
$ ./generate_profiling_data_apache.sh @remote_user @remote_ip /path/to/artifact/source/tree @workload_folder

```
@workload_folder is a user supplied string representing the folder (name not path) in which the workload will
be saved (must be unique for each generated workload). Both commands will create a 
workload folder in the $PATH_TO_ARTIFACT/playground/metadata directory. Each folder will contain
metadata files used to remap execution statistics back to the LLVM IR of the kernel and 
are used by our indirect call promotion and inlining optimization passes (only one workload
can be used at a time when optimizing).

@remote_ip is the remote IP of the test machine, while @remote_user is a user with sudo privileges
on the test machine (we need sudo privileges to start Apache2 server from the remote and to 
load the kernel module used to manage kernel branch statistics during workload generation).

/path/to/artifact/source/tree -> points to the root path of the artifact (on the test machine).

#### 3. Using the workloads.
Assuming that in the previous step you supplied the workload folders: apache_custom (for Apache)
and lmbench_custom (for LMBench) then one can use these workloads to optimize a kernel configured
with transient defenses in the following manner:

```sh
# To optimize at a 99% budget via the apache workload a kernel configured with LVI-CFI and Retpolines.
$ ./compile_install_kernel.sh hard-compile +retpolines+lvi optimizations apache_custom 990000
# To optimize at a 99.9% budget via the lmbench workload a kernel configured with Return Retpolines.
$ ./compile_install_kernel.sh hard-compile +retretpolines optimizations lmbench_custom 999000

```
#### 4. Provided workloads.
The artifact comes with our Apache and LMBench profiling workloads obtained on a Intel i7-8700K (Skylake) CPU.
The names of the workloads are lmbench3 (for LMBench) and apache2 (for Apache). Make sure not to use
these names in case you are planning to generate your own workloads as this will overwrite our
provided workloads.


