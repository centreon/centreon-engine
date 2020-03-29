#!/bin/bash

typ=$1
state=$2

now=`date +%s`
commandfile='/tmp/var/lib/centreon-engine/rw/centengine.cmd'

if [[ $typ == "host" ]] ; then
  case $state in
  0)
    printf "[%lu] PROCESS_HOST_CHECK_RESULT;central;0;UP - Everything is GOOD\n" $now > $commandfile
    ;;
  1)
    printf "[%lu] PROCESS_HOST_CHECK_RESULT;central;1;DOWN - Everything is going WRONG\n" $now > $commandfile
    ;;
  esac
else
  case $state in
  0)
    printf "[%lu] PROCESS_SERVICE_CHECK_RESULT;central;service;0;OK - Everything is GOOD\n" $now > $commandfile
    ;;
  1)
    printf "[%lu] PROCESS_SERVICE_CHECK_RESULT;central;service;1;WARNING - Everything is going WRONG\n" $now > $commandfile
    ;;
  2)
    printf "[%lu] PROCESS_SERVICE_CHECK_RESULT;central;service;2;CRITICAL - Everything is DOWN\n" $now > $commandfile
    ;;
  esac
fi

exit $state
