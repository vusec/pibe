#!/bin/bash
. global_exports.sh
cd playground/performance/ && python3 print_lmbench_median_latencies.py "$@" && cd ../..
