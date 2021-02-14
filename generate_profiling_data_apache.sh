echo "Usage:"
echo "./generate_profiling_data_apache.sh username remote_ip pibe_remote_top_dir workload_folder"
[[ !  -z  $1  ]] || exit
[[ !  -z  $2  ]] || exit
[[ !  -z  $3  ]] || exit
[[ !  -z  $4  ]] || exit

sudo apt install apache2-utils

ssh -t $1@$2 "cd $3 && ./start_profiling_apache.sh" || exit

sleep 2

echo "Starting Iterations"

TIMES=(0 1 2 3 4)

for TOKEN in ${TIMES[@]}; do
    echo "Profiling iteration "${TOKEN}
    ab -c100 -n1000000 http://$2:80/index.html
done

ssh -t $1@$2 "cd $3 && ./stop_profiling_apache.sh $4" || exit
