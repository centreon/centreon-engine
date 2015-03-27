/*
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

#include <cstdlib>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/modules/external_commands/processing.hh"
#include "com/centreon/engine/retention/applier/state.hh"
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/retention/parser.hh"
#include "com/centreon/engine/retention/state.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules::external_command;

processing::processing() {
  // process commands.
  _lst_command["SHUTDOWN_PROGRAM"] =
    command_info(CMD_SHUTDOWN_PROCESS,
                 &_redirector<&cmd_signal_process>);
  _lst_command["SHUTDOWN_PROCESS"] =
    command_info(CMD_SHUTDOWN_PROCESS,
                 &_redirector<&cmd_signal_process>);
  _lst_command["RESTART_PROGRAM"] =
    command_info(CMD_RESTART_PROCESS,
                 &_redirector<&cmd_signal_process>);
  _lst_command["RESTART_PROCESS"] =
    command_info(CMD_RESTART_PROCESS,
                 &_redirector<&cmd_signal_process>);
  _lst_command["SAVE_STATE_INFORMATION"] =
    command_info(CMD_SAVE_STATE_INFORMATION,
                 &_redirector<&_wrapper_save_state_information>);
  _lst_command["READ_STATE_INFORMATION"] =
    command_info(CMD_READ_STATE_INFORMATION,
                 &_redirector<&_wrapper_read_state_information>);
  _lst_command["ENABLE_EVENT_HANDLERS"] =
    command_info(CMD_ENABLE_EVENT_HANDLERS,
                 &_redirector<&start_using_event_handlers>);
  _lst_command["DISABLE_EVENT_HANDLERS"] =
    command_info(CMD_DISABLE_EVENT_HANDLERS,
                 &_redirector<&stop_using_event_handlers>);
  // _lst_command["FLUSH_PENDING_COMMANDS"] =
  //   command_info(CMD_FLUSH_PENDING_COMMANDS,
  //                &_redirector<&>);
  _lst_command["START_OBSESSING_OVER_HOST_CHECKS"] =
    command_info(CMD_START_OBSESSING_OVER_HOST_CHECKS,
                 &_redirector<&start_obsessing_over_host_checks>);
  _lst_command["STOP_OBSESSING_OVER_HOST_CHECKS"] =
    command_info(CMD_STOP_OBSESSING_OVER_HOST_CHECKS,
                 &_redirector<&stop_obsessing_over_host_checks>);
  _lst_command["START_OBSESSING_OVER_SVC_CHECKS"] =
    command_info(CMD_START_OBSESSING_OVER_SVC_CHECKS,
                 &_redirector<&start_obsessing_over_service_checks>);
  _lst_command["STOP_OBSESSING_OVER_SVC_CHECKS"] =
    command_info(CMD_STOP_OBSESSING_OVER_SVC_CHECKS,
                 &_redirector<&stop_obsessing_over_service_checks>);
  _lst_command["ENABLE_FLAP_DETECTION"] =
    command_info(CMD_ENABLE_FLAP_DETECTION,
                 &_redirector<&enable_flap_detection_routines>);
  _lst_command["DISABLE_FLAP_DETECTION"] =
    command_info(CMD_DISABLE_FLAP_DETECTION,
                 &_redirector<&disable_flap_detection_routines>);
  _lst_command["CHANGE_GLOBAL_HOST_EVENT_HANDLER"] =
    command_info(CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER,
                 &_redirector<&cmd_change_object_char_var>);
  _lst_command["CHANGE_GLOBAL_SVC_EVENT_HANDLER"] =
    command_info(CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER,
                 &_redirector<&cmd_change_object_char_var>);
  _lst_command["ENABLE_SERVICE_FRESHNESS_CHECKS"] =
    command_info(CMD_ENABLE_SERVICE_FRESHNESS_CHECKS,
                 &_redirector<&enable_service_freshness_checks>);
  _lst_command["DISABLE_SERVICE_FRESHNESS_CHECKS"] =
    command_info(CMD_DISABLE_SERVICE_FRESHNESS_CHECKS,
                 &_redirector<&disable_service_freshness_checks>);
  _lst_command["ENABLE_HOST_FRESHNESS_CHECKS"] =
    command_info(CMD_ENABLE_HOST_FRESHNESS_CHECKS,
                 &_redirector<&enable_host_freshness_checks>);
  _lst_command["DISABLE_HOST_FRESHNESS_CHECKS"] =
    command_info(CMD_DISABLE_HOST_FRESHNESS_CHECKS,
                 &_redirector<&disable_host_freshness_checks>);

  // host-related commands.
  _lst_command["ENABLE_HOST_SVC_CHECKS"] =
    command_info(CMD_ENABLE_HOST_SVC_CHECKS,
                 &_redirector_host<&_wrapper_enable_host_svc_checks>);
  _lst_command["DISABLE_HOST_SVC_CHECKS"] =
    command_info(CMD_DISABLE_HOST_SVC_CHECKS,
                 &_redirector_host<&_wrapper_disable_host_svc_checks>);
  _lst_command["SCHEDULE_HOST_SVC_CHECKS"] =
    command_info(CMD_SCHEDULE_HOST_SVC_CHECKS,
                 &_redirector<&cmd_schedule_check>);
  _lst_command["SCHEDULE_FORCED_HOST_SVC_CHECKS"] =
    command_info(CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS,
                 &_redirector<&cmd_schedule_check>);
  _lst_command["ENABLE_HOST_EVENT_HANDLER"] =
    command_info(CMD_ENABLE_HOST_EVENT_HANDLER,
                 &_redirector_host<&enable_host_event_handler>);
  _lst_command["DISABLE_HOST_EVENT_HANDLER"] =
    command_info(CMD_DISABLE_HOST_EVENT_HANDLER,
                 &_redirector_host<&disable_host_event_handler>);
  _lst_command["ENABLE_HOST_CHECK"] =
    command_info(CMD_ENABLE_HOST_CHECK,
                 &_redirector_host<&enable_host_checks>);
  _lst_command["DISABLE_HOST_CHECK"] =
    command_info(CMD_DISABLE_HOST_CHECK,
                 &_redirector_host<&disable_host_checks>);
  _lst_command["SCHEDULE_HOST_CHECK"] =
    command_info(CMD_SCHEDULE_HOST_CHECK,
                 &_redirector<&cmd_schedule_check>);
  _lst_command["SCHEDULE_FORCED_HOST_CHECK"] =
    command_info(CMD_SCHEDULE_FORCED_HOST_CHECK,
                 &_redirector<&cmd_schedule_check>);
  _lst_command["ENABLE_HOST_FLAP_DETECTION"] =
    command_info(CMD_ENABLE_HOST_FLAP_DETECTION,
                 &_redirector_host<&enable_host_flap_detection>);
  _lst_command["DISABLE_HOST_FLAP_DETECTION"] =
    command_info(CMD_DISABLE_HOST_FLAP_DETECTION,
                 &_redirector_host<&disable_host_flap_detection>);
  _lst_command["START_OBSESSING_OVER_HOST"] =
    command_info(CMD_START_OBSESSING_OVER_HOST,
                 &_redirector_host<&start_obsessing_over_host>);
  _lst_command["STOP_OBSESSING_OVER_HOST"] =
    command_info(CMD_STOP_OBSESSING_OVER_HOST,
                 &_redirector_host<&stop_obsessing_over_host>);
  _lst_command["CHANGE_HOST_EVENT_HANDLER"] =
    command_info(CMD_CHANGE_HOST_EVENT_HANDLER,
                 &_redirector<&cmd_change_object_char_var>);
  _lst_command["CHANGE_HOST_CHECK_COMMAND"] =
    command_info(CMD_CHANGE_HOST_CHECK_COMMAND,
                 &_redirector<&cmd_change_object_char_var>);
  _lst_command["CHANGE_NORMAL_HOST_CHECK_INTERVAL"] =
    command_info(CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL,
                 &_redirector<&cmd_change_object_int_var>);
  _lst_command["CHANGE_RETRY_HOST_CHECK_INTERVAL"] =
    command_info(CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL,
                 &_redirector<&cmd_change_object_int_var>);
  _lst_command["CHANGE_MAX_HOST_CHECK_ATTEMPTS"] =
    command_info(CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS,
                 &_redirector<&cmd_change_object_int_var>);
  _lst_command["CHANGE_HOST_CHECK_TIMEPERIOD"] =
    command_info(CMD_CHANGE_HOST_CHECK_TIMEPERIOD,
                 &_redirector<&cmd_change_object_char_var>);
  _lst_command["CHANGE_CUSTOM_HOST_VAR"] =
    command_info(CMD_CHANGE_CUSTOM_HOST_VAR,
                 &_redirector<&cmd_change_object_custom_var>);
  _lst_command["CHANGE_HOST_MODATTR"] =
    command_info(CMD_CHANGE_HOST_MODATTR,
                 &_redirector<&cmd_change_object_int_var>);

  // hostgroup-related commands.
  _lst_command["ENABLE_HOSTGROUP_HOST_CHECKS"] =
    command_info(CMD_ENABLE_HOSTGROUP_HOST_CHECKS,
                 &_redirector_hostgroup<&enable_host_checks>);
  _lst_command["DISABLE_HOSTGROUP_HOST_CHECKS"] =
    command_info(CMD_DISABLE_HOSTGROUP_HOST_CHECKS,
                 &_redirector_hostgroup<&disable_host_checks>);
  _lst_command["ENABLE_HOSTGROUP_SVC_CHECKS"] =
    command_info(CMD_ENABLE_HOSTGROUP_SVC_CHECKS,
                 &_redirector_hostgroup<&_wrapper_enable_service_checks>);
  _lst_command["DISABLE_HOSTGROUP_SVC_CHECKS"] =
    command_info(CMD_DISABLE_HOSTGROUP_SVC_CHECKS,
                 &_redirector_hostgroup<&_wrapper_disable_service_checks>);

  // service-related commands.
  _lst_command["SCHEDULE_SVC_CHECK"] =
    command_info(CMD_SCHEDULE_SVC_CHECK,
                 &_redirector<&cmd_schedule_check>);
  _lst_command["SCHEDULE_FORCED_SVC_CHECK"] =
    command_info(CMD_SCHEDULE_FORCED_SVC_CHECK,
                 &_redirector<&cmd_schedule_check>);
  _lst_command["ENABLE_SVC_CHECK"] =
    command_info(CMD_ENABLE_SVC_CHECK,
                 &_redirector_service<&enable_service_checks>);
  _lst_command["DISABLE_SVC_CHECK"] =
    command_info(CMD_DISABLE_SVC_CHECK,
                 &_redirector_service<&disable_service_checks>);
  _lst_command["PROCESS_SERVICE_CHECK_RESULT"] =
    command_info(CMD_PROCESS_SERVICE_CHECK_RESULT,
                 &_redirector<&cmd_process_service_check_result>);
  _lst_command["PROCESS_HOST_CHECK_RESULT"] =
    command_info(CMD_PROCESS_HOST_CHECK_RESULT,
                 &_redirector<&cmd_process_host_check_result>);
  _lst_command["ENABLE_SVC_EVENT_HANDLER"] =
    command_info(CMD_ENABLE_SVC_EVENT_HANDLER,
                 &_redirector_service<&enable_service_event_handler>);
  _lst_command["DISABLE_SVC_EVENT_HANDLER"] =
    command_info(CMD_DISABLE_SVC_EVENT_HANDLER,
                 &_redirector_service<&disable_service_event_handler>);
  _lst_command["ENABLE_SVC_FLAP_DETECTION"] =
    command_info(CMD_ENABLE_SVC_FLAP_DETECTION,
                 &_redirector_service<&enable_service_flap_detection>);
  _lst_command["DISABLE_SVC_FLAP_DETECTION"] =
    command_info(CMD_DISABLE_SVC_FLAP_DETECTION,
                 &_redirector_service<&disable_service_flap_detection>);
  _lst_command["START_OBSESSING_OVER_SVC"] =
    command_info(CMD_START_OBSESSING_OVER_SVC,
                 &_redirector_service<&start_obsessing_over_service>);
  _lst_command["STOP_OBSESSING_OVER_SVC"] =
    command_info(CMD_STOP_OBSESSING_OVER_SVC,
                 &_redirector_service<&stop_obsessing_over_service>);
  _lst_command["CHANGE_SVC_EVENT_HANDLER"] =
    command_info(CMD_CHANGE_SVC_EVENT_HANDLER,
                 &_redirector<&cmd_change_object_char_var>);
  _lst_command["CHANGE_SVC_CHECK_COMMAND"] =
    command_info(CMD_CHANGE_SVC_CHECK_COMMAND,
                 &_redirector<&cmd_change_object_char_var>);
  _lst_command["CHANGE_NORMAL_SVC_CHECK_INTERVAL"] =
    command_info(CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL,
                 &_redirector<&cmd_change_object_int_var>);
  _lst_command["CHANGE_RETRY_SVC_CHECK_INTERVAL"] =
    command_info(CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL,
                 &_redirector<&cmd_change_object_int_var>);
  _lst_command["CHANGE_MAX_SVC_CHECK_ATTEMPTS"] =
    command_info(CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS,
                 &_redirector<&cmd_change_object_int_var>);
  _lst_command["CHANGE_SVC_CHECK_TIMEPERIOD"] =
    command_info(CMD_CHANGE_SVC_CHECK_TIMEPERIOD,
                 &_redirector<&cmd_change_object_char_var>);
  _lst_command["CHANGE_CUSTOM_SVC_VAR"] =
    command_info(CMD_CHANGE_CUSTOM_SVC_VAR,
                 &_redirector<&cmd_change_object_custom_var>);
  _lst_command["CHANGE_SVC_MODATTR"] =
    command_info(CMD_CHANGE_SVC_MODATTR,
                 &_redirector<&cmd_change_object_int_var>);

  // servicegroup-related commands.
  _lst_command["ENABLE_SERVICEGROUP_HOST_CHECKS"] =
    command_info(CMD_ENABLE_SERVICEGROUP_HOST_CHECKS,
                 &_redirector_servicegroup<&enable_host_checks>);
  _lst_command["DISABLE_SERVICEGROUP_HOST_CHECKS"] =
    command_info(CMD_DISABLE_SERVICEGROUP_HOST_CHECKS,
                 &_redirector_servicegroup<&disable_host_checks>);
  _lst_command["ENABLE_SERVICEGROUP_SVC_CHECKS"] =
    command_info(CMD_ENABLE_SERVICEGROUP_SVC_CHECKS,
                 &_redirector_servicegroup<&enable_service_checks>);
  _lst_command["DISABLE_SERVICEGROUP_SVC_CHECKS"] =
    command_info(CMD_DISABLE_SERVICEGROUP_SVC_CHECKS,
                 &_redirector_servicegroup<&disable_service_checks>);

  // misc commands.
  _lst_command["PROCESS_FILE"] =
    command_info(CMD_PROCESS_FILE,
                 &_redirector<&cmd_process_external_commands_from_file>);
}

processing::~processing() throw () {}

bool processing::execute(char const* cmd) const {
  logger(dbg_functions, basic) << "processing external command";

  if (!cmd)
    return (false);

  // Trim command
  while (*cmd && isspace(*cmd))
    ++cmd;
  unsigned int len(strlen(cmd));
  unsigned int end(len);
  while (end && isspace(cmd[end - 1]))
    --end;
  char* command(new char[end + 1]);
  memcpy(command, cmd, end);
  command[end] = 0;

  logger(dbg_external_command, most) << "raw command: " << command;

  if (end < 15
      || command[0] != '['
      || command[11] != ']'
      || command[12] != ' ') {
    delete[] command;
    return (false);
  }

  unsigned int start(13);
  while (command[start]) {
    if (command[start] == ';') {
      command[start] = 0;
      ++start;
      break;
    }
    ++start;
  }

  time_t entry_time(static_cast<time_t>(strtoul(command + 1, NULL, 10)));
  char* command_name(command + 13);
  char* args(command + start);
  int command_id(CMD_CUSTOM_COMMAND);

  umap<std::string, command_info>::const_iterator
    it(_lst_command.find(command_name));
  if (it != _lst_command.end())
    command_id = it->second.id;
  else if (command_name[0] != '_') {
    logger(log_external_command | log_runtime_warning, basic)
      << "Warning: Unrecognized external command -> " << command_name;
    delete[] command;
    return (false);
  }

  // update statistics for external commands.
  update_check_stats(EXTERNAL_COMMAND_STATS, time(NULL));

  // log the external command.
  if (command_id == CMD_PROCESS_SERVICE_CHECK_RESULT
      || command_id == CMD_PROCESS_HOST_CHECK_RESULT) {
    // passive checks are logged in checks.c.
    if (config->log_passive_checks())
      logger(log_passive_check, basic)
        << "EXTERNAL COMMAND: " << command_name << ';' << args;
  }
  else if (config->log_external_commands())
    logger(log_external_command, basic)
      << "EXTERNAL COMMAND: " << command_name << ';' << args;

  logger(dbg_external_command, more)
    << "External command id: " << command_id
    << "\nCommand entry time: " << entry_time
    << "\nCommand arguments: " << args;

  // send data to event broker.
  broker_external_command(
    NEBTYPE_EXTERNALCOMMAND_START,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    command_id,
    entry_time,
    command_name,
    args,
    NULL);

  if (it != _lst_command.end())
    (*it->second.func)(command_id, entry_time, args);

  // send data to event broker.
  broker_external_command(
    NEBTYPE_EXTERNALCOMMAND_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    command_id,
    entry_time,
    command_name,
    args,
    NULL);

  delete[] command;
  return (true);
}

void processing::_wrapper_read_state_information() {
  try {
    retention::state state;
    retention::parser p;
    p.parse(config->state_retention_file(), state);
    retention::applier::state app_state;
    app_state.apply(*config, state);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: could not load retention file: "
      << e.what();
  }
  return ;
}

void processing::_wrapper_save_state_information() {
  retention::dump::save(config->state_retention_file());
}

void processing::_wrapper_disable_host_svc_checks(host* hst) {
  for (servicesmember* member = hst->services;
       member != NULL;
       member = member->next)
    if (member->service_ptr)
      disable_service_checks(member->service_ptr);
}

void processing::_wrapper_enable_host_svc_checks(host* hst) {
  for (servicesmember* member = hst->services;
       member != NULL;
       member = member->next)
    if (member->service_ptr)
      enable_service_checks(member->service_ptr);
}

void processing::_wrapper_enable_service_checks(host* hst) {
  for (servicesmember* member = hst->services;
       member != NULL;
       member = member->next)
    if (member->service_ptr)
      enable_service_checks(member->service_ptr);
}

void processing::_wrapper_disable_service_checks(host* hst) {
  for (servicesmember* member = hst->services;
       member != NULL;
       member = member->next)
    if (member->service_ptr)
      disable_service_checks(member->service_ptr);
}
