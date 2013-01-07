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

save_file="/tmp/.$(basename $1)_process_cpu.data"
pid=$(ps axf -o'pid comm' | grep "[^_] $1$" | sed 's/^ *//' | head -n1 | cut -d ' ' -f1)
# check pid information.
if [ -z "$pid" ] || [ ! -d "/proc/$pid" ]; then
    echo "process not running!"
    exit 3
fi

# get process cpu information.
now=$(date '+%s')
data_stat=$(cat "/proc/$pid/stat")
uptime=$(cat "/proc/uptime" | cut -d ' ' -f1 | sed 's/\.//')
utime=$(echo "$data_stat" | cut -d ' ' -f14)
stime=$(echo "$data_stat" | cut -d ' ' -f15)

# create first information.
if [ ! -e "$save_file" ]; then
    echo "$now $uptime $utime $stime" > $save_file
    echo "Initialize process_cpu."
    exit 1
# check file integrity.
elif [ $(wc -l "$save_file" | cut -d ' ' -f1) -ne 1 ]; then
    echo "$now $uptime $utime $stime" > $save_file
    echo "Invalid file, initialize process_cpu."
    exit 1    
fi

# get last process cpu information.
last_data=$(cat "$save_file")
last_time=$(echo "$last_data" | cut -d ' ' -f1)
last_uptime=$(echo "$last_data" | cut -d ' ' -f2)
last_utime=$(echo "$last_data" | cut -d ' ' -f3)
last_stime=$(echo "$last_data" | cut -d ' ' -f4)

# save process cpu information.
echo "$now $uptime $utime $stime" > $save_file

last_ptime=$(($last_utime + $last_stime))
curr_ptime=$(($utime + $stime))
cpu=$(echo "scale=2; ($curr_ptime - $last_ptime) * 100 / ($uptime - $last_uptime)" | bc)
user=$(echo "scale=2; ($utime - $last_utime) * 100 / ($uptime - $last_uptime)" | bc)
sys=$(echo "scale=2; ($stime - $last_stime) * 100 / ($uptime - $last_uptime)" | bc)

unit="%"
output="PROCESS CPU ($1) - user: $user$unit, system: $sys$unit, total=$cpu$unit"
perf_data="user=$user;system=$sys;total=$cpu"
echo "$output|$perf_data"
