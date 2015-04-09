/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
** Copyright 2011-2015 Merethis
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

#ifndef CCE_GLOBALS_HH
#  define CCE_GLOBALS_HH

#  include <map>
#  include <string>
#  include <stdio.h>
#  include "com/centreon/engine/checks.hh"
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/events/hash_timed_event.hh"
#  include "com/centreon/engine/events/sched_info.hh"
#  include "com/centreon/engine/events/timed_event.hh"
#  include "com/centreon/engine/nebmods.hh"
#  include "com/centreon/engine/objects.hh"
#  include "com/centreon/engine/utils.hh"

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */


extern int                       config_errors;
extern int                       config_warnings;

extern com::centreon::engine::configuration::state* config;
extern char*                     config_file;

extern command*                  global_host_event_handler_ptr;
extern command*                  global_service_event_handler_ptr;

extern command*                  ocsp_command_ptr;
extern command*                  ochp_command_ptr;

extern unsigned long             logging_options;
extern unsigned long             syslog_options;

extern com::centreon::engine::events::hash_timed_event quick_timed_event;

extern time_t                    last_command_check;
extern time_t                    last_command_status_update;

extern unsigned long             modified_host_process_attributes;
extern unsigned long             modified_service_process_attributes;

extern unsigned long             next_event_id;
extern unsigned long             next_problem_id;

extern int                       sighup;
extern int                       sigshutdown;
extern int                       sigrestart;

extern char const*               sigs[35];

extern int                       caught_signal;
extern int                       sig_id;

extern int                       restarting;

extern int                       verify_config;
extern int                       verify_circular_paths;
extern int                       test_scheduling;

extern unsigned int              currently_running_service_checks;
extern unsigned int              currently_running_host_checks;

extern time_t                    program_start;
extern time_t                    event_start;

extern host*                     host_list;
extern host*                     host_list_tail;
extern std::map<std::string, host_other_properties> host_other_props;
extern service*                  service_list;
extern service*                  service_list_tail;
extern hostgroup*                hostgroup_list;
extern hostgroup*                hostgroup_list_tail;
extern servicegroup*             servicegroup_list;
extern servicegroup*             servicegroup_list_tail;
extern command*                  command_list;
extern command*                  command_list_tail;
extern timeperiod*               timeperiod_list;
extern timeperiod*               timeperiod_list_tail;
extern servicedependency*        servicedependency_list;
extern servicedependency*        servicedependency_list_tail;
extern hostdependency*           hostdependency_list;
extern hostdependency*           hostdependency_list_tail;

extern int                       __nagios_object_structure_version;

extern check_stats               check_statistics[];

extern timed_event*              event_list_low;
extern timed_event*              event_list_low_tail;
extern timed_event*              event_list_high;
extern timed_event*              event_list_high_tail;
extern sched_info                scheduling_info;

extern char*                     macro_x_names[];
extern char*                     macro_user[];

extern FILE*                     debug_file_fp;

extern char**                    macro_x;

extern nebcallback*              neb_callback_list[];

extern char* log_file;
extern unsigned int debug_level;
extern unsigned int debug_verbosity;
extern char* debug_file;
extern unsigned long max_debug_file_size;
extern char* command_file;
extern char* global_host_event_handler;
extern char* global_service_event_handler;
extern char* ocsp_command;
extern char* ochp_command;
extern unsigned int use_syslog;
extern unsigned int log_service_retries;
extern unsigned int log_event_handlers;
extern unsigned int log_external_commands;
extern unsigned int log_passive_checks;
extern unsigned int log_initial_states;
extern int log_host_retries;
extern unsigned int retention_update_interval;
extern int additional_freshness_latency;
extern unsigned int obsess_over_services;
extern unsigned int obsess_over_hosts;
extern unsigned int passive_host_checks_are_soft;
extern unsigned int service_check_timeout;
extern unsigned int host_check_timeout;
extern unsigned int event_handler_timeout;
extern unsigned int ocsp_timeout;
extern unsigned int ochp_timeout;
extern unsigned long cached_host_check_horizon;
extern unsigned int enable_predictive_host_dependency_checks;
extern unsigned long cached_service_check_horizon;
extern unsigned int enable_predictive_service_dependency_checks;
extern unsigned int soft_state_dependencies;
extern unsigned int enable_event_handlers;
extern unsigned int execute_service_checks;
extern unsigned int execute_host_checks;
extern unsigned int max_parallel_service_checks;
extern unsigned int check_reaper_interval;
extern float sleep_time;
extern unsigned int interval_length;
/* command_check_interval_is_seconds; */
extern int command_check_interval;
extern unsigned int check_service_freshness;
extern unsigned int check_host_freshness;
extern unsigned int service_freshness_check_interval;
extern unsigned int host_freshness_check_interval;
extern unsigned int time_change_threshold;
extern unsigned int enable_flap_detection;
extern float low_service_flap_threshold;
extern float high_service_flap_threshold;
extern float low_host_flap_threshold;
extern float high_host_flap_threshold;
extern char* use_timezone;
extern unsigned long event_broker_options;
extern char* illegal_object_chars;
extern char* illegal_output_chars;
extern int external_command_buffer_slots;

#  ifdef __cplusplus
}
#  endif /* C++ */

#endif /* !CCE_GLOBALS_HH */
