/*
** Copyright 1999-2009           Ethan Galstad
** Copyright 2009-2010           Nagios Core Development Team and Community
*Contributors
** Copyright 2011-2013,2016-2017 Centreon
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

#include "com/centreon/engine/globals.hh"
#include <map>
#include "com/centreon/engine/logging/logger.hh"
#include "nagios.h"

using namespace com::centreon::engine;

configuration::state* config(NULL);

char const* sigs[] = {"EXIT", "HUP",    "INT",    "QUIT",  "ILL",    "TRAP",
                      "ABRT", "BUS",    "FPE",    "KILL",  "USR1",   "SEGV",
                      "USR2", "PIPE",   "ALRM",   "TERM",  "STKFLT", "CHLD",
                      "CONT", "STOP",   "TSTP",   "TTIN",  "TTOU",   "URG",
                      "XCPU", "XFSZ",   "VTALRM", "PROF",  "WINCH",  "IO",
                      "PWR",  "UNUSED", "ZERR",   "DEBUG", NULL};

char* check_result_path(NULL);
char* config_file(NULL);
char* debug_file(NULL);
char* global_host_event_handler(NULL);
char* global_service_event_handler(NULL);
char* illegal_object_chars(NULL);
char* illegal_output_chars(NULL);
char* log_file(NULL);
std::string macro_user[MAX_USER_MACROS];   // $USERx$ macros
std::string macro_x_names[MACRO_X_COUNT];  // The macro names
char* ochp_command(NULL);
char* ocsp_command(NULL);
char* use_timezone(NULL);
check_stats check_statistics[MAX_CHECK_STATS_TYPES];
circular_buffer external_command_buffer;
com::centreon::engine::commands::command* global_host_event_handler_ptr(NULL);
com::centreon::engine::commands::command* global_service_event_handler_ptr(
    NULL);
com::centreon::engine::commands::command* ochp_command_ptr(NULL);
com::centreon::engine::commands::command* ocsp_command_ptr(NULL);
int additional_freshness_latency(15);
int config_errors(0);
int config_warnings(0);
int sig_id(0);
bool sighup{false};
int sigrestart(false);
int sigshutdown(false);
int test_scheduling(false);
int verify_circular_paths(true);
int verify_config(false);
nebcallback* neb_callback_list[NEBCALLBACK_NUMITEMS];
pthread_t worker_threads[TOTAL_WORKER_THREADS];
sched_info scheduling_info;
time_t event_start((time_t)-1);
time_t last_command_check((time_t)-1);
time_t last_command_status_update((time_t)-1);
time_t last_log_rotation((time_t)-1);
time_t program_start((time_t)-1);
unsigned int accept_passive_host_checks(true);
unsigned int accept_passive_service_checks(true);
unsigned int check_external_commands(true);
unsigned int check_host_freshness(false);
unsigned int check_reaper_interval(10);
unsigned int check_service_freshness(true);
unsigned int currently_running_host_checks(0);
unsigned int currently_running_service_checks(0);
unsigned int enable_event_handlers(true);
unsigned int enable_flap_detection(false);
unsigned int enable_notifications(true);
unsigned int execute_host_checks(true);
unsigned int execute_service_checks(true);
unsigned int interval_length(60);
unsigned int log_notifications(true);
unsigned int log_passive_checks(true);
unsigned int max_host_check_spread(5);
unsigned int max_service_check_spread(5);
unsigned int notification_timeout(30);
unsigned int obsess_over_hosts(false);
unsigned int obsess_over_services(false);
unsigned int process_performance_data(false);
unsigned int soft_state_dependencies(false);
unsigned int use_aggressive_host_checking(false);
unsigned int use_large_installation_tweaks(false);
unsigned long cached_host_check_horizon(15);
unsigned long logging_options(
    logging::log_runtime_error | logging::log_runtime_warning |
    logging::log_verification_error | logging::log_verification_warning |
    logging::log_config_error | logging::log_config_warning |
    logging::log_process_info | logging::log_host_notification |
    logging::log_service_notification | logging::log_event_handler |
    logging::log_external_command | logging::log_passive_check |
    logging::log_host_up | logging::log_host_down |
    logging::log_host_unreachable | logging::log_service_ok |
    logging::log_service_warning | logging::log_service_unknown |
    logging::log_service_critical | logging::log_info_message);
unsigned long modified_host_process_attributes(MODATTR_NONE);
unsigned long modified_service_process_attributes(MODATTR_NONE);
unsigned long next_event_id(1);
unsigned long next_notification_id(1);
unsigned long next_problem_id(0L);
