#!/bin/bash

#./run_nginx_wrk.sh  victor 10.0.0.3 /tools/reproductions/pibe-reproduction 400 20m nginx_400_20min_affinity
#./run_apache_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 400 20m apache_400_20min_affinity
#./run_nginx_wrk.sh  victor 10.0.0.3 /tools/reproductions/pibe-reproduction 50 20m nginx_50_20min_affinity
#./run_apache_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 100 20m apache_100_20min_affinity
#./run_nginx_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 100 20m nginx_100_20min_affinity
./run_apache_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 150 10m apache_150_10min_affinity
./run_nginx_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 150 10m nginx_150_10min_affinity
./run_apache_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 100 60m apache_100_60min_affinity
./run_nginx_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 100 60m nginx_100_60min_affinity
#./run_apache_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 25 5m apache_25_5min_affinity
#./run_nginx_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 25 5m nginx_25_5min_affinity
#./run_apache_wrk.sh victor 10.0.0.3 /tools/reproductions/pibe-reproduction 100 20m apache_400_20min

