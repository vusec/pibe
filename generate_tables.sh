#!/bin/bash
[ ! -d paper/tables ] && mkdir -p paper/tables 
rm -f paper/reproduced.pdf
export RUNS=11
./visualise_tables.sh -baselines test.baseline.noopt -benchmarks test.alldefenses.noopt test.alldefenses.pibe-opt.lmbench-work.999999  test.alldefenses.pibe-opt.apache-work.999999 -sablon 'nooptimization' '99.9999(\%) LMBench work.' '99.9999(\%) Apache work.' > paper/tables/test-eval-lmbench-all.tex

./visualise_tables.sh -sablon 'nooptimization' '+icp (99.999\%)' -only-retpolines-test > paper/tables/test-eval-retpolines.tex

echo "\begin{tabular}{l|rr}" > paper/tables/test-eval-lmbench-geomeans.tex
echo "\multicolumn{1}{l|}{Defense}&\multicolumn{1}{l}{LTO}&\multicolumn{1}{l}{PIBE}\\\\" >> paper/tables/test-eval-lmbench-geomeans.tex
echo "\hline" >> paper/tables/test-eval-lmbench-geomeans.tex

./visualise_results.sh -baselines test.baseline.noopt -benchmarks  test.baseline.noopt  test.baseline.pibe-opt.lmbench-work.999900 | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo None | awk '{print $0" \\\\"}' >>  paper/tables/test-eval-lmbench-geomeans.tex
./visualise_results.sh -only-retpolines-test  | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo Retpolines | awk '{print $0" \\\\"}' >>  paper/tables/test-eval-lmbench-geomeans.tex
./visualise_results.sh -baselines test.baseline.noopt -benchmarks  test.+retretpolines.noopt test.+retretpolines.pibe-opt.lmbench-work.999999 | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo Return retpolines | awk '{print $0" \\\\"}' >>  paper/tables/test-eval-lmbench-geomeans.tex

export RUNS=21
./visualise_results.sh -baselines test.baseline.noopt -benchmarks  test.+lvi.noopt test.+lvi.pibe-opt.lmbench-work.999999 | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo LVI-CFI  | awk '{print $0" \\\\"}' >>  paper/tables/test-eval-lmbench-geomeans.tex

export RUNS=11
./visualise_results.sh -baselines test.baseline.noopt -benchmarks  test.alldefenses.noopt test.alldefenses.pibe-opt.lmbench-work.999999 | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo All | awk '{print $0" \\\\"}' >>  paper/tables/test-eval-lmbench-geomeans.tex

echo "\end{tabular}" >> paper/tables/test-eval-lmbench-geomeans.tex

unset RUNS
./visualise_tables.sh -baselines baseline.noopt -benchmarks alldefenses.noopt alldefenses.pibe-opt.lmbench-work.999999  alldefenses.pibe-opt.apache-work.999999 -sablon 'nooptimization' '99.9999(\%) LMBench work.' '99.9999(\%) Apache work.' > paper/tables/eval-lmbench-all.tex

./visualise_tables.sh -sablon 'nooptimization' '+icp (99.999\%)' -only-retpolines > paper/tables/eval-retpolines.tex

echo "\begin{tabular}{l|rr}" > paper/tables/eval-lmbench-geomeans.tex
echo "\multicolumn{1}{l|}{Defense}&\multicolumn{1}{l}{LTO}&\multicolumn{1}{l}{PIBE}\\\\" >> paper/tables/eval-lmbench-geomeans.tex
echo "\hline" >> paper/tables/eval-lmbench-geomeans.tex

./visualise_results.sh -baselines baseline.noopt -benchmarks  baseline.noopt  baseline.pibe-opt.lmbench-work.999900 | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo None | awk '{print $0" \\\\"}' >>  paper/tables/eval-lmbench-geomeans.tex

./visualise_results.sh -only-retpolines  | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo Retpolines  | awk '{print $0" \\\\"}' >>  paper/tables/eval-lmbench-geomeans.tex
./visualise_results.sh -baselines baseline.noopt -benchmarks  +retretpolines.noopt +retretpolines.pibe-opt.lmbench-work.999999 | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo Return retpolines  | awk '{print $0" \\\\"}' >>  paper/tables/eval-lmbench-geomeans.tex

./visualise_results.sh -baselines baseline.noopt -benchmarks  +lvi.noopt +lvi.pibe-opt.lmbench-work.999999 | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo LVI-CFI  | awk '{print $0" \\\\"}' >>  paper/tables/eval-lmbench-geomeans.tex

./visualise_results.sh -baselines baseline.noopt -benchmarks  alldefenses.noopt alldefenses.pibe-opt.lmbench-work.999999 | grep Geometric |   sed  "s/%/'\\\\'%/"| awk '{print "&"$4}' | xargs echo All  | awk '{print $0" \\\\"}' >>  paper/tables/eval-lmbench-geomeans.tex

echo "\end{tabular}" >> paper/tables/eval-lmbench-geomeans.tex




cd paper && pdflatex reproduced.tex && pdflatex reproduced.tex && cd ..

rm -f paper/reproduced.log paper/reproduced.aux
