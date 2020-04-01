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

import os

root_dir = os.getcwd()


def _save(name: str, define_cmd: str, lst: list):
    f = open(name, "w")
    for c in lst:
        f.write("define %s {\n" % define_cmd)
        for k, v in c.items():
            if type(v) is list:
                f.write("    %s    %s\n" % (k, ','.join(v)))
            else:
                f.write("    %s    %s\n" % (k, v))
        f.write("}\n\n")
    f.close()


def save_commands(commands: list):
    _save("centreon-engine/commands.cfg", "command", commands)


def save_services(services: list):
    _save("centreon-engine/services.cfg", "service", services)


def save_hosts(hosts: list):
    _save("centreon-engine/hosts.cfg", "host", hosts)


def save_hostgroups(hostgroups: list):
    _save("centreon-engine/hostgroups.cfg", "hostgroup", hostgroups)


def save_various():
    f = open("centreon-engine/timeperiods.cfg", "w")
    f.write("""define timeperiod {
    name                           24x7
    timeperiod_name                24x7
    alias                          24_Hours_A_Day,_7_Days_A_Week
    sunday                         00:00-24:00
    monday                         00:00-24:00
    tuesday                        00:00-24:00
    wednesday                      00:00-24:00
    thursday                       00:00-24:00
    friday                         00:00-24:00
    saturday                       00:00-24:00
}""")
    f.close()

    f = open("centreon-engine/resource.cfg", "w")
    f.write(f"$USER1$={root_dir}/plugins\n")
    f.write(f"$CENTREONPLUGINS$={root_dir}/plugins\n")
    f.close()


def save_engine():
    global root_dir

    conf_dir = root_dir + "/centreon-engine"
    conf_broker_dir = root_dir + "/centreon-broker"
    log_dir = root_dir + "/log"
    lib_dir = root_dir + "/lib"
    bin_dir = root_dir + "/build"
    f = open("centreon-engine/centengine.cfg", "w")
    f.write(f"""# Centengine configuration
cfg_file={conf_dir}/hosts.cfg
cfg_file={conf_dir}/services.cfg
cfg_file={conf_dir}/commands.cfg
cfg_file={conf_dir}/timeperiods.cfg
broker_module={bin_dir}/modules/external_commands/externalcmd.so
broker_module={bin_dir}/src/simumod/simumod.so %s/central-module.xml
interval_length=60
use_timezone=:Europe/Paris
resource_file={conf_dir}/resource.cfg
log_file={log_dir}/centengine.log
state_retention_file={log_dir}/retention.dat
status_file={log_dir}/status.dat
command_check_interval=1s
command_file={lib_dir}/rw/centengine.cmd
retention_update_interval=60
sleep_time=0.2
service_inter_check_delay_method=s
service_interleave_factor=s
max_concurrent_checks=400
max_service_check_spread=5
check_result_reaper_frequency=5
low_service_flap_threshold=25.0
high_service_flap_threshold=50.0
low_host_flap_threshold=25.0
high_host_flap_threshold=50.0
service_check_timeout=60
host_check_timeout=12
event_handler_timeout=30
notification_timeout=30
ocsp_timeout=5
ochp_timeout=5
perfdata_timeout=5
date_format=euro
illegal_object_name_chars=~!$%^&*"|'<>?,()=
illegal_macro_output_chars=`~$^&"|'<>
admin_email=admin@localhost
admin_pager=admin
event_broker_options=-1
cached_host_check_horizon=60
debug_file={log_dir}/centengine.debug
debug_level=0
debug_verbosity=2
log_pid=1
enable_macros_filter=0
enable_notifications=1
execute_service_checks=1
accept_passive_service_checks=1
enable_event_handlers=1
check_external_commands=1
use_retained_program_state=1
use_retained_scheduling_info=1
use_syslog=0
log_notifications=1
log_service_retries=1
log_host_retries=1
log_event_handlers=1
log_external_commands=1
use_aggressive_host_checking=1
soft_state_dependencies=0
obsess_over_services=0
process_performance_data=0
check_for_orphaned_services=0
check_for_orphaned_hosts=0
check_service_freshness=1
enable_flap_detection=0""")
    f.close()
