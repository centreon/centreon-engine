#!/usr/bin/python3
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
import os
import sys
import json
from conf_builder import host_builder as hb
from conf_builder import service_builder as sb
from conf_builder import command_builder as cb
from conf_builder import file_builder as fb
from conf_builder import hostgroup_builder  as hgb


def main():
    conf_dir = "centreon-engine"

    hosts = [hb.create_template()]
    services = []
    hostgroups = []
    anomalydetections = []

    # Conf directory creation
    if not os.path.exists(conf_dir):
        os.mkdir(conf_dir)

    if len(sys.argv) != 2:
        print("Error: this script needs a json configuration file.")
        exit(1)

    json_file = open(sys.argv[1])
    conf = json.load(json_file)

    for hg in conf['hostgroups']:
        for i in range(hg['count']):
            new_hostgroup = hgb.create_hostgroup()
            hostgroups.append(new_hostgroup)

        assert type(hg['hosts']) is list
        for h in hg['hosts']:
            for i in range(h['count']):
                new_host = hb.create_host()
                hosts.append(new_host)
                for j in range(h['services']):
                    services.append(sb.create_service(new_host['host_name']))
                for j in range(h['anomalydetections']):
                    anomalydetections.append(sb.create_anomalydetection(new_host['host_name'], new_host['_HOST_ID']))
                new_hostgroup['members'].append(new_host['host_name'])
    commands = cb.create_templates()
    fb.save_hosts(hosts)
    fb.save_services(services)
    fb.save_anomalydetections(anomalydetections)
    fb.save_commands(commands)
    fb.save_hostgroups(hostgroups)
    fb.save_various()
    fb.save_engine()


if __name__ == "__main__":
    # execute only if run as a script
    main()
