This file provides tips for visualising the results and mapping results back to the 
relevant tables from the main paper.
## Generating the results in pdf format
To assemble all results obtained while running the artifact into a pdf run the following
command from the root of the artifact:
```sh
$ ./generate_tables.sh
```
The script is essentially a wrapper over *visualise_results.sh* and will show in parallel
the results obtained on you machine vs the results in the main paper to which these results
match back to.

The resulting pdf is split in three subsections:
- First subsection shows LMBench overheads with/without optimizations for images that apply all 
  transient defenses (refers mostly to Table 5 from the main paper).
- Second subsection shows LMBench overheads for images that only apply retpolines (with/without
  optimizations (refers mostly to Table 3 from the main paper).
- Third subsection shows geometric overheads for each defense and the combination of all defenses (
  refers mostly to Table 6 from the main paper).

Each subsection contains an explanatory paragraph to describe how each column in the generated 
table can be matched back to the main results in the paper.



## What happens when calling visualise_results.sh.

When called in this manner:

```sh
$ ./visualise_results.sh -all
```

Will show the median LMBench micro-benchmark latencies for each kernel configuration described
in [Experiments.md](Experiments.md). The latencies must be previously obtained by running **run_artifact.sh**
as desribed in **Experiments.md**. Additionally, it will also compute the overhead of 
each micro-benchmark relative to its vanilla counterpart (dubbed baseline.noopt).
For each kernel configuration it will also output the mean geometric overhead
across all micro-benchmarks. (Look for the string "Geometric Mean Overhead:").

If you run the script in the following manner:

```sh
$ ./visualise_results.sh # no parameters
```
It will output reference results for the same experiment. The results were obtained by 
us on an Intel i7-8700K machine.

To compare against the PIBE baseline (dubbed baseline.pibe-opt.lmbench-work.999900) 
one must run the command:

```sh
$ ./visualise_results.sh -all -baselines baseline.pibe-opt.lmbench-work.999900
```

#### Mapping results back to the tables in the paper.

 - Results outputed by **./visualise_results.sh -all** for the **alldefenses.noopt** configuration map back
to Table 5 (Column **"regular LTO"**) -> the latencies and overheads of a kernel image without PIBE's
optimizations but with *retpolines*, *lvi-cfi*, and *return retpolines* enabled.

 - The results outputed for **alldefenses.pibe-opt.lmbench-work.999999** map back to
Table 5 (Column **"99.9999%"**) -> the latencies of a kernel image that also applies PIBE's optimizations
apart from enabling all transient execution defenses.

 - The geometric mean overhead outputed for the **alldefenses.pibe-opt.apache-work.999999** map back
to the overheads reported while benchmarking with LMBench but optimizing with an Apache workload
(reported in Section 8.4).
 - **+retretpolines.noopt**, **+retretpolines.pibe-opt.lmbench-work.999999** the geometric mean
   overheads map back to Table 6 line **"Return retpolines"**.
 - **+lvi.noopt**, **+lvi.pibe-opt.lmbench-work.999999** the geometric mean overheads map back to Table 6 line **"LVI-CFI"**

Note that for the +retpolines kernel configurations the previous invocation of the visualisation script 
will report smaller mean geometric overheads than in the paper. In the paper, in Section 8.2,  
we reported the geometric overhead across a subset of LMBench microbenchmarks that showed to 
be strongly impacted by the retpolines mitigation. To visualise only the results shown in the paper for retpolines
run the command:

```sh
$ ./visualise_results.sh -only-retpolines
```

The micro-benchmark latencies and median micro-benchmark overheads returned by this command map to 
Table 3 column **"regular LTO"** (the **+retpolines.noopt** configuration), and Table 3 column **"99.999%"**
(the **+retpolines.pibe-opt.lmbench-work.999990** configuration). Additionally, the geometric mean
overhead returned by this command maps back to Table 6 line **"Retpolines"**.

To compare the two baselines against each other you can run:

```sh
$ ./visualise_results.sh -baselines baseline.noopt -benchmarks baseline.pibe-opt.lmbench-work.999900
```
The latencies and geometric mean overhead reported by this command matches Table 2 in the main paper.

## Customizing the comparison.

If you want to compare specific configuration results you can use the general format of **visualise_results.sh**.
```sh
$ ./visualise_results.sh -baselines @base -benchmarks @bench1 @bench2 ..
```
@base, @bench1, @bench2, .. are relative names to "$PATH_TO_ARTIFACT/playground/performance".

Benchmark results obtained through **run_artifact.sh** have the same name as their corresponding
kernel configuration name described [here](Experiments.md).

You can use this feature if you created your own custom kernel configuration (check tips on [compilation](Compilation.md))
and benchmarked it via LMBench (as described in our [experiment](Experiments.md) tutorial).



## Customizing median selection
This is controlled via the same environment variable used to customize the number of benchmarking
iterations. In file **global_exports.sh** from the root directory of our evaluation source tree change the
following line:
```sh
export RUNS=5
```


