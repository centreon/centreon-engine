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

bin=$(locate /bin/nagiostats | head -n1)
data=$($bin)
if [ $? -ne 0 ]; then
    echo "read nagios statistics failed!"
    exit 2
fi
data=$(echo "$data" | grep 'Passive Hosts Last' | tr '/' ' ' | sed 's/  */ /g' | cut -d: -f2)

one=$(echo "$data" | cut -d ' ' -f2)
five=$(echo "$data" | cut -d ' ' -f3)
fifteen=$(echo "$data" | cut -d ' ' -f4)
sixty=$(echo "$data" | cut -d ' ' -f5)

output="PASSIVE HOSTS LAST - last_min: $one, last_five_min: $five, last_fifteen_min: $fifteen, last_sixty_min: $sixty"
perf_data="last_min=$one;last_five_min=$five;last_fifteen_min=$fifteen;last_sixty_min=$sixty"
echo "$output|$perf_data"
exit 0
