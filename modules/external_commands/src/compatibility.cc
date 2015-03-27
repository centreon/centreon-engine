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

#include <cstdlib>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/modules/external_commands/compatibility.hh"
#include "com/centreon/engine/retention/applier/state.hh"
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/retention/parser.hh"
#include "com/centreon/engine/retention/state.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

int process_external_command1(char* cmd) {
  char* command_id = NULL;
  char* args = NULL;
  time_t entry_time = 0L;
  int command_type = CMD_NONE;
  char* temp_ptr = NULL;

  logger(dbg_functions, basic)
    << "process_external_command1()";

  if (cmd == NULL)
    return (ERROR);

  /* strip the command of newlines and carriage returns */
  strip(cmd);

  logger(dbg_external_command, most)
    << "Raw command entry: " << cmd;

  /* get the command entry time */
  if ((temp_ptr = my_strtok(cmd, "[")) == NULL)
    return (ERROR);
  if ((temp_ptr = my_strtok(NULL, "]")) == NULL)
    return (ERROR);
  entry_time = (time_t)strtoul(temp_ptr, NULL, 10);

  /* get the command identifier */
  if ((temp_ptr = my_strtok(NULL, ";")) == NULL)
    return (ERROR);
  command_id = string::dup(temp_ptr + 1);

  /* get the command arguments */
  if ((temp_ptr = my_strtok(NULL, "\n")) == NULL)
    args = string::dup("");
  else
    args = string::dup(temp_ptr);

  /* decide what type of command this is... */

  /**************************/
  /**** PROCESS COMMANDS ****/
  /**************************/

  if (!strcmp(command_id, "SHUTDOWN_PROGRAM")
           || !strcmp(command_id, "SHUTDOWN_PROCESS"))
    command_type = CMD_SHUTDOWN_PROCESS;
  else if (!strcmp(command_id, "RESTART_PROGRAM")
           || !strcmp(command_id, "RESTART_PROCESS"))
    command_type = CMD_RESTART_PROCESS;

  else if (!strcmp(command_id, "SAVE_STATE_INFORMATION"))
    command_type = CMD_SAVE_STATE_INFORMATION;
  else if (!strcmp(command_id, "READ_STATE_INFORMATION"))
    command_type = CMD_READ_STATE_INFORMATION;

  else if (!strcmp(command_id, "ENABLE_EVENT_HANDLERS"))
    command_type = CMD_ENABLE_EVENT_HANDLERS;
  else if (!strcmp(command_id, "DISABLE_EVENT_HANDLERS"))
    command_type = CMD_DISABLE_EVENT_HANDLERS;

  else if (!strcmp(command_id, "FLUSH_PENDING_COMMANDS"))
    command_type = CMD_FLUSH_PENDING_COMMANDS;

  else if (!strcmp(command_id, "START_OBSESSING_OVER_HOST_CHECKS"))
    command_type = CMD_START_OBSESSING_OVER_HOST_CHECKS;
  else if (!strcmp(command_id, "STOP_OBSESSING_OVER_HOST_CHECKS"))
    command_type = CMD_STOP_OBSESSING_OVER_HOST_CHECKS;

  else if (!strcmp(command_id, "START_OBSESSING_OVER_SVC_CHECKS"))
    command_type = CMD_START_OBSESSING_OVER_SVC_CHECKS;
  else if (!strcmp(command_id, "STOP_OBSESSING_OVER_SVC_CHECKS"))
    command_type = CMD_STOP_OBSESSING_OVER_SVC_CHECKS;

  else if (!strcmp(command_id, "ENABLE_FLAP_DETECTION"))
    command_type = CMD_ENABLE_FLAP_DETECTION;
  else if (!strcmp(command_id, "DISABLE_FLAP_DETECTION"))
    command_type = CMD_DISABLE_FLAP_DETECTION;

  else if (!strcmp(command_id, "CHANGE_GLOBAL_HOST_EVENT_HANDLER"))
    command_type = CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER;
  else if (!strcmp(command_id, "CHANGE_GLOBAL_SVC_EVENT_HANDLER"))
    command_type = CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER;

  else if (!strcmp(command_id, "ENABLE_SERVICE_FRESHNESS_CHECKS"))
    command_type = CMD_ENABLE_SERVICE_FRESHNESS_CHECKS;
  else if (!strcmp(command_id, "DISABLE_SERVICE_FRESHNESS_CHECKS"))
    command_type = CMD_DISABLE_SERVICE_FRESHNESS_CHECKS;

  else if (!strcmp(command_id, "ENABLE_HOST_FRESHNESS_CHECKS"))
    command_type = CMD_ENABLE_HOST_FRESHNESS_CHECKS;
  else if (!strcmp(command_id, "DISABLE_HOST_FRESHNESS_CHECKS"))
    command_type = CMD_DISABLE_HOST_FRESHNESS_CHECKS;

  /*******************************/
  /**** HOST-RELATED COMMANDS ****/
  /*******************************/

  else if (!strcmp(command_id, "ENABLE_HOST_SVC_CHECKS"))
    command_type = CMD_ENABLE_HOST_SVC_CHECKS;
  else if (!strcmp(command_id, "DISABLE_HOST_SVC_CHECKS"))
    command_type = CMD_DISABLE_HOST_SVC_CHECKS;

  else if (!strcmp(command_id, "SCHEDULE_HOST_SVC_CHECKS"))
    command_type = CMD_SCHEDULE_HOST_SVC_CHECKS;
  else if (!strcmp(command_id, "SCHEDULE_FORCED_HOST_SVC_CHECKS"))
    command_type = CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS;

  else if (!strcmp(command_id, "ENABLE_HOST_EVENT_HANDLER"))
    command_type = CMD_ENABLE_HOST_EVENT_HANDLER;
  else if (!strcmp(command_id, "DISABLE_HOST_EVENT_HANDLER"))
    command_type = CMD_DISABLE_HOST_EVENT_HANDLER;

  else if (!strcmp(command_id, "ENABLE_HOST_CHECK"))
    command_type = CMD_ENABLE_HOST_CHECK;
  else if (!strcmp(command_id, "DISABLE_HOST_CHECK"))
    command_type = CMD_DISABLE_HOST_CHECK;

  else if (!strcmp(command_id, "SCHEDULE_HOST_CHECK"))
    command_type = CMD_SCHEDULE_HOST_CHECK;
  else if (!strcmp(command_id, "SCHEDULE_FORCED_HOST_CHECK"))
    command_type = CMD_SCHEDULE_FORCED_HOST_CHECK;

  else if (!strcmp(command_id, "ENABLE_HOST_FLAP_DETECTION"))
    command_type = CMD_ENABLE_HOST_FLAP_DETECTION;
  else if (!strcmp(command_id, "DISABLE_HOST_FLAP_DETECTION"))
    command_type = CMD_DISABLE_HOST_FLAP_DETECTION;

  else if (!strcmp(command_id, "START_OBSESSING_OVER_HOST"))
    command_type = CMD_START_OBSESSING_OVER_HOST;
  else if (!strcmp(command_id, "STOP_OBSESSING_OVER_HOST"))
    command_type = CMD_STOP_OBSESSING_OVER_HOST;

  else if (!strcmp(command_id, "CHANGE_HOST_EVENT_HANDLER"))
    command_type = CMD_CHANGE_HOST_EVENT_HANDLER;
  else if (!strcmp(command_id, "CHANGE_HOST_CHECK_COMMAND"))
    command_type = CMD_CHANGE_HOST_CHECK_COMMAND;

  else if (!strcmp(command_id, "CHANGE_NORMAL_HOST_CHECK_INTERVAL"))
    command_type = CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL;
  else if (!strcmp(command_id, "CHANGE_RETRY_HOST_CHECK_INTERVAL"))
    command_type = CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL;

  else if (!strcmp(command_id, "CHANGE_MAX_HOST_CHECK_ATTEMPTS"))
    command_type = CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS;

  else if (!strcmp(command_id, "CHANGE_HOST_CHECK_TIMEPERIOD"))
    command_type = CMD_CHANGE_HOST_CHECK_TIMEPERIOD;

  else if (!strcmp(command_id, "CHANGE_CUSTOM_HOST_VAR"))
    command_type = CMD_CHANGE_CUSTOM_HOST_VAR;

  else if (!strcmp(command_id, "CHANGE_HOST_MODATTR"))
    command_type = CMD_CHANGE_HOST_MODATTR;

  /************************************/
  /**** HOSTGROUP-RELATED COMMANDS ****/
  /************************************/

  else if (!strcmp(command_id, "ENABLE_HOSTGROUP_HOST_CHECKS"))
    command_type = CMD_ENABLE_HOSTGROUP_HOST_CHECKS;
  else if (!strcmp(command_id, "DISABLE_HOSTGROUP_HOST_CHECKS"))
    command_type = CMD_DISABLE_HOSTGROUP_HOST_CHECKS;

  else if (!strcmp(command_id, "ENABLE_HOSTGROUP_SVC_CHECKS"))
    command_type = CMD_ENABLE_HOSTGROUP_SVC_CHECKS;
  else if (!strcmp(command_id, "DISABLE_HOSTGROUP_SVC_CHECKS"))
    command_type = CMD_DISABLE_HOSTGROUP_SVC_CHECKS;

  /**********************************/
  /**** SERVICE-RELATED COMMANDS ****/
  /**********************************/

  else if (!strcmp(command_id, "SCHEDULE_SVC_CHECK"))
    command_type = CMD_SCHEDULE_SVC_CHECK;
  else if (!strcmp(command_id, "SCHEDULE_FORCED_SVC_CHECK"))
    command_type = CMD_SCHEDULE_FORCED_SVC_CHECK;

  else if (!strcmp(command_id, "ENABLE_SVC_CHECK"))
    command_type = CMD_ENABLE_SVC_CHECK;
  else if (!strcmp(command_id, "DISABLE_SVC_CHECK"))
    command_type = CMD_DISABLE_SVC_CHECK;

  else if (!strcmp(command_id, "PROCESS_SERVICE_CHECK_RESULT"))
    command_type = CMD_PROCESS_SERVICE_CHECK_RESULT;
  else if (!strcmp(command_id, "PROCESS_HOST_CHECK_RESULT"))
    command_type = CMD_PROCESS_HOST_CHECK_RESULT;

  else if (!strcmp(command_id, "ENABLE_SVC_EVENT_HANDLER"))
    command_type = CMD_ENABLE_SVC_EVENT_HANDLER;
  else if (!strcmp(command_id, "DISABLE_SVC_EVENT_HANDLER"))
    command_type = CMD_DISABLE_SVC_EVENT_HANDLER;

  else if (!strcmp(command_id, "ENABLE_SVC_FLAP_DETECTION"))
    command_type = CMD_ENABLE_SVC_FLAP_DETECTION;
  else if (!strcmp(command_id, "DISABLE_SVC_FLAP_DETECTION"))
    command_type = CMD_DISABLE_SVC_FLAP_DETECTION;

  else if (!strcmp(command_id, "START_OBSESSING_OVER_SVC"))
    command_type = CMD_START_OBSESSING_OVER_SVC;
  else if (!strcmp(command_id, "STOP_OBSESSING_OVER_SVC"))
    command_type = CMD_STOP_OBSESSING_OVER_SVC;

  else if (!strcmp(command_id, "CHANGE_SVC_EVENT_HANDLER"))
    command_type = CMD_CHANGE_SVC_EVENT_HANDLER;
  else if (!strcmp(command_id, "CHANGE_SVC_CHECK_COMMAND"))
    command_type = CMD_CHANGE_SVC_CHECK_COMMAND;

  else if (!strcmp(command_id, "CHANGE_NORMAL_SVC_CHECK_INTERVAL"))
    command_type = CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL;
  else if (!strcmp(command_id, "CHANGE_RETRY_SVC_CHECK_INTERVAL"))
    command_type = CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL;

  else if (!strcmp(command_id, "CHANGE_MAX_SVC_CHECK_ATTEMPTS"))
    command_type = CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS;

  else if (!strcmp(command_id, "CHANGE_SVC_CHECK_TIMEPERIOD"))
    command_type = CMD_CHANGE_SVC_CHECK_TIMEPERIOD;

  else if (!strcmp(command_id, "CHANGE_CUSTOM_SVC_VAR"))
    command_type = CMD_CHANGE_CUSTOM_SVC_VAR;

  else if (!strcmp(command_id, "CHANGE_SVC_MODATTR"))
    command_type = CMD_CHANGE_SVC_MODATTR;

  /***************************************/
  /**** SERVICEGROUP-RELATED COMMANDS ****/
  /***************************************/

  else if (!strcmp(command_id, "ENABLE_SERVICEGROUP_HOST_CHECKS"))
    command_type = CMD_ENABLE_SERVICEGROUP_HOST_CHECKS;
  else if (!strcmp(command_id, "DISABLE_SERVICEGROUP_HOST_CHECKS"))
    command_type = CMD_DISABLE_SERVICEGROUP_HOST_CHECKS;

  else if (!strcmp(command_id, "ENABLE_SERVICEGROUP_SVC_CHECKS"))
    command_type = CMD_ENABLE_SERVICEGROUP_SVC_CHECKS;
  else if (!strcmp(command_id, "DISABLE_SERVICEGROUP_SVC_CHECKS"))
    command_type = CMD_DISABLE_SERVICEGROUP_SVC_CHECKS;

  /**************************/
  /****** MISC COMMANDS *****/
  /**************************/

  else if (!strcmp(command_id, "PROCESS_FILE"))
    command_type = CMD_PROCESS_FILE;

  /****************************/
  /****** CUSTOM COMMANDS *****/
  /****************************/

  else if (command_id[0] == '_')
    command_type = CMD_CUSTOM_COMMAND;

  /**** UNKNOWN COMMAND* ***/
  else {
    /* log the bad external command */
    logger(log_external_command | log_runtime_warning, basic)
      << "Warning: Unrecognized external command -> "
      << command_id << ";" << args;

    /* free memory */
    delete[] command_id;
    delete[] args;

    return (ERROR);
  }

  /* update statistics for external commands */
  update_check_stats(EXTERNAL_COMMAND_STATS, time(NULL));

  /* log the external command */
  std::ostringstream oss;
  oss << "EXTERNAL COMMAND: " << command_id << ';' << args << std::endl;

  if (command_type == CMD_PROCESS_SERVICE_CHECK_RESULT
      || command_type == CMD_PROCESS_HOST_CHECK_RESULT) {
    /* passive checks are logged in checks.c as well, as some my bypass external commands by getting dropped in checkresults dir */
    if (config->log_passive_checks())
      logger(log_passive_check, basic) << oss.str();
  }
  else {
    if (config->log_external_commands())
      logger(log_external_command, basic) << oss.str();
  }

  /* send data to event broker */
  broker_external_command(NEBTYPE_EXTERNALCOMMAND_START,
                          NEBFLAG_NONE,
                          NEBATTR_NONE,
                          command_type,
                          entry_time,
                          command_id,
                          args,
                          NULL);

  /* process the command */
  process_external_command2(command_type, entry_time, args);

  /* send data to event broker */
  broker_external_command(NEBTYPE_EXTERNALCOMMAND_END,
                          NEBFLAG_NONE,
                          NEBATTR_NONE,
                          command_type,
                          entry_time,
                          command_id,
                          args,
                          NULL);

  /* free memory */
  delete[] command_id;
  delete[] args;

  return (OK);
}

int process_external_command2(int cmd,
                              time_t entry_time,
                              char* args) {
  logger(dbg_functions, basic)
    << "process_external_command2()";
  logger(dbg_external_command, more)
    << "External Command Type: " << cmd;
  logger(dbg_external_command, more)
    << "Command Entry Time: " << (unsigned long)entry_time;
  logger(dbg_external_command, more)
    << "Command Arguments: " << (args == NULL ? "" : args);

  /* how shall we execute the command? */
  switch (cmd) {
    /***************************/
    /***** SYSTEM COMMANDS *****/
    /***************************/

  case CMD_SHUTDOWN_PROCESS:
  case CMD_RESTART_PROCESS:
    cmd_signal_process(cmd, args);
    break;

  case CMD_SAVE_STATE_INFORMATION:
    retention::dump::save(config->state_retention_file());
    break;

  case CMD_READ_STATE_INFORMATION:
    {
      retention::state state;
      retention::parser p;
      p.parse(config->state_retention_file(), state);
      retention::applier::state app_state;
      app_state.apply(*config, state);
    }
    break;

  case CMD_START_OBSESSING_OVER_SVC_CHECKS:
    start_obsessing_over_service_checks();
    break;

  case CMD_STOP_OBSESSING_OVER_SVC_CHECKS:
    stop_obsessing_over_service_checks();
    break;

  case CMD_START_OBSESSING_OVER_HOST_CHECKS:
    start_obsessing_over_host_checks();
    break;

  case CMD_STOP_OBSESSING_OVER_HOST_CHECKS:
    stop_obsessing_over_host_checks();
    break;

  case CMD_ENABLE_EVENT_HANDLERS:
    start_using_event_handlers();
    break;

  case CMD_DISABLE_EVENT_HANDLERS:
    stop_using_event_handlers();
    break;

  case CMD_ENABLE_FLAP_DETECTION:
    enable_flap_detection_routines();
    break;

  case CMD_DISABLE_FLAP_DETECTION:
    disable_flap_detection_routines();
    break;

  case CMD_ENABLE_SERVICE_FRESHNESS_CHECKS:
    enable_service_freshness_checks();
    break;

  case CMD_DISABLE_SERVICE_FRESHNESS_CHECKS:
    disable_service_freshness_checks();
    break;

  case CMD_ENABLE_HOST_FRESHNESS_CHECKS:
    enable_host_freshness_checks();
    break;

  case CMD_DISABLE_HOST_FRESHNESS_CHECKS:
    disable_host_freshness_checks();
    break;

    /***************************/
    /*****  HOST COMMANDS  *****/
    /***************************/

  case CMD_ENABLE_HOST_CHECK:
  case CMD_DISABLE_HOST_CHECK:
  case CMD_ENABLE_HOST_SVC_CHECKS:
  case CMD_DISABLE_HOST_SVC_CHECKS:
  case CMD_ENABLE_HOST_FLAP_DETECTION:
  case CMD_DISABLE_HOST_FLAP_DETECTION:
  case CMD_ENABLE_HOST_EVENT_HANDLER:
  case CMD_DISABLE_HOST_EVENT_HANDLER:
  case CMD_START_OBSESSING_OVER_HOST:
  case CMD_STOP_OBSESSING_OVER_HOST:
    process_host_command(cmd, entry_time, args);
    break;

    /*****************************/
    /***** HOSTGROUP COMMANDS ****/
    /*****************************/

  case CMD_ENABLE_HOSTGROUP_HOST_CHECKS:
  case CMD_DISABLE_HOSTGROUP_HOST_CHECKS:
  case CMD_ENABLE_HOSTGROUP_SVC_CHECKS:
  case CMD_DISABLE_HOSTGROUP_SVC_CHECKS:
    process_hostgroup_command(cmd, entry_time, args);
    break;

    /***************************/
    /***** SERVICE COMMANDS ****/
    /***************************/

  case CMD_ENABLE_SVC_CHECK:
  case CMD_DISABLE_SVC_CHECK:
  case CMD_ENABLE_SVC_FLAP_DETECTION:
  case CMD_DISABLE_SVC_FLAP_DETECTION:
  case CMD_ENABLE_SVC_EVENT_HANDLER:
  case CMD_DISABLE_SVC_EVENT_HANDLER:
  case CMD_START_OBSESSING_OVER_SVC:
  case CMD_STOP_OBSESSING_OVER_SVC:
    process_service_command(cmd, entry_time, args);
    break;

    /********************************/
    /***** SERVICEGROUP COMMANDS ****/
    /********************************/

  case CMD_ENABLE_SERVICEGROUP_HOST_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_HOST_CHECKS:
  case CMD_ENABLE_SERVICEGROUP_SVC_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_SVC_CHECKS:
    process_servicegroup_command(cmd, entry_time, args);
    break;

    /***************************/
    /**** UNSORTED COMMANDS ****/
    /***************************/

  case CMD_SCHEDULE_SVC_CHECK:
  case CMD_SCHEDULE_FORCED_SVC_CHECK:
    cmd_schedule_check(cmd, args);
    break;

  case CMD_SCHEDULE_HOST_SVC_CHECKS:
  case CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS:
    cmd_schedule_check(cmd, args);
    break;

  case CMD_PROCESS_SERVICE_CHECK_RESULT:
    cmd_process_service_check_result(cmd, entry_time, args);
    break;

  case CMD_PROCESS_HOST_CHECK_RESULT:
    cmd_process_host_check_result(cmd, entry_time, args);
    break;

  case CMD_SCHEDULE_HOST_CHECK:
  case CMD_SCHEDULE_FORCED_HOST_CHECK:
    cmd_schedule_check(cmd, args);
    break;

  case CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER:
  case CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER:
  case CMD_CHANGE_HOST_EVENT_HANDLER:
  case CMD_CHANGE_SVC_EVENT_HANDLER:
  case CMD_CHANGE_HOST_CHECK_COMMAND:
  case CMD_CHANGE_SVC_CHECK_COMMAND:
  case CMD_CHANGE_HOST_CHECK_TIMEPERIOD:
  case CMD_CHANGE_SVC_CHECK_TIMEPERIOD:
    cmd_change_object_char_var(cmd, args);
    break;

  case CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL:
  case CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL:
  case CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL:
  case CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL:
  case CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS:
  case CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS:
  case CMD_CHANGE_HOST_MODATTR:
  case CMD_CHANGE_SVC_MODATTR:
    cmd_change_object_int_var(cmd, args);
    break;

  case CMD_CHANGE_CUSTOM_HOST_VAR:
  case CMD_CHANGE_CUSTOM_SVC_VAR:
    cmd_change_object_custom_var(cmd, args);
    break;

    /***********************/
    /**** MISC COMMANDS ****/
    /***********************/

  case CMD_PROCESS_FILE:
    cmd_process_external_commands_from_file(cmd, args);
    break;

    /*************************/
    /**** CUSTOM COMMANDS ****/
    /*************************/

  case CMD_CUSTOM_COMMAND:
    /* custom commands aren't handled internally by Centreon Engine, but may be by NEB modules */
    break;

  default:
    return (ERROR);
  }

  return (OK);
}

int process_hostgroup_command(int cmd,
                              time_t entry_time,
                              char* args) {
  char* hostgroup_name = NULL;
  hostgroup* temp_hostgroup = NULL;
  hostsmember* temp_member = NULL;
  host* temp_host = NULL;
  service* temp_service = NULL;
  servicesmember* temp_servicesmember = NULL;

  (void)entry_time;

  /* get the hostgroup name */
  if ((hostgroup_name = my_strtok(args, ";")) == NULL)
    return (ERROR);

  /* find the hostgroup */
  if ((temp_hostgroup = find_hostgroup(hostgroup_name)) == NULL)
    return (ERROR);

  /* loop through all hosts in the hostgroup */
  for (temp_member = temp_hostgroup->members;
       temp_member != NULL;
       temp_member = temp_member->next) {

    if ((temp_host = (host*)temp_member->host_ptr) == NULL)
      continue;

    switch (cmd) {

    case CMD_ENABLE_HOSTGROUP_HOST_CHECKS:
      enable_host_checks(temp_host);
      break;

    case CMD_DISABLE_HOSTGROUP_HOST_CHECKS:
      disable_host_checks(temp_host);
      break;

    default:

      /* loop through all services on the host */
      for (temp_servicesmember = temp_host->services;
           temp_servicesmember != NULL;
           temp_servicesmember = temp_servicesmember->next) {
        if ((temp_service = temp_servicesmember->service_ptr) == NULL)
          continue;

        switch (cmd) {

        case CMD_ENABLE_HOSTGROUP_SVC_CHECKS:
          enable_service_checks(temp_service);
          break;

        case CMD_DISABLE_HOSTGROUP_SVC_CHECKS:
          disable_service_checks(temp_service);
          break;

        default:
          break;
        }
      }
      break;
    }
  }
  return (OK);
}

int process_host_command(int cmd,
                         time_t entry_time,
                         char* args) {
  char* host_name = NULL;
  host* temp_host = NULL;
  service* temp_service = NULL;
  servicesmember* temp_servicesmember = NULL;

  (void)entry_time;

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == NULL)
    return (ERROR);

  /* find the host */
  if ((temp_host = find_host(host_name)) == NULL)
    return (ERROR);

  switch (cmd) {
  case CMD_ENABLE_HOST_SVC_CHECKS:
  case CMD_DISABLE_HOST_SVC_CHECKS:
    for (temp_servicesmember = temp_host->services;
         temp_servicesmember != NULL;
         temp_servicesmember = temp_servicesmember->next) {
      if ((temp_service = temp_servicesmember->service_ptr) == NULL)
        continue;
      if (cmd == CMD_ENABLE_HOST_SVC_CHECKS)
        enable_service_checks(temp_service);
      else
        disable_service_checks(temp_service);
    }
    break;

  case CMD_ENABLE_HOST_CHECK:
    enable_host_checks(temp_host);
    break;

  case CMD_DISABLE_HOST_CHECK:
    disable_host_checks(temp_host);
    break;

  case CMD_ENABLE_HOST_EVENT_HANDLER:
    enable_host_event_handler(temp_host);
    break;

  case CMD_DISABLE_HOST_EVENT_HANDLER:
    disable_host_event_handler(temp_host);
    break;

  case CMD_ENABLE_HOST_FLAP_DETECTION:
    enable_host_flap_detection(temp_host);
    break;

  case CMD_DISABLE_HOST_FLAP_DETECTION:
    disable_host_flap_detection(temp_host);
    break;

  case CMD_START_OBSESSING_OVER_HOST:
    start_obsessing_over_host(temp_host);
    break;

  case CMD_STOP_OBSESSING_OVER_HOST:
    stop_obsessing_over_host(temp_host);
    break;

  default:
    break;
  }

  return (OK);
}

int process_service_command(int cmd,
                            time_t entry_time,
                            char* args) {
  char* host_name = NULL;
  char* svc_description = NULL;
  service* temp_service = NULL;

  (void)entry_time;

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == NULL)
    return (ERROR);

  /* get the service description */
  if ((svc_description = my_strtok(NULL, ";")) == NULL)
    return (ERROR);

  /* find the service */
  if ((temp_service = find_service(host_name, svc_description)) == NULL)
    return (ERROR);

  switch (cmd) {
  case CMD_ENABLE_SVC_CHECK:
    enable_service_checks(temp_service);
    break;

  case CMD_DISABLE_SVC_CHECK:
    disable_service_checks(temp_service);
    break;

  case CMD_ENABLE_SVC_EVENT_HANDLER:
    enable_service_event_handler(temp_service);
    break;

  case CMD_DISABLE_SVC_EVENT_HANDLER:
    disable_service_event_handler(temp_service);
    break;

  case CMD_ENABLE_SVC_FLAP_DETECTION:
    enable_service_flap_detection(temp_service);
    break;

  case CMD_DISABLE_SVC_FLAP_DETECTION:
    disable_service_flap_detection(temp_service);
    break;

  case CMD_START_OBSESSING_OVER_SVC:
    start_obsessing_over_service(temp_service);
    break;

  case CMD_STOP_OBSESSING_OVER_SVC:
    stop_obsessing_over_service(temp_service);
    break;

  default:
    break;
  }

  return (OK);
}

int process_servicegroup_command(int cmd,
                                 time_t entry_time,
                                 char* args) {
  char* servicegroup_name = NULL;
  servicegroup* temp_servicegroup = NULL;
  servicesmember* temp_member = NULL;
  host* temp_host = NULL;
  host* last_host = NULL;
  service* temp_service = NULL;

  (void)entry_time;

  /* get the servicegroup name */
  if ((servicegroup_name = my_strtok(args, ";")) == NULL)
    return (ERROR);

  /* find the servicegroup */
  if ((temp_servicegroup = find_servicegroup(servicegroup_name)) == NULL)
    return (ERROR);

  switch (cmd) {

  case CMD_ENABLE_SERVICEGROUP_SVC_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_SVC_CHECKS:

    /* loop through all servicegroup members */
    for (temp_member = temp_servicegroup->members;
         temp_member != NULL;
         temp_member = temp_member->next) {

      temp_service = find_service(temp_member->host_name, temp_member->service_description);
      if (temp_service == NULL)
        continue;

      switch (cmd) {

      case CMD_ENABLE_SERVICEGROUP_SVC_CHECKS:
        enable_service_checks(temp_service);
        break;

      case CMD_DISABLE_SERVICEGROUP_SVC_CHECKS:
        disable_service_checks(temp_service);
        break;

      default:
        break;
      }
    }
    break;

  case CMD_ENABLE_SERVICEGROUP_HOST_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_HOST_CHECKS:
    /* loop through all hosts that have services belonging to the servicegroup */
    last_host = NULL;
    for (temp_member = temp_servicegroup->members;
         temp_member != NULL;
         temp_member = temp_member->next) {

      if ((temp_host = find_host(temp_member->host_name)) == NULL)
        continue;

      if (temp_host == last_host)
        continue;

      switch (cmd) {

      case CMD_ENABLE_SERVICEGROUP_HOST_CHECKS:
        enable_host_checks(temp_host);
        break;

      case CMD_DISABLE_SERVICEGROUP_HOST_CHECKS:
        disable_host_checks(temp_host);
        break;

      default:
        break;
      }

      last_host = temp_host;
    }
    break;

  default:
    break;
  }
  return (OK);
}
