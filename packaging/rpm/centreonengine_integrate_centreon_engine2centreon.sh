#!/bin/bash

USE_CENTREON=0

##
# Help function
##
function help_usage
{
    echo "Usage: $(basename $0) [-c] [-h]"
    echo "	-h	This help"
    echo "	-c	Add right for centreon user and configuration (Requires Centreon RPMs)"
}

# Parse options
while [ $# -gt 0 ]; do
   if [ "$1" = "-h" ]; then
       help_usage
       exit 0
   elif [ "$1" = "-c" ]; then
       USE_CENTREON=1
   else
       echo "Bad arguments" >&2
       help_usage
       exit 1
   fi
   shift
done

#
# Variables
#
RES_COL="60"
MOVE_TO_COL="\\033[${RES_COL}G"
SETCOLOR_SUCCESS="\\033[1;32m"
SETCOLOR_FAILURE="\\033[1;31m"
SETCOLOR_NORMAL="\\033[0;39m"

##
# Print a status
#
# $1 the code if equal 0 Ok , else nok
##
function print_status
{
  if [ $1 -eq 0 ]; then
    echo -e "${MOVE_TO_COL}[${SETCOLOR_SUCCESS}OK${SETCOLOR_NORMAL}]"
  else
    echo -e "${MOVE_TO_COL}[${SETCOLOR_FAILURE}NOK${SETCOLOR_NORMAL}]"
  fi
}


if id apache &>/dev/null; then
  RESTART_APACHE=0
  echo "Add groups to apache user"
  id apache | grep "(centreon-engine)" &>/dev/null
  if [ $? -ne 0 ]; then
    echo -n "* Add group centreon-engine"
    usermod -aG centreon-engine apache
    print_status $?
    RESTART_APACHE=1
  fi
  id apache | grep "(centreon)" &>/dev/null
  if getent group centreon &>/dev/null; then
    if [ $? -ne 0 ]; then
      echo -n "* Add group centreon"
      usermod -aG centreon apache
      print_status $?
      RESTART_APACHE=1
    fi
  fi
  if [ ${RESTART_APACHE} -eq 1 ]; then
    service httpd restart
  fi
fi

echo "Add group to user centreon-engine"
id centreon-engine | grep "(nagios)" &>/dev/null
if [ $? -ne 0 ]; then
  if getent group nagios &>/dev/null; then
    echo -n "* Add group nagios"
    usermod -aG nagios centreon-engine
    print_status $?
  fi
fi
id centreon-engine | grep "(centreon)" &>/dev/null
if [ $? -ne 0 ]; then
  if getent group centreon &>/dev/null; then
    echo -n "* Add group centreon"
    usermod -aG centreon centreon-engine
    print_status $?
  fi
fi

if [ ${USE_CENTREON} -eq 1 ]; then
  echo -n "Create symlinks for nagios.cfg"
  if [ -f "/etc/centreon-engine/centengine.cfg" ] ; then
    mv /etc/centreon-engine/centengine.cfg /etc/centreon-engine/centengine.cfg.old
  fi
  ln -s /etc/centreon-engine/nagios.cfg /etc/centreon-engine/centengine.cfg
  print_status $?
fi

grep User_Alias /etc/sudoers | grep "centreon-engine" &>/dev/null
if [ $? -ne 0 ]; then
  echo "Add configuration for Centreon Engine in sudo"
  echo -n "* Add user alias"
  sed -i "s/^User_Alias\(.*\)$/User_Alias\1, centreon-engine/g" /etc/sudoers
  print_status $?

  echo -n "* Add commands"
  cat <<EOF >> /etc/sudoers

# Centreon Engine Restart
CENTREON   ALL = NOPASSWD: /etc/init.d/centengine restart
# Centreon Engine stop
CENTREON   ALL = NOPASSWD: /etc/init.d/centengine start
# Centreon Engine stop
CENTREON   ALL = NOPASSWD: /etc/init.d/centengine stop
# Centreon Engine reload
CENTREON   ALL = NOPASSWD: /etc/init.d/centengine reload
# Centreon Engine test config
CENTREON   ALL = NOPASSWD: /usr/sbin/centengine -v *
# Centreon Engine test for optim config
CENTREON   ALL = NOPASSWD: /usr/sbin/centengine -s *
EOF
  print_status $?
fi

