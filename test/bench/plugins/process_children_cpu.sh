#!/bin/sh
##
## Copyright 2012-2013 Merethis
##
## This file is part of Centreon Engine.
##
## Centreon Engine is free software: you can redistribute it and/or
## modify it under the terms of the GNU General Public License version 2
## as published by the Free Software Foundation.
##
## Centreon Engine is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Centreon Engine. If not, see
## <http://www.gnu.org/licenses/>.
##

# check script argument.
if [ $# -ne 1 ]; then
    echo "usage: $0 process_name"
    exit 3
fi

save_file="/tmp/.$(basename $1)_process_children_cpu"
pid=$(ps axf -o'pid comm' | grep "[^_] $1$" | sed 's/^ *//' | head -n1 | cut -d ' ' -f1)
# check pid information.
if [ -z "$pid" ] || [ ! -d "/proc/$pid" ]; then
    echo "process not running!"
    exit 3
fi

# get process children cpu information.
now=$(date '+%s')
data_stat=$(cat "/proc/$pid/stat")
uptime=$(cat "/proc/uptime" | cut -d ' ' -f1 | sed 's/\.//')
cutime=$(echo "$data_stat" | cut -d ' ' -f16)
cstime=$(echo "$data_stat" | cut -d ' ' -f17)

# create first information.
if [ ! -e "$save_file" ]; then
    echo "$now $uptime $cutime $cstime" > $save_file
    echo "Initialize children_cpu."
    exit 1
# check file integrity.
elif [ $(wc -l "$save_file" | cut -d ' ' -f1) -ne 1 ]; then
    echo "$now $uptime $cutime $cstime" > $save_file
    echo "Invalid file, initialize children_cpu."
    exit 1    
fi

# get last process children cpu information.
last_data=$(cat "$save_file")
last_time=$(echo "$last_data" | cut -d ' ' -f1)
last_uptime=$(echo "$last_data" | cut -d ' ' -f2)
last_cutime=$(echo "$last_data" | cut -d ' ' -f3)
last_cstime=$(echo "$last_data" | cut -d ' ' -f4)

# save process children cpu information.
echo "$now $uptime $cutime $cstime" > $save_file

last_ptime=$(($last_cutime + $last_cstime))
curr_ptime=$(($cutime + $cstime))
cpu=$(echo "scale=2; ($curr_ptime - $last_ptime) * 100 / ($uptime - $last_uptime)" | bc)
user=$(echo "scale=2; ($cutime - $last_cutime) * 100 / ($uptime - $last_uptime)" | bc)
system=$(echo "scale=2; ($cstime - $last_cstime) * 100 / ($uptime - $last_uptime)" | bc)

unit="%"
output="PROCESS CHILDREN CPU ($1) - user: $user$unit, system: $system$unit, total=$cpu$unit"
perf_data="user=$user;system=$system;total=$cpu"
echo "$output|$perf_data"
