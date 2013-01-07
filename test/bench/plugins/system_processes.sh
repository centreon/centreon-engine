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

save_file="/tmp/.system_processes.data"

# get processes information.
now=$(date '+%s')
nb_process=$(cat "/proc/stat" | grep '^processes' | cut -d ' ' -f2)

# create first information.
if [ ! -e "$save_file" ]; then
    echo "$now $nb_process" > $save_file
    echo "Initialize system_processes."
    exit 1
# check file integrity.
elif [ $(wc -l "$save_file" | cut -d ' ' -f1) -ne 1 ]; then
    echo "$now $uptime $utime $stime" > $save_file
    echo "Invalid file, initialize system_processes."
    exit 1
fi

# get last processes information.
last_data=$(cat "$save_file")
last_time=$(echo "$last_data" | cut -d ' ' -f1)
last_nb_process=$(echo "$last_data" | cut -d ' ' -f2)

# save processes information.
echo "$now $nb_process" > $save_file

interval=$(($now - $last_time))
nb_process=$(echo "scale=2; ($nb_process - $last_nb_process) / $interval" | bc)

unit="p/s"
output="SYSTEM CREATE PROCESSES - nb_processes: $nb_process$unit"
perf_data="nb_processes=$nb_process"
echo "$output|$perf_data"
