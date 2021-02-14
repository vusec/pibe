
##  Some tips to navigate the repo

 - **kernel/backport**: Linux Kernel 5.1.0 modified with support for LLVM LTO, and integrated with our optimization passes.
 - **playground/images**: Contains all precompiled kernel binaries for the evaluation (details [here](Experiments.md)).
 - **playground/performance**: After the experiment finishes you will find a folder for each benchmarked kernel configuration here.
 - **playground/metadata**: portable Apache2 (named apache2) and LMBench3 (dubbed lmbench3) workloads. All custom workloads created by user go here. 
 - **tools/lmbench3**: LMBench benchmarking suite v3 used for the evaluation.
 - **tools/llvm_10**: Modified LLVM 10 framework with support for *lvi-cfi* and *return retpolines*.
 - **lkm**: Profiling kernel module source tree.
 - **passes**: PIBE's LLVM passes. 
   - Most relevant passes are:
        - vuprofiler: enables our profiling instrumentation on the Linux Kernel.
        - linearpromotion: the indirect call promotion pass.
        - vuinliner: our PGO inlining pass.
        - icp_convert: transforms portable workloads obtained through profiling into LLVM sample-profile format.

   - To see how passes run in the kernel compilation pipeline check kernel/backport/scripts/link-vmlinux.sh for lto_incremental_modpost_link function.
 - **include**: general include directory (used by kernel, kernel module, LLVM passes etc.)

## Documentation

  1. In case you are using a kernel newer than 5.1.0 check [this](Grub2.md).
  2. All experiments enabled through our workflow are detailed [here](Experiments.md).
  3. For information on how to map results back to their corresponding tables check our [results](Results.md) tutorial.
  4. Tips about regenerating kernel binaries are provided [here](Compilation.md).
  5. For regenerating profiling workloads check our [profiling](Profiling.md) tutorial.
