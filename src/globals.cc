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

com::centreon::engine::configuration::state config;
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

int                       embedded_perl_initialized = FALSE;

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

timed_event*              event_list_low = NULL;
timed_event*              event_list_low_tail = NULL;
timed_event*              event_list_high = NULL;
timed_event*              event_list_high_tail = NULL;
sched_info                scheduling_info;

comment*                  comment_list = NULL;
int                       defer_comment_sorting = 0;

char*                     macro_x_names[MACRO_X_COUNT];     /* the macro names */
char*                     macro_user[MAX_USER_MACROS];      /* $USERx$ macros */

scheduled_downtime*       scheduled_downtime_list = NULL;
int                       defer_downtime_sorting = 0;

FILE*                     debug_file_fp = NULL;

/*
 * These point to their corresponding pointer arrays in global_macros
 * AFTER macros have been initialized.
 *
 * They really only exist so that eventbroker modules that reference
 * them won't need to be re-compiled, although modules that rely
 * on their values after having run a certain command will require an
 * update
 */
char**                    macro_x = NULL;

nebcallback*              neb_callback_list[NEBCALLBACK_NUMITEMS];

char* log_file = NULL;
unsigned int debug_level;
unsigned int debug_verbosity;
char* debug_file = NULL;
unsigned long max_debug_file_size;
char* command_file = NULL;
char* temp_path = NULL;
char* check_result_path = NULL;
char* global_host_event_handler = NULL;
char* global_service_event_handler = NULL;
char* ocsp_command = NULL;
char* ochp_command = NULL;
unsigned int use_syslog;
unsigned int log_notifications;
unsigned int log_service_retries;
unsigned int log_event_handlers;
unsigned int log_external_commands;
unsigned int log_passive_checks;
unsigned int log_initial_states;
int log_host_retries;
int allow_empty_hostgroup_assignment;
unsigned int retain_state_information;
unsigned int retention_update_interval;
unsigned int use_retained_program_state;
unsigned int use_retained_scheduling_info;
unsigned int retention_scheduling_horizon;
int additional_freshness_latency;
unsigned long retained_host_attribute_mask;
unsigned long retained_process_host_attribute_mask;
unsigned long retained_contact_service_attribute_mask;
unsigned long max_check_result_file_age;
unsigned long retained_contact_host_attribute_mask;
unsigned int obsess_over_services;
unsigned int obsess_over_hosts;
unsigned int translate_passive_host_checks;
unsigned int passive_host_checks_are_soft;
unsigned int service_check_timeout;
unsigned int host_check_timeout;
unsigned int event_handler_timeout;
unsigned int notification_timeout;
unsigned int ocsp_timeout;
unsigned int ochp_timeout;
unsigned int use_aggressive_host_checking;
unsigned long cached_host_check_horizon;
unsigned int enable_predictive_host_dependency_checks;
unsigned long cached_service_check_horizon;
unsigned int enable_predictive_service_dependency_checks;
unsigned int soft_state_dependencies;
unsigned int log_rotation_method;
char* log_archive_path = NULL;
unsigned int enable_event_handlers;
unsigned int enable_notifications;
unsigned int execute_service_checks;
unsigned int accept_passive_service_checks;
unsigned int execute_host_checks;
unsigned int accept_passive_host_checks;
unsigned int service_inter_check_delay_method;
unsigned int max_service_check_spread;
unsigned int host_inter_check_delay_method;
unsigned int max_host_check_spread;
unsigned int service_interleave_factor_method;
unsigned int max_parallel_service_checks;
unsigned int check_reaper_interval;
unsigned int max_check_reaper_time;
float sleep_time;
unsigned int interval_length;
unsigned int check_external_commands;
// command_check_interval_is_seconds;
int command_check_interval;
unsigned int check_orphaned_services;
unsigned int check_orphaned_hosts;
unsigned int check_service_freshness;
unsigned int check_host_freshness;
unsigned int service_freshness_check_interval;
unsigned int host_freshness_check_interval;
unsigned int auto_reschedule_checks;
unsigned int auto_rescheduling_interval;
unsigned int auto_rescheduling_window;
unsigned int status_update_interval;
unsigned int time_change_threshold;
unsigned int process_performance_data;
unsigned int enable_flap_detection;
unsigned int enable_failure_prediction;
float low_service_flap_threshold;
float high_service_flap_threshold;
float low_host_flap_threshold;
float high_host_flap_threshold;
unsigned int date_format;
char* use_timezone = NULL;
char* p1_file = NULL;
unsigned long event_broker_options;
char* illegal_object_chars = NULL;
char* illegal_output_chars = NULL;
unsigned int use_regexp_matches;
unsigned int use_true_regexp_matching;
unsigned int use_large_installation_tweaks;
unsigned int enable_environment_macros;
unsigned int free_child_process_memory;
unsigned int child_processes_fork_twice;
unsigned int enable_embedded_perl;
unsigned int use_embedded_perl_implicitly;
int external_command_buffer_slots;
// auth_file;
