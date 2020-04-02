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

import fileinput
import datetime
import grpc
import enginerpc.engine_pb2
import enginerpc.engine_pb2_grpc

channel = grpc.insecure_channel("127.0.0.1:50126")
stub = enginerpc.engine_pb2_grpc.EngineStub(channel)
stats = stub.GetStats(enginerpc.engine_pb2.Stats())
print(stats)

exit(0)

event = ''
service_processed = False
host_id = 0
service_id = 0
start_time = 0
end_time = 0
host = {}

for line in fileinput.input():
    if line.startswith("nebstruct_"):
        if service_processed:
            service_processed = False
            if host_id not in host:
                host[host_id] = {}
            if service_id not in host[host_id]:
                host[host_id][service_id] = {
                    'start_time': [],
                    'end_time': []
                }
            host[host_id][service_id]['start_time'].append(start_time)
            host[host_id][service_id]['end_time'].append(start_time)
        event = line[10:-3]
    else:
        if service_processed:
            if line.startswith('  start_time='):
                print("start_time", line[13:-1])
                start_time = datetime.datetime.fromisoformat(line[13:-1])
            elif line.startswith('  end_time='):
                print("end_time", line[11:-1])
                end_time = datetime.datetime.fromisoformat(line[11:-1])
            elif line.startswith('  host_name='):
                host_id = int(line[19:-1])
                print("host ", line, " gives ", host_id)
            elif line.startswith('  service_description='):
                service_id = int(line[29:-1])
                print("service ", line, " gives ", service_id)
        elif event == 'service_check_data':
            if line.startswith('  type=701'):
                service_processed = True
            else:
                service_processed = False
