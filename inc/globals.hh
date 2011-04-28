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

#ifndef CCE_GLOBALS_HH_
# define CCE_GLOBALS_HH_

# include <stdio.h>
# include "checks.hh"
# include "configuration.hh"
# include "engine.hh"
# include "notifications.hh"
# include "objects.hh"
# include "skiplist.hh"
# include "utils.hh"

extern com::centreon::engine::configuration config;
extern char*                     config_file;

extern command*                  global_host_event_handler_ptr;
extern command*                  global_service_event_handler_ptr;

extern command*                  ocsp_command_ptr;
extern command*                  ochp_command_ptr;

extern unsigned long             logging_options;
extern unsigned long             syslog_options;

extern time_t                    last_command_check;
extern time_t                    last_command_status_update;
extern time_t                    last_log_rotation;

extern unsigned long             modified_host_process_attributes;
extern unsigned long             modified_service_process_attributes;

extern unsigned long             next_comment_id;
extern unsigned long             next_downtime_id;
extern unsigned long             next_event_id;
extern unsigned long             next_problem_id;
extern unsigned long             next_notification_id;

extern int                       sigshutdown;
extern int                       sigrestart;

extern char const*               sigs[35];

extern int                       caught_signal;
extern int                       sig_id;

extern int                       restarting;

extern int                       verify_config;
extern int                       verify_circular_paths;
extern int                       test_scheduling;
extern int                       precache_objects;
extern int                       use_precached_objects;

extern unsigned int              currently_running_service_checks;
extern unsigned int              currently_running_host_checks;

extern time_t                    program_start;
extern time_t                    event_start;
extern int                       nagios_pid;

extern int                       embedded_perl_initialized;

extern int                       command_file_fd;
extern FILE*                     command_file_fp;
extern int                       command_file_created;

extern host*                     host_list;
extern host*                     host_list_tail;
extern service*                  service_list;
extern service*                  service_list_tail;
extern contact*                  contact_list;
extern contact*                  contact_list_tail;
extern contactgroup*             contactgroup_list;
extern contactgroup*             contactgroup_list_tail;
extern hostgroup*                hostgroup_list;
extern hostgroup*                hostgroup_list_tail;
extern servicegroup*             servicegroup_list;
extern servicegroup*             servicegroup_list_tail;
extern command*                  command_list;
extern command*                  command_list_tail;
extern timeperiod*               timeperiod_list;
extern timeperiod*               timeperiod_list_tail;
extern serviceescalation*        serviceescalation_list;
extern serviceescalation*        serviceescalation_list_tail;
extern servicedependency*        servicedependency_list;
extern servicedependency*        servicedependency_list_tail;
extern hostdependency*           hostdependency_list;
extern hostdependency*           hostdependency_list_tail;
extern hostescalation*           hostescalation_list;
extern hostescalation*           hostescalation_list_tail;

extern skiplist*                 object_skiplists[];

extern int                       __nagios_object_structure_version;

extern notification*             notification_list;

extern check_result              check_result_info;
extern check_result*             check_result_list;

extern dbuf                      check_result_dbuf;

extern circular_buffer           external_command_buffer;
extern circular_buffer           check_result_buffer;
extern pthread_t                 worker_threads[];

extern check_stats               check_statistics[];

#endif /* !CCE_GLOBALS_HH_ */
