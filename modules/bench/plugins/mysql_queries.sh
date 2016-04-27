#!/bin/sh

if [ $# -ne 3 ]; then
    echo "usage: $0 host user password"
    exit 1
fi

host="$1"
user="$2"
password="$3"
save_file="/tmp/.mysql_queries_$host-$user.data"

now=$(date '+%s')
nb_queries="$(echo 'show status;' | mysql -h$host -u$user -p$password | grep '^Queries' | cut  -f2)"
if [ -n "$(echo $nb_queries | grep '^ERROR')" ]; then
    echo "$nb_queries"
    exit 1
fi

# create first information.
if [ ! -e "$save_file" ]; then
    echo "$now $nb_queries" > $save_file
    echo "Initialize mysql_queries."
    exit 1
# check file integrity.
elif [ $(wc -l "$save_file" | cut -d ' ' -f1) -ne 1 ]; then
    echo "$now $nb_queries" > $save_file
    echo "Invalid file, initialize mysql_queries."
    exit 1    
fi

# get last mysql queries information.
last_data=$(cat "$save_file")
last_time=$(echo "$last_data" | cut -d ' ' -f1)
last_nb_queries=$(echo "$last_data" | cut -d ' ' -f2)

# save mysql queries information.
echo "$now $nb_queries" > $save_file

# calculate.
interval=$(($now - $last_time))
nb_queries=$(echo "scale=2; ($nb_queries - $last_nb_queries) / $interval" | bc)

unit="q/s"
output="MYSQL QUERIES ($host-$user) - nb_queries: $nb_queries$unit"
perf_data="nb_queries=$nb_queries"
echo "$output|$perf_data"

exit 0