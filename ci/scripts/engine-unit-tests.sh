#!/bin/bash
set -e
DISTRIB=$(lsb_release -rs | cut -f1 -d.)

# echo "################################################## install CLIB ##################################################"
if [ $DISTRIB = "7" ] ; then
  curl http://yum-1.centreon.com/standard/21.10/el7/unstable/x86_64/clib/centreon-clib-21.10.0-1632908959.e89357c/centreon-clib-21.10.0-1632908959.e89357c.el7.centos.x86_64.rpm --output centreon-clib-21.10.0-1632908959.e89357c.el7.centos.x86_64.rpm
  curl http://yum-1.centreon.com/standard/21.10/el7/unstable/x86_64/clib/centreon-clib-21.10.0-1632908959.e89357c/centreon-clib-devel-21.10.0-1632908959.e89357c.el7.centos.x86_64.rpm --output centreon-clib-devel-21.10.0-1632908959.e89357c.el7.centos.x86_64.rpm
  yum -y install centreon-clib*.rpm
elif [ $DISTRIB = "8" ] ; then
  curl http://yum-1.centreon.com/standard/21.10/el8/unstable/x86_64/clib/clib-21.10.0-1632908959.e89357c/centreon-clib-21.10.0-1632908959.e89357c.el8.x86_64.rpm --output centreon-clib-21.10.0-1632908959.e89357c.el8.x86_64.rpm
  curl http://yum-1.centreon.com/standard/21.10/el8/unstable/x86_64/clib/clib-21.10.0-1632908959.e89357c/centreon-clib-devel-21.10.0-1632908959.e89357c.el8.x86_64.rpm --output centreon-clib-devel-21.10.0-1632908959.e89357c.el8.x86_64.rpm
  yum -y install centreon-clib*.rpm
fi
# elif [ "$(cat /etc/debian_version)" = "10.10" ] ; then

# fi

#Cmake
rm -rf /src/build
mkdir /src/build
cd /src/build/

if [ "$DISTRIB" = "7" ] ; then
    source /opt/rh/devtoolset-9/enable
fi 
conan install .. -s compiler.libcxx=libstdc++11 --build=missing
if [ "$(cat /etc/debian_version)" = "10.10" ] ; then
    CXXFLAGS="-Wall -Wextra" cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DWITH_CENTREON_CLIB_INCLUDE_DIR=../centreon-clib/inc/ -DWITH_CENTREON_CLIB_LIBRARIES=centreon-clib/libcentreon_clib.so -DCMAKE_BUILD_TYPE=Debug -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER_BROKER=centreon-broker -DWITH_USER_ENGINE=centreon-engine -DWITH_GROUP_BROKER=centreon-broker -DWITH_GROUP_ENGINE=centreon-engine -DWITH_TESTING=On -DWITH_PREFIX_MODULES=/usr/share/centreon/lib/centreon-broker -DWITH_PREFIX_CONF_BROKER=/etc/centreon-broker -DWITH_PREFIX_LIB_BROKER=/usr/lib64/nagios -DWITH_PREFIX_CONF_ENGINE=/etc/centreon-engine -DWITH_PREFIX_LIB_ENGINE=/usr/lib64/centreon-engine -DWITH_PREFIX_LIB_CLIB=/usr/lib64/ -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_MODULE_SIMU=On ..
else 
    CXXFLAGS="-Wall -Wextra" cmake3 -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DWITH_CENTREON_CLIB_INCLUDE_DIR=../centreon-clib/inc/ -DWITH_CENTREON_CLIB_LIBRARIES=centreon-clib/libcentreon_clib.so -DCMAKE_BUILD_TYPE=Debug -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER_BROKER=centreon-broker -DWITH_USER_ENGINE=centreon-engine -DWITH_GROUP_BROKER=centreon-broker -DWITH_GROUP_ENGINE=centreon-engine -DWITH_TESTING=On -DWITH_PREFIX_MODULES=/usr/share/centreon/lib/centreon-broker -DWITH_PREFIX_CONF_BROKER=/etc/centreon-broker -DWITH_PREFIX_LIB_BROKER=/usr/lib64/nagios -DWITH_PREFIX_CONF_ENGINE=/etc/centreon-engine -DWITH_PREFIX_LIB_ENGINE=/usr/lib64/centreon-engine -DWITH_PREFIX_LIB_CLIB=/usr/lib64/ -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_MODULE_SIMU=On ..
fi

#Build
make -j9 
make -j9 install

#Test

tests/ut_engine --gtest_output=xml:engine-ut.xml
 



