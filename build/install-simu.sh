#!/bin/bash

mkdir -p /tmp/etc/centreon-engine
mkdir -p /tmp/etc/centreon-broker
rsync -avp ../src/simumod/conf/ /tmp/etc/centreon-engine

mkdir -p /tmp/usr/lib64/centreon-engine
cp modules/external_commands/externalcmd.so /tmp/usr/lib64/centreon-engine

mkdir -p /tmp/usr/lib64/nagios
cp simumod/simumod.so /tmp/usr/lib64/nagios/

mkdir -p /tmp/var/log/centreon-engine
mkdir -p /tmp/var/lib/centreon-engine/rw

mkdir -p /tmp/usr/lib64/nagios/plugins
mkdir -p /tmp/usr/lib/centreon/plugins
cp ../src/simumod/plugins/check.sh /tmp/usr/lib/centreon/plugins
