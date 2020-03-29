#!/usr/bin/python3.7
"""
** Copyright 2020 Centreon
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
"""
host_id = 1
ip = [20, 0, 0, 1]


def new_address():
    global ip
    retval = '.'.join(map(str, ip))
    for i in range(3, 0, -1):
        if ip[i] < 254:
            ip[i] += 1
            break
        ip[i] = 1
    return retval


def create_template():
    retval = {
        'name': "generic-dummy-host",
        'alias': "generic-dummy-host",
        'address': "App-Monitoring-Centreon-Host-Dummy",
        'check_period': '24x7',
        'max_check_attempts': 1,
        'check_interval': 15,
        'register': 0,
        'active_checks_enabled': 1,
        'passive_checks_enabled': 0,
        '_DUMMYSTATUS': 0,
        '_DUMMYOUTPUT': 'This is a dummy check',
    }
    return retval

def create_host():
    global host_id
    retval = {
        'host_name': "central" + str(host_id),
        'alias': "central" + str(host_id),
        'address': new_address(),
        'check_command': "App-Monitoring-Centreon-Service-Dummy",
        'check_period': '24x7',
        'check_interval': 1,
        'retry_interval': 1,
        'register': 1,
        'active_checks_enabled': 1,
        'passive_checks_enabled': 1,
        'use': 'generic-dummy-host',
        '_SNMPCOMMUNITY': 'public',
        '_SNMPVERSION': '2c',
        '_HOST_ID': host_id,
    }
    host_id += 1
    return retval
