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
service_id = 1


def create_service(host_name: str):
    global service_id
    retval = {
        'host_name': host_name,
        'service_description': "service" + str(service_id),
        'check_command': "App-Monitoring-Centreon-Service-Dummy",
        'max_check_attempts': 1,
        'check_period': "24x7",
        'check_interval': 1,
        'retry_interval': 1,
        'register': 1,
        'active_checks_enabled': 1,
        'passive_checks_enabled': 1,
        '_DUMMYSTATUS': 0,
        '_DUMMYOUTPUT': 'Check Dummy',
        '_SERVICE_ID': service_id,
    }
    service_id += 1
    return retval
