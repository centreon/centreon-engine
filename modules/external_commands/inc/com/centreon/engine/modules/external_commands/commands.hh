/*
** Copyright 2002-2006           Ethan Galstad
** Copyright 2011-2013,2015-2016 Centreon
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
int cmd_delay_notification(int cmd,char* args);                             // delay a service or host notification
int cmd_schedule_check(int cmd,char* args);                                 // schedule an immediate or delayed host check
void cmd_signal_process(int cmd, char* args);                               // schedules a program shutdown or restart
int cmd_process_service_check_result(int cmd,time_t check_time,char* args); // processes a passive service check
int process_passive_service_check(time_t check_time, char const* host_name, char const* svc_description, int return_code, char const* output);
int cmd_process_host_check_result(int cmd,time_t check_time,char* args);    // processes a passive host check
int process_passive_host_check(time_t check_time, char const* host_name, int return_code, char const* output);
int cmd_acknowledge_problem(int cmd,char* args);                            // acknowledges a host or service problem
int cmd_remove_acknowledgement(int cmd,char* args);                         // removes a host or service acknowledgement
int cmd_schedule_downtime(int cmd, time_t entry_time, char* args);          // schedules host or service downtime
int cmd_delete_downtime(int cmd, char* args);                               // cancels active/pending host or service scheduled downtime
int cmd_delete_downtime_full(int cmd, char* args);                          // cancels active/pending host or service scheduled downtime
int cmd_change_object_int_var(int cmd,char* args);                          // changes host/svc (int) variable
int cmd_change_object_char_var(int cmd,char* args);                         // changes host/svc (char) variable
int cmd_change_object_custom_var(int cmd, char* args);                      // changes host/svc custom variable
int cmd_process_external_commands_from_file(int cmd, char* args);           // process external commands from a file
int cmd_delete_downtime_by_start_time_comment(int, char*);
int cmd_delete_downtime_by_host_name(int, char*);
int cmd_delete_downtime_by_hostgroup_name(int, char*);
void disable_service_checks(com::centreon::engine::service* svc);                                  // disables a service check
void enable_service_checks(com::centreon::engine::service* svc);                                   // enables a service check
void enable_all_notifications(void);                                        // enables notifications on a program-wide basis
void disable_all_notifications(void);                                       // disables notifications on a program-wide basis
void enable_service_notifications(com::centreon::engine::service* svc);                            // enables service notifications
void disable_service_notifications(com::centreon::engine::service* svc);                           // disables service notifications
void enable_host_notifications(com::centreon::engine::host* hst);           // enables host notifications
void disable_host_notifications(com::centreon::engine::host* hst);          // disables host notifications
void enable_and_propagate_notifications(com::centreon::engine::host* hst, int level, int affect_top_host, int affect_hosts, int affect_services);  // enables notifications for all hosts and services beyond a given host
void disable_and_propagate_notifications(com::centreon::engine::host* hst, int level, int affect_top_host, int affect_hosts, int affect_services); // disables notifications for all hosts and services beyond a given host
void enable_contact_host_notifications(com::centreon::engine::contact* cntct);                     // enables host notifications for a specific contact
void disable_contact_host_notifications(com::centreon::engine::contact* cntct);                    // disables host notifications for a specific contact
void enable_contact_service_notifications(com::centreon::engine::contact* cntct);                  // enables service notifications for a specific contact
void disable_contact_service_notifications(com::centreon::engine::contact* cntct);                 // disables service notifications for a specific contact
void schedule_and_propagate_downtime(com::centreon::engine::host* temp_host, time_t entry_time, char const* author, char const* comment_data, time_t start_time, time_t end_time, int fixed, unsigned long triggered_by, unsigned long duration); // schedules downtime for all hosts beyond a given host
void acknowledge_host_problem(com::centreon::engine::host* hst, char* ack_author, char* ack_data, int type, int notify, int persistent);       // acknowledges a host problem
void acknowledge_service_problem(com::centreon::engine::service* svc, char* ack_author, char* ack_data, int type, int notify, int persistent); // acknowledges a service problem
void remove_host_acknowledgement(com::centreon::engine::host* hst);        // removes a host acknowledgement
void remove_service_acknowledgement(com::centreon::engine::service* svc);                         // removes a service acknowledgement
void start_executing_service_checks(void);                                 // starts executing service checks
void stop_executing_service_checks(void);                                  // stops executing service checks
void start_accepting_passive_service_checks(void);                         // starts accepting passive service check results
void stop_accepting_passive_service_checks(void);                          // stops accepting passive service check results
void enable_passive_service_checks(com::centreon::engine::service* svc);                          // enables passive service checks for a particular service
void disable_passive_service_checks(com::centreon::engine::service* svc);                         // disables passive service checks for a particular service
void start_executing_host_checks(void);                                    // starts executing host checks
void stop_executing_host_checks(void);                                     // stops executing host checks
void start_accepting_passive_host_checks(void);                            // starts accepting passive host check results
void stop_accepting_passive_host_checks(void);                             // stops accepting passive host check results
void enable_passive_host_checks(com::centreon::engine::host* hst);         // enables passive host checks for a particular host
void disable_passive_host_checks(com::centreon::engine::host* hst);        // disables passive host checks for a particular host
void start_using_event_handlers(void);                                     // enables event handlers on a program-wide basis
void stop_using_event_handlers(void);                                      // disables event handlers on a program-wide basis
void enable_service_event_handler(com::centreon::engine::service* svc);                           // enables the event handler for a particular service
void disable_service_event_handler(com::centreon::engine::service* svc);                          // disables the event handler for a particular service
void enable_host_event_handler(com::centreon::engine::host* hst);          // enables the event handler for a particular host
void disable_host_event_handler(com::centreon::engine::host* hst);         // disables the event handler for a particular host
void disable_host_checks(com::centreon::engine::host* hst);                // disables checks of a particular host
void enable_host_checks(com::centreon::engine::host* hst);                 // enables checks of a particular host
void start_obsessing_over_service_checks(void);                            // start obsessing about service check results
void stop_obsessing_over_service_checks(void);                             // stop obsessing about service check results
void start_obsessing_over_host_checks(void);                               // start obsessing about host check results
void stop_obsessing_over_host_checks(void);                                // stop obsessing about host check results
void enable_service_freshness_checks(void);                                // enable service freshness checks
void disable_service_freshness_checks(void);                               // disable service freshness checks
void enable_host_freshness_checks(void);                                   // enable host freshness checks
void disable_host_freshness_checks(void);                                  // disable host freshness checks
void enable_performance_data(void);                                        // enables processing of performance data on a program-wide basis
void disable_performance_data(void);                                       // disables processing of performance data on a program-wide basis
void start_obsessing_over_service(com::centreon::engine::service* svc);                           // start obsessing about specific service check results
void stop_obsessing_over_service(com::centreon::engine::service* svc);                            // stop obsessing about specific service check results
void start_obsessing_over_host(com::centreon::engine::host* hst);             // start obsessing about specific host check results
void stop_obsessing_over_host(com::centreon::engine::host* hst);              // stop obsessing about specific host check results
void set_host_notification_number(com::centreon::engine::host* hst, int num); // sets current notification number for a specific host
void set_service_notification_number(com::centreon::engine::service* svc, int num);               // sets current notification number for a specific service

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_MOD_EXTCMD_COMMANDS_HH
