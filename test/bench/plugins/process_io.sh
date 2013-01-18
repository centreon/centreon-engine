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

save_file="/tmp/.$(basename $1)_process_io.data"
pid=$(ps axf -o'pid comm' | grep "[^_] $1$" | sed 's/^ *//' | head -n1 | cut -d ' ' -f1)
# check pid information.
if [ -z "$pid" ] || [ ! -d "/proc/$pid" ]; then
    echo "process not running!"
    exit 3
fi

# get process io information.
now=$(date '+%s')
data=$(cat "/proc/$pid/io" | cut -d ' ' -f 2)
read=$(echo "$data" | head -n5 | tail -n1)
write=$(echo "$data" | head -n6 | tail -n1)
cancel=$(echo "$data" | head -n7 | tail -n1)

# create first information.
if [ ! -e "$save_file" ]; then
    echo "$now $read $write $cancel" > $save_file
    echo "Initialize process_io"
    exit 1
# check file integrity.
elif [ $(wc -l "$save_file" | cut -d ' ' -f1) -ne 1 ]; then
    echo "$now $read $write $cancel" > $save_file
    echo "Invalid file, initialize process_io"
    exit 1    
fi

# get last process io information.
last_data=$(cat "$save_file")
last_time=$(echo "$last_data" | cut -d ' ' -f1)
last_read=$(echo "$last_data" | cut -d ' ' -f2)
last_write=$(echo "$last_data" | cut -d ' ' -f3)
last_cancel=$(echo "$last_data" | cut -d ' ' -f4)

# save process io information.
echo "$now $read $write $cancel" > $save_file

# calculate.
interval=$(($now - $last_time))
read=$(echo "scale=2; ($read - $last_read) / $interval" | bc)
write=$(echo "scale=2; ($write - $last_write) / $interval" | bc)
cancel=$(echo "scale=2; ($cancel - $last_cancel) / $interval" | bc)

unit="b/s"
output="PROCESS IO ($1) - read_bytes: $read$unit, write_bytes: $write$unit, cancelled_write_bytes: $cancel$unit"
perf_data="read_bytes=$read;write_bytes=$write;cancelled_write_bytes=$cancel"
echo "$output|$perf_data"

exit 0