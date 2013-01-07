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

pid=$(ps axf -o'pid comm' | grep "[^_] $1$" | sed 's/^ *//' | head -n1 | cut -d ' ' -f1)
# check pid information.
if [ -z "$pid" ] || [ ! -d "/proc/$pid" ]; then
    echo "process not running!"
    exit 3
fi

# get process memory information.
data=$(cat "/proc/$pid/stat")
vm_size=$(echo "$data" | cut -d ' ' -f 23)
vm_rss=$(($(echo "$data" | cut -d ' ' -f 24) * $(getconf PAGESIZE)))

unit="octets"
output="PROCESS MEMORY ($1) - vm_size: $vm_size$unit, vm_rss: $vm_rss$unit"
perf_data="vm_size=$vm_size;vm_rss=$vm_rss"
echo "$output|$perf_data"
exit 0