/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
** Copyright 2011      Merethis
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
*/

#include "globals.hh"

com::centreon::engine::configuration config;
char*                     config_file = NULL;

command*                  global_host_event_handler_ptr = NULL;
command*                  global_service_event_handler_ptr = NULL;

command*                  ocsp_command_ptr = NULL;
command*                  ochp_command_ptr = NULL;

unsigned long             logging_options = 0;
unsigned long             syslog_options = 0;

time_t                    last_command_check = 0L;
time_t                    last_command_status_update = 0L;
time_t                    last_log_rotation = 0L;

unsigned long             modified_host_process_attributes = MODATTR_NONE;
unsigned long             modified_service_process_attributes = MODATTR_NONE;

unsigned long             next_comment_id = 0L;
unsigned long             next_downtime_id = 0L;
unsigned long             next_event_id = 0L;
unsigned long             next_problem_id = 0L;
unsigned long             next_notification_id = 0L;

int                       sigshutdown = FALSE;
int                       sigrestart = FALSE;

char const*               sigs[35] = {
  "EXIT", "HUP", "INT", "QUIT", "ILL",
  "TRAP", "ABRT", "BUS", "FPE", "KILL",
  "USR1", "SEGV", "USR2", "PIPE", "ALRM",
  "TERM", "STKFLT", "CHLD", "CONT", "STOP",
  "TSTP", "TTIN", "TTOU", "URG", "XCPU",
  "XFSZ", "VTALRM", "PROF", "WINCH", "IO",
  "PWR", "UNUSED", "ZERR", "DEBUG", NULL
};

int                       caught_signal = FALSE;
int                       sig_id = 0;

int                       restarting = FALSE;

int                       verify_config = FALSE;
int                       verify_circular_paths = TRUE;
int                       test_scheduling = FALSE;
int                       precache_objects = FALSE;
int                       use_precached_objects = FALSE;

unsigned int              currently_running_service_checks = 0;
unsigned int              currently_running_host_checks = 0;

time_t                    program_start = 0L;
time_t                    event_start = 0L;
int                       nagios_pid = 0;

int                       embedded_perl_initialized = FALSE;

int                       command_file_fd = -1;
FILE*                     command_file_fp = NULL;
int                       command_file_created = FALSE;

host*                     host_list = NULL;
host*                     host_list_tail = NULL;
service*                  service_list = NULL;
service*                  service_list_tail = NULL;
contact*                  contact_list = NULL;
contact*                  contact_list_tail = NULL;
contactgroup*             contactgroup_list = NULL;
contactgroup*             contactgroup_list_tail = NULL;
hostgroup*                hostgroup_list = NULL;
hostgroup*                hostgroup_list_tail = NULL;
servicegroup*             servicegroup_list = NULL;
servicegroup*             servicegroup_list_tail = NULL;
command*                  command_list = NULL;
command*                  command_list_tail = NULL;
timeperiod*               timeperiod_list = NULL;
timeperiod*               timeperiod_list_tail = NULL;
serviceescalation*        serviceescalation_list = NULL;
serviceescalation*        serviceescalation_list_tail = NULL;
servicedependency*        servicedependency_list = NULL;
servicedependency*        servicedependency_list_tail = NULL;
hostdependency*           hostdependency_list = NULL;
hostdependency*           hostdependency_list_tail = NULL;
hostescalation*           hostescalation_list = NULL;
hostescalation*           hostescalation_list_tail = NULL;

skiplist*                 object_skiplists[NUM_OBJECT_SKIPLISTS];

int                       __nagios_object_structure_version = CURRENT_OBJECT_STRUCTURE_VERSION;

notification*             notification_list = NULL;

check_result              check_result_info;
check_result*             check_result_list = NULL;

dbuf                      check_result_dbuf;

circular_buffer           external_command_buffer;
circular_buffer           check_result_buffer;
pthread_t                 worker_threads[TOTAL_WORKER_THREADS];

check_stats               check_statistics[MAX_CHECK_STATS_TYPES];
