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
hostgroup_id = 1


def create_hostgroup():
    global hostgroup_id
    retval = {
        'hostgroup_id': hostgroup_id,
        'hostgroup_name': "central-hostgroup" + str(hostgroup_id),
        'alias': "central-hostgroup" + str(hostgroup_id),
        'members': [],
    }
    hostgroup_id += 1
    return retval
