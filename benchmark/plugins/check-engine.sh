#!/bin/bash

cmd=$(ps -o pid,user,%mem,%cpu,command ax | grep -v grep | grep centengine | sort -r | awk '{print $3,$4}')
if [[ $cmd == "" ]] ; then
  echo "centreon-engine: unknown status"
  exit 3
fi

mem=$(echo $cmd | awk '{print $1}')
cpu=$(echo $cmd | awk '{print $2}')
echo "centreon-engine state mem=$mem cpu=$cpu | mem=$mem%;;;0;100 cpu=$cpu%;;;0;100"
exit 0
