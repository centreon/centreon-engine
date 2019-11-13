/*
** Copyright 2012-2013 Merethis
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

#include <time.h>
#include <iostream>
#include "com/centreon/engine/events/sched_info.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "downtime.h"
#include "nagios.h"
#include "nebstructs.h"

/**
 *  Write function to output stream.
 *
 *  @param[in,out] out Output stream.
 *  @param[in]     ptr Function pointer.
 *
 *  @return out.
 */
std::ostream& operator<<(std::ostream& out, void (*ptr)()) {
  union {
    void (*func)();
    void* data;
  } type;
  type.func = ptr;
  return (out << type.data);
}

extern "C" {
// External symbols not exported by headers.
extern int accept_passive_host_checks;
extern int accept_passive_service_checks;
extern int check_external_commands;
extern int check_host_freshness;
extern check_result check_result_info;
extern check_result* check_result_list;
extern int check_service_freshness;
extern command* command_list;
extern char* config_file;
extern contact* contact_list;
extern contactgroup* contactgroup_list;
extern int currently_running_host_checks;
extern int currently_running_service_checks;
extern int enable_event_handlers;
extern int enable_flap_detection;
extern int enable_notifications;
extern unsigned long event_broker_options;
extern timed_event* event_list_high;
extern timed_event* event_list_high_tail;
extern int execute_host_checks;
extern int execute_service_checks;
extern circular_buffer external_command_buffer;
extern int external_command_buffer_slots;
extern char* global_host_event_handler;
extern char* global_service_event_handler;
extern unsigned int host_check_timeout;
extern host* host_list;
extern hostdependency* hostdependency_list;
extern hostescalation* hostescalation_list;
extern hostgroup* hostgroup_list;
extern int interval_length;
extern time_t last_command_check;
extern time_t last_log_rotation;
extern char* log_archive_path;
extern char* log_file;
extern char* macro_user[MAX_USER_MACROS];
extern int nagios_pid;
extern nebmodule* neb_module_list;
extern int obsess_over_hosts;
extern int obsess_over_services;
extern int process_performance_data;
extern time_t program_start;
extern sched_info* scheduling_info;
extern int service_check_timeout;
extern service* service_list;
extern servicedependency* servicedependency_list;
extern serviceescalation* serviceescalation_list;
extern servicegroup* servicegroup_list;
extern int submit_external_command(char const* cmd, int* buffer_items);
extern timeperiod* timeperiod_list;

/**
 *  This is a shared object that requires multiple symbols that should
 *  be defined by Centreon Engine to provide compatibility with
 *  well-known modules.
 */
void run() {
  // Centreon Broker 2.1.0
  std::cout << "Centreon Broker 2.1.0\n"
            << "event_list_high: " << event_list_high << "\n"
            << "event_list_high_tail: " << event_list_high << "\n"
            << "find_host_comment: " << (void (*)())find_host_comment << "\n"
            << "find_service_comment: " << (void (*)())find_service_comment
            << "\n"
            << "get_program_version: " << (void (*)())get_program_version
            << "\n"
            << "host_list: " << host_list << "\n"
            << "hostdependency_list: " << hostdependency_list << "\n"
            << "hostgroup_list: " << hostgroup_list << "\n"
            << "neb_deregister_callback: "
            << (void (*)())neb_deregister_callback << "\n"
            << "neb_module_list: " << neb_module_list << "\n"
            << "neb_register_callback: " << (void (*)())neb_register_callback
            << "\n"
            << "neb_set_module_info: " << (void (*)())neb_set_module_info
            << "\n"
            << "remove_event: " << (void (*)())remove_event << "\n"
            << "schedule_new_event: " << (void (*)())schedule_new_event << "\n"
            << "service_list: " << service_list << "\n"
            << "servicedependency_list: " << servicedependency_list << "\n"
            << "servicegroup_list: " << servicegroup_list << "\n"
            << std::endl;

  // NDOUtils 1.4b9
  std::cout << "NDOUtils 1.4b9\n"
            << "command_list: " << command_list << "\n"
            << "config_file: " << config_file << "\n"
            << "contact_list: " << contact_list << "\n"
            << "contactgroup_list: " << contactgroup_list << "\n"
            << "find_command: " << (void (*)())find_command << "\n"
            << "find_downtime: " << (void (*)())find_downtime << "\n"
            << "find_host_comment: " << (void (*)())find_host_comment << "\n"
            << "find_service_comment: " << (void (*)())find_service_comment
            << "\n"
            << "get_program_modification_date: "
            << (void (*)())get_program_modification_date << "\n"
            << "get_program_version: " << (void (*)())get_program_version
            << "\n"
            << "get_raw_command_line: " << (void (*)())get_raw_command_line
            << "\n"
            << "global_host_event_handler: " << global_host_event_handler
            << "\n"
            << "global_service_event_handler: " << global_service_event_handler
            << "\n"
            << "host_list: " << host_list << "\n"
            << "hostdependency_list: " << hostdependency_list << "\n"
            << "hostescalation_list: " << hostescalation_list << "\n"
            << "hostgroup_list: " << hostgroup_list << "\n"
            << "my_system: " << (void (*)())my_system << "\n"
            << "neb_deregister_callback: "
            << (void (*)())neb_deregister_callback << "\n"
            << "neb_register_callback: " << (void (*)())neb_register_callback
            << "\n"
            << "process_macros: " << (void (*)())process_macros << "\n"
            << "schedule_new_event: " << (void (*)())schedule_new_event << "\n"
            << "scheduling_info: " << scheduling_info << "\n"
            << "service_list: " << service_list << "\n"
            << "servicedependency_list: " << servicedependency_list << "\n"
            << "serviceescalation_list: " << serviceescalation_list << "\n"
            << "servicegroup_list: " << servicegroup_list << "\n"
            << "strip: " << (void (*)())strip << "\n"
            << "timeperiod_list: " << timeperiod_list << "\n"
            << "write_to_all_logs: " << (void (*)())write_to_all_logs << "\n"
            << std::endl;

  // mk-livestatus 1.1.12p6
  std::cout
      << "mk-livestatus 1.1.12p6\n"
      << "accept_passive_host_checks: " << accept_passive_host_checks << "\n"
      << "accept_passive_service_checks: " << accept_passive_service_checks
      << "\n"
      << "check_external_commands: " << check_external_commands << "\n"
      << "check_host_freshness: " << check_host_freshness << "\n"
      << "check_service_freshness: " << check_service_freshness << "\n"
      << "check_time_against_period: " << (void (*)())check_time_against_period
      << "\n"
      << "command_list: " << command_list << "\n"
      << "contactgroup_list: " << contactgroup_list << "\n"
      << "contact_list: " << contact_list << "\n"
      << "enable_event_handlers: " << enable_event_handlers << "\n"
      << "enable_flap_detection: " << enable_flap_detection << "\n"
      << "enable_notifications: " << enable_notifications << "\n"
      << "event_broker_options: " << event_broker_options << "\n"
      << "execute_host_checks: " << execute_host_checks << "\n"
      << "execute_service_checks: " << execute_service_checks << "\n"
      << "external_command_buffer: " << external_command_buffer.buffer << "\n"
      << "external_command_buffer_slots: " << external_command_buffer_slots
      << "\n"
      << "find_command: " << (void (*)())find_command << "\n"
      << "find_contact: " << (void (*)())find_contact << "\n"
      << "find_contactgroup: " << (void (*)())find_contactgroup << "\n"
      << "find_host: " << (void (*)())find_host << "\n"
      << "find_hostgroup: " << (void (*)())find_hostgroup << "\n"
      << "find_service: " << (void (*)())find_service << "\n"
      << "find_servicegroup: " << (void (*)())find_servicegroup << "\n"
      << "get_program_version: " << (void (*)())get_program_version << "\n"
      << "hostgroup_list: " << hostgroup_list << "\n"
      << "host_list: " << host_list << "\n"
      << "interval_length: " << interval_length << "\n"
      << "is_contact_for_host: " << (void (*)())is_contact_for_host << "\n"
      << "is_contact_for_service: " << (void (*)())is_contact_for_service
      << "\n"
      << "is_escalated_contact_for_host: "
      << (void (*)())is_escalated_contact_for_host << "\n"
      << "is_escalated_contact_for_service: "
      << (void (*)())is_escalated_contact_for_service << "\n"
      << "last_command_check: " << last_command_check << "\n"
      << "last_log_rotation: " << last_log_rotation << "\n"
      << "log_archive_path: " << log_archive_path << "\n"
      << "log_file: " << log_file << "\n"
      << "macro_user: " << macro_user << "\n"
      << "nagios_pid: " << nagios_pid << "\n"
      << "neb_deregister_callback: " << (void (*)())neb_deregister_callback
      << "\n"
      << "neb_register_callback: " << (void (*)())neb_register_callback << "\n"
      << "obsess_over_hosts: " << obsess_over_hosts << "\n"
      << "obsess_over_services: " << obsess_over_services << "\n"
      << "process_performance_data: " << process_performance_data << "\n"
      << "program_start: " << program_start << "\n"
      << "rotate_log_file: " << (void (*)())rotate_log_file << "\n"
      << "servicegroup_list: " << servicegroup_list << "\n"
      << "service_list: " << service_list << "\n"
      << "submit_external_command: " << (void (*)())submit_external_command
      << "\n"
      << "timeperiod_list: " << timeperiod_list << "\n"
      << "write_to_all_logs: " << (void (*)())write_to_all_logs << "\n"
      << std::endl;

  // npcdmod 0.6.16
  std::cout << "npcdmod 0.6.16\n"
            << "find_host: " << (void (*)())find_host << "\n"
            << "find_service: " << (void (*)())find_service << "\n"
            << "my_rename: " << (void (*)())my_rename << "\n"
            << "neb_deregister_callback: "
            << (void (*)())neb_deregister_callback << "\n"
            << "neb_register_callback: " << (void (*)())neb_register_callback
            << "\n"
            << "neb_set_module_info: " << (void (*)())neb_set_module_info
            << "\n"
            << "process_performance_data: " << process_performance_data << "\n"
            << "schedule_new_event: " << (void (*)())schedule_new_event << "\n"
            << "strip: " << (void (*)())strip << "\n"
            << "write_to_all_logs: " << (void (*)())write_to_all_logs << "\n"
            << std::endl;

  // mod_gearman 1.3.8
  std::cout << "mod_gearman 1.3.8\n"
            << "adjust_host_check_attempt_3x: "
            << (void (*)())adjust_host_check_attempt_3x << "\n"
            << "check_result_info: " << &check_result_info << "\n"
            << "check_result_list: " << check_result_list << "\n"
            << "clear_volatile_macros: " << (void (*)())clear_volatile_macros
            << "\n"
            << "currently_running_host_checks: "
            << currently_running_host_checks << "\n"
            << "currently_running_service_checks: "
            << currently_running_service_checks << "\n"
            << "event_broker_options: " << event_broker_options << "\n"
            << "find_host: " << (void (*)())find_host << "\n"
            << "find_hostgroup: " << (void (*)())find_hostgroup << "\n"
            << "find_service: " << (void (*)())find_service << "\n"
            << "find_servicegroup: " << (void (*)())find_servicegroup << "\n"
            << "grab_host_macros: " << (void (*)())grab_host_macros << "\n"
            << "grab_service_macros: " << (void (*)())grab_service_macros
            << "\n"
            << "host_check_timeout: " << host_check_timeout << "\n"
            << "init_check_result: " << (void (*)())init_check_result << "\n"
            << "is_host_member_of_hostgroup: "
            << (void (*)())is_host_member_of_hostgroup << "\n"
            << "is_service_member_of_servicegroup: "
            << (void (*)())is_service_member_of_servicegroup << "\n"
            << "neb_deregister_callback: "
            << (void (*)())neb_deregister_callback << "\n"
            << "neb_register_callback: " << (void (*)())neb_register_callback
            << "\n"
            << "neb_set_module_info: " << (void (*)())neb_set_module_info
            << "\n"
            << "process_macros: " << (void (*)())process_macros << "\n"
            << "process_performance_data: " << process_performance_data << "\n"
            << "service_check_timeout: " << service_check_timeout << "\n"
            << "write_to_all_logs: " << (void (*)())write_to_all_logs << "\n"
            << std::endl;

  return;
}
}
