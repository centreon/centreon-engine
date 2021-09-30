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
    CXXFLAGS="-Wall -Wextra" cmake3 -DWITH_PREFIX=/usr -DWITH_CENTREON_CLIB_LIBRARY_DIR=/usr/lib -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Debug -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On -DWITH_CREATE_FILES=OFF -DWITH_BENCH=On ..
else 
    CXXFLAGS="-Wall -Wextra" cmake3 -DWITH_PREFIX=/usr -DWITH_CENTREON_CLIB_LIBRARY_DIR=/usr/lib64 -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Debug -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On -DWITH_CREATE_FILES=OFF -DWITH_BENCH=On ..
fi

#Build
make -j9 
make -j9 install

#Test

tests/ut_engine --gtest_output=xml:engine-ut.xml
 



