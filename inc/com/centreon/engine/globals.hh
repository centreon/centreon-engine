/*
** Copyright 1999-2009      Ethan Galstad
** Copyright 2009-2010      Nagios Core Development Team and Community Contributors
** Copyright 2011-2013,2015 Merethis
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
#  include "com/centreon/engine/circular_buffer.hh"
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/events/hash_timed_event.hh"
#  include "com/centreon/engine/events/sched_info.hh"
#  include "com/centreon/engine/events/timed_event.hh"
#  include "com/centreon/engine/nebmods.hh"
#  include "com/centreon/engine/notifications.hh"
#  include "com/centreon/engine/objects.hh"
#  include "com/centreon/engine/utils.hh"
#  include "skiplist.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */


extern int                       config_errors;
extern int                       config_warnings;

extern unsigned long             max_check_result_file_age;
extern char*                     check_result_path;

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
extern time_t                    last_log_rotation;

extern unsigned long             modified_host_process_attributes;
extern unsigned long             modified_service_process_attributes;

extern unsigned long             next_comment_id;
extern unsigned long             next_downtime_id;
extern unsigned long             next_event_id;
extern unsigned long             next_problem_id;
extern unsigned long             next_notification_id;

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
extern int                       precache_objects;
extern int                       use_precached_objects;

extern unsigned int              currently_running_service_checks;
extern unsigned int              currently_running_host_checks;

extern time_t                    program_start;
extern time_t                    event_start;

extern int                       embedded_perl_initialized;

extern host*                     host_list;
extern host*                     host_list_tail;
extern std::map<std::string, host_other_properties> host_other_props;
extern service*                  service_list;
extern service*                  service_list_tail;
extern std::map<std::pair<std::string, std::string>, service_other_properties> service_other_props;
extern contact*                  contact_list;
extern contact*                  contact_list_tail;
extern std::map<std::string, contact_other_properties> contact_other_props;
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
extern pthread_t                 worker_threads[];

extern check_stats               check_statistics[];

extern timed_event*              event_list_low;
extern timed_event*              event_list_low_tail;
extern timed_event*              event_list_high;
extern timed_event*              event_list_high_tail;
extern sched_info                scheduling_info;

extern comment*                  comment_list;
extern int                       defer_comment_sorting;

extern char*                     macro_x_names[];
extern char*                     macro_user[];

extern scheduled_downtime*       scheduled_downtime_list;
extern int                       defer_downtime_sorting;

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
extern unsigned int log_notifications;
extern unsigned int log_service_retries;
extern unsigned int log_event_handlers;
extern unsigned int log_external_commands;
extern unsigned int log_passive_checks;
extern unsigned int log_initial_states;
extern int log_host_retries;
extern int allow_empty_hostgroup_assignment;
extern unsigned int retain_state_information;
extern unsigned int retention_update_interval;
extern unsigned int use_retained_program_state;
extern unsigned int use_retained_scheduling_info;
extern unsigned int retention_scheduling_horizon;
extern int additional_freshness_latency;
extern unsigned long retained_host_attribute_mask;
/* retained_service_attribute_mask; */
extern unsigned long retained_process_host_attribute_mask;
/* retained_process_service_attribute_mask; */
extern unsigned long retained_contact_service_attribute_mask;
extern unsigned long retained_contact_host_attribute_mask;
extern unsigned int obsess_over_services;
extern unsigned int obsess_over_hosts;
extern unsigned int translate_passive_host_checks;
extern unsigned int passive_host_checks_are_soft;
extern unsigned int service_check_timeout;
extern unsigned int host_check_timeout;
extern unsigned int event_handler_timeout;
extern unsigned int notification_timeout;
extern unsigned int ocsp_timeout;
extern unsigned int ochp_timeout;
extern unsigned int use_aggressive_host_checking;
extern unsigned long cached_host_check_horizon;
extern unsigned int enable_predictive_host_dependency_checks;
extern unsigned long cached_service_check_horizon;
extern unsigned int enable_predictive_service_dependency_checks;
extern unsigned int soft_state_dependencies;
extern unsigned int enable_event_handlers;
extern unsigned int enable_notifications;
extern unsigned int execute_service_checks;
extern unsigned int accept_passive_service_checks;
extern unsigned int execute_host_checks;
extern unsigned int accept_passive_host_checks;
extern unsigned int service_inter_check_delay_method;
extern unsigned int max_service_check_spread;
extern unsigned int host_inter_check_delay_method;
extern unsigned int max_host_check_spread;
extern unsigned int service_interleave_factor_method;
extern unsigned int max_parallel_service_checks;
extern unsigned int check_reaper_interval;
extern unsigned int max_check_reaper_time;
extern float sleep_time;
extern unsigned int interval_length;
extern unsigned int check_external_commands;
/* command_check_interval_is_seconds; */
extern int command_check_interval;
extern unsigned int check_orphaned_services;
extern unsigned int check_orphaned_hosts;
extern unsigned int check_service_freshness;
extern unsigned int check_host_freshness;
extern unsigned int service_freshness_check_interval;
extern unsigned int host_freshness_check_interval;
extern unsigned int auto_reschedule_checks;
extern unsigned int auto_rescheduling_interval;
extern unsigned int auto_rescheduling_window;
extern unsigned int status_update_interval;
extern unsigned int time_change_threshold;
extern unsigned int process_performance_data;
extern unsigned int enable_flap_detection;
extern unsigned int enable_failure_prediction;
extern float low_service_flap_threshold;
extern float high_service_flap_threshold;
extern float low_host_flap_threshold;
extern float high_host_flap_threshold;
extern unsigned int date_format;
extern char* use_timezone;
extern unsigned long event_broker_options;
extern char* illegal_object_chars;
extern char* illegal_output_chars;
extern unsigned int use_regexp_matches;
extern unsigned int use_true_regexp_matching;
extern unsigned int use_large_installation_tweaks;
extern unsigned int enable_environment_macros;
extern int external_command_buffer_slots;
/* auth_file; */

#  ifdef __cplusplus
}
#  endif /* C++ */

#endif /* !CCE_GLOBALS_HH */
