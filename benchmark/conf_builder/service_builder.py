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
from random import randint

service_id = 1
services = {}

ad = []


def create_service(host_name: str):
    global services
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
    services.setdefault(host_name, []).append(service_id)
    service_id += 1
    return retval


def create_anomalydetection(host_name: str, host_id: int):
    global services
    global service_id
    choices = services.setdefault(host_name, [])
    if len(choices) > 0:
        dep_id = choices[randint(0, len(services) - 1)]
    else:
        assert(1==0)

    retval = {
        'host_name': host_name,
        'service_description': "anomalydetection" + str(service_id),
        'dependent_service_id': dep_id,
        'metric_name': "metric",
        'notification_interval': 10,
        'notification_options': "c,r",
        'thresholds_file': "/etc/centreon-engine/anomaly/anomalydetetion" + str(service_id) + ".json",
        'status_change': 1,
        'notifications_enabled': 0,
        '_SERVICE_ID': service_id,
        '_HOST_ID': host_id
    }
    ad.append(service_id)
    service_id += 1
    return retval
