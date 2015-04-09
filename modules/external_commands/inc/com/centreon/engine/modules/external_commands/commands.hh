/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_MOD_EXTCMD_COMMANDS_HH
#  define CCE_MOD_EXTCMD_COMMANDS_HH

#  include <time.h>
#  include "com/centreon/engine/objects.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// External command functions
int check_for_external_commands();                                          // checks for any external commands
int process_external_commands_from_file(char const* file, int delete_file); // process external commands in a file
int process_external_command(char const* cmd);                              // external command processor

// External command implementations
int cmd_add_comment(int cmd,time_t entry_time,char* args);                  // add a service or host comment
int cmd_delete_comment(int cmd,char* args);                                 // delete a service or host comment
int cmd_delete_all_comments(int cmd,char* args);                            // delete all comments associated with a host or service
int cmd_schedule_check(int cmd,char* args);                                 // schedule an immediate or delayed host check
int cmd_schedule_host_service_checks(int cmd,char* args, int force);        // schedule an immediate or delayed checks of all services on a host
void cmd_signal_process(int cmd, char* args);                               // schedules a program shutdown or restart
int cmd_process_service_check_result(int cmd,time_t check_time,char* args); // processes a passive service check
int process_passive_service_check(time_t check_time, char const* host_name, char const* svc_description, int return_code, char const* output);
int cmd_process_host_check_result(int cmd,time_t check_time,char* args);    // processes a passive host check
int process_passive_host_check(time_t check_time, char const* host_name, int return_code, char const* output);
int cmd_change_object_int_var(int cmd,char* args);                          // changes host/svc (int) variable
int cmd_change_object_char_var(int cmd,char* args);                         // changes host/svc (char) variable
int cmd_change_object_custom_var(int cmd, char* args);                      // changes host/svc custom variable
int cmd_process_external_commands_from_file(int cmd, char* args);           // process external commands from a file
void disable_service_checks(service* svc);                                  // disables a service check
void enable_service_checks(service* svc);                                   // enables a service check
void start_using_event_handlers(void);                                     // enables event handlers on a program-wide basis
void stop_using_event_handlers(void);                                      // disables event handlers on a program-wide basis
void enable_service_event_handler(service* svc);                           // enables the event handler for a particular service
void disable_service_event_handler(service* svc);                          // disables the event handler for a particular service
void enable_host_event_handler(host* hst);                                 // enables the event handler for a particular host
void disable_host_event_handler(host* hst);                                // disables the event handler for a particular host
void disable_host_checks(host* hst);                                       // disables checks of a particular host
void enable_host_checks(host* hst);                                        // enables checks of a particular host
void start_obsessing_over_service_checks(void);                            // start obsessing about service check results
void stop_obsessing_over_service_checks(void);                             // stop obsessing about service check results
void start_obsessing_over_host_checks(void);                               // start obsessing about host check results
void stop_obsessing_over_host_checks(void);                                // stop obsessing about host check results
void enable_service_freshness_checks(void);                                // enable service freshness checks
void disable_service_freshness_checks(void);                               // disable service freshness checks
void enable_host_freshness_checks(void);                                   // enable host freshness checks
void disable_host_freshness_checks(void);                                  // disable host freshness checks
void start_obsessing_over_service(service* svc);                           // start obsessing about specific service check results
void stop_obsessing_over_service(service* svc);                            // stop obsessing about specific service check results
void start_obsessing_over_host(host* hst);                                 // start obsessing about specific host check results
void stop_obsessing_over_host(host* hst);                                  // stop obsessing about specific host check results

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_MOD_EXTCMD_COMMANDS_HH
