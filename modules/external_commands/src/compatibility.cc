/*
** Copyright 2002-2006 Ethan Galstad
** Copyright 2011-2012 Merethis
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

#include <stdlib.h>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/external_commands/commands.hh"
#include "com/centreon/engine/external_commands/compatibility.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/sretention.hh"

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
  command_id = my_strdup(temp_ptr + 1);

  /* get the command arguments */
  if ((temp_ptr = my_strtok(NULL, "\n")) == NULL)
    args = my_strdup("");
  else
    args = my_strdup(temp_ptr);

  /* decide what type of command this is... */

  /**************************/
  /**** PROCESS COMMANDS ****/
  /**************************/

  if (!strcmp(command_id, "ENTER_STANDBY_MODE")
      || !strcmp(command_id, "DISABLE_NOTIFICATIONS"))
    command_type = CMD_DISABLE_NOTIFICATIONS;
  else if (!strcmp(command_id, "ENTER_ACTIVE_MODE")
           || !strcmp(command_id, "ENABLE_NOTIFICATIONS"))
    command_type = CMD_ENABLE_NOTIFICATIONS;

  else if (!strcmp(command_id, "SHUTDOWN_PROGRAM")
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

  else if (!strcmp(command_id, "ENABLE_FAILURE_PREDICTION"))
    command_type = CMD_ENABLE_FAILURE_PREDICTION;
  else if (!strcmp(command_id, "DISABLE_FAILURE_PREDICTION"))
    command_type = CMD_DISABLE_FAILURE_PREDICTION;

  else if (!strcmp(command_id, "ENABLE_PERFORMANCE_DATA"))
    command_type = CMD_ENABLE_PERFORMANCE_DATA;
  else if (!strcmp(command_id, "DISABLE_PERFORMANCE_DATA"))
    command_type = CMD_DISABLE_PERFORMANCE_DATA;

  else if (!strcmp(command_id, "START_EXECUTING_HOST_CHECKS"))
    command_type = CMD_START_EXECUTING_HOST_CHECKS;
  else if (!strcmp(command_id, "STOP_EXECUTING_HOST_CHECKS"))
    command_type = CMD_STOP_EXECUTING_HOST_CHECKS;

  else if (!strcmp(command_id, "START_EXECUTING_SVC_CHECKS"))
    command_type = CMD_START_EXECUTING_SVC_CHECKS;
  else if (!strcmp(command_id, "STOP_EXECUTING_SVC_CHECKS"))
    command_type = CMD_STOP_EXECUTING_SVC_CHECKS;

  else if (!strcmp(command_id, "START_ACCEPTING_PASSIVE_HOST_CHECKS"))
    command_type = CMD_START_ACCEPTING_PASSIVE_HOST_CHECKS;
  else if (!strcmp(command_id, "STOP_ACCEPTING_PASSIVE_HOST_CHECKS"))
    command_type = CMD_STOP_ACCEPTING_PASSIVE_HOST_CHECKS;

  else if (!strcmp(command_id, "START_ACCEPTING_PASSIVE_SVC_CHECKS"))
    command_type = CMD_START_ACCEPTING_PASSIVE_SVC_CHECKS;
  else if (!strcmp(command_id, "STOP_ACCEPTING_PASSIVE_SVC_CHECKS"))
    command_type = CMD_STOP_ACCEPTING_PASSIVE_SVC_CHECKS;

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

  else if (!strcmp(command_id, "ADD_HOST_COMMENT"))
    command_type = CMD_ADD_HOST_COMMENT;
  else if (!strcmp(command_id, "DEL_HOST_COMMENT"))
    command_type = CMD_DEL_HOST_COMMENT;
  else if (!strcmp(command_id, "DEL_ALL_HOST_COMMENTS"))
    command_type = CMD_DEL_ALL_HOST_COMMENTS;

  else if (!strcmp(command_id, "DELAY_HOST_NOTIFICATION"))
    command_type = CMD_DELAY_HOST_NOTIFICATION;

  else if (!strcmp(command_id, "ENABLE_HOST_NOTIFICATIONS"))
    command_type = CMD_ENABLE_HOST_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_HOST_NOTIFICATIONS"))
    command_type = CMD_DISABLE_HOST_NOTIFICATIONS;

  else if (!strcmp(command_id, "ENABLE_ALL_NOTIFICATIONS_BEYOND_HOST"))
    command_type = CMD_ENABLE_ALL_NOTIFICATIONS_BEYOND_HOST;
  else if (!strcmp(command_id, "DISABLE_ALL_NOTIFICATIONS_BEYOND_HOST"))
    command_type = CMD_DISABLE_ALL_NOTIFICATIONS_BEYOND_HOST;

  else if (!strcmp(command_id, "ENABLE_HOST_AND_CHILD_NOTIFICATIONS"))
    command_type = CMD_ENABLE_HOST_AND_CHILD_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_HOST_AND_CHILD_NOTIFICATIONS"))
    command_type = CMD_DISABLE_HOST_AND_CHILD_NOTIFICATIONS;

  else if (!strcmp(command_id, "ENABLE_HOST_SVC_NOTIFICATIONS"))
    command_type = CMD_ENABLE_HOST_SVC_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_HOST_SVC_NOTIFICATIONS"))
    command_type = CMD_DISABLE_HOST_SVC_NOTIFICATIONS;

  else if (!strcmp(command_id, "ENABLE_HOST_SVC_CHECKS"))
    command_type = CMD_ENABLE_HOST_SVC_CHECKS;
  else if (!strcmp(command_id, "DISABLE_HOST_SVC_CHECKS"))
    command_type = CMD_DISABLE_HOST_SVC_CHECKS;

  else if (!strcmp(command_id, "ENABLE_PASSIVE_HOST_CHECKS"))
    command_type = CMD_ENABLE_PASSIVE_HOST_CHECKS;
  else if (!strcmp(command_id, "DISABLE_PASSIVE_HOST_CHECKS"))
    command_type = CMD_DISABLE_PASSIVE_HOST_CHECKS;

  else if (!strcmp(command_id, "SCHEDULE_HOST_SVC_CHECKS"))
    command_type = CMD_SCHEDULE_HOST_SVC_CHECKS;
  else if (!strcmp(command_id, "SCHEDULE_FORCED_HOST_SVC_CHECKS"))
    command_type = CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS;

  else if (!strcmp(command_id, "ACKNOWLEDGE_HOST_PROBLEM"))
    command_type = CMD_ACKNOWLEDGE_HOST_PROBLEM;
  else if (!strcmp(command_id, "REMOVE_HOST_ACKNOWLEDGEMENT"))
    command_type = CMD_REMOVE_HOST_ACKNOWLEDGEMENT;

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

  else if (!strcmp(command_id, "SCHEDULE_HOST_DOWNTIME"))
    command_type = CMD_SCHEDULE_HOST_DOWNTIME;
  else if (!strcmp(command_id, "SCHEDULE_HOST_SVC_DOWNTIME"))
    command_type = CMD_SCHEDULE_HOST_SVC_DOWNTIME;
  else if (!strcmp(command_id, "DEL_HOST_DOWNTIME"))
    command_type = CMD_DEL_HOST_DOWNTIME;
  else if (!strcmp(command_id, "DEL_DOWNTIME_BY_HOST_NAME"))
    command_type = CMD_DEL_DOWNTIME_BY_HOST_NAME;
  else if (!strcmp(command_id, "DEL_DOWNTIME_BY_HOSTGROUP_NAME"))
    command_type = CMD_DEL_DOWNTIME_BY_HOSTGROUP_NAME;
  else if (!strcmp(command_id, "DEL_DOWNTIME_BY_START_TIME_COMMENT"))
    command_type = CMD_DEL_DOWNTIME_BY_START_TIME_COMMENT;

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

  else if (!strcmp(command_id, "SCHEDULE_AND_PROPAGATE_TRIGGERED_HOST_DOWNTIME"))
    command_type = CMD_SCHEDULE_AND_PROPAGATE_TRIGGERED_HOST_DOWNTIME;

  else if (!strcmp(command_id, "SCHEDULE_AND_PROPAGATE_HOST_DOWNTIME"))
    command_type = CMD_SCHEDULE_AND_PROPAGATE_HOST_DOWNTIME;

  else if (!strcmp(command_id, "SET_HOST_NOTIFICATION_NUMBER"))
    command_type = CMD_SET_HOST_NOTIFICATION_NUMBER;

  else if (!strcmp(command_id, "CHANGE_HOST_CHECK_TIMEPERIOD"))
    command_type = CMD_CHANGE_HOST_CHECK_TIMEPERIOD;

  else if (!strcmp(command_id, "CHANGE_CUSTOM_HOST_VAR"))
    command_type = CMD_CHANGE_CUSTOM_HOST_VAR;

  else if (!strcmp(command_id, "SEND_CUSTOM_HOST_NOTIFICATION"))
    command_type = CMD_SEND_CUSTOM_HOST_NOTIFICATION;

  else if (!strcmp(command_id, "CHANGE_HOST_NOTIFICATION_TIMEPERIOD"))
    command_type = CMD_CHANGE_HOST_NOTIFICATION_TIMEPERIOD;

  else if (!strcmp(command_id, "CHANGE_HOST_MODATTR"))
    command_type = CMD_CHANGE_HOST_MODATTR;

  /************************************/
  /**** HOSTGROUP-RELATED COMMANDS ****/
  /************************************/

  else if (!strcmp(command_id, "ENABLE_HOSTGROUP_HOST_NOTIFICATIONS"))
    command_type = CMD_ENABLE_HOSTGROUP_HOST_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_HOSTGROUP_HOST_NOTIFICATIONS"))
    command_type = CMD_DISABLE_HOSTGROUP_HOST_NOTIFICATIONS;

  else if (!strcmp(command_id, "ENABLE_HOSTGROUP_SVC_NOTIFICATIONS"))
    command_type = CMD_ENABLE_HOSTGROUP_SVC_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_HOSTGROUP_SVC_NOTIFICATIONS"))
    command_type = CMD_DISABLE_HOSTGROUP_SVC_NOTIFICATIONS;

  else if (!strcmp(command_id, "ENABLE_HOSTGROUP_HOST_CHECKS"))
    command_type = CMD_ENABLE_HOSTGROUP_HOST_CHECKS;
  else if (!strcmp(command_id, "DISABLE_HOSTGROUP_HOST_CHECKS"))
    command_type = CMD_DISABLE_HOSTGROUP_HOST_CHECKS;

  else if (!strcmp(command_id, "ENABLE_HOSTGROUP_PASSIVE_HOST_CHECKS"))
    command_type = CMD_ENABLE_HOSTGROUP_PASSIVE_HOST_CHECKS;
  else if (!strcmp(command_id, "DISABLE_HOSTGROUP_PASSIVE_HOST_CHECKS"))
    command_type = CMD_DISABLE_HOSTGROUP_PASSIVE_HOST_CHECKS;

  else if (!strcmp(command_id, "ENABLE_HOSTGROUP_SVC_CHECKS"))
    command_type = CMD_ENABLE_HOSTGROUP_SVC_CHECKS;
  else if (!strcmp(command_id, "DISABLE_HOSTGROUP_SVC_CHECKS"))
    command_type = CMD_DISABLE_HOSTGROUP_SVC_CHECKS;

  else if (!strcmp(command_id, "ENABLE_HOSTGROUP_PASSIVE_SVC_CHECKS"))
    command_type = CMD_ENABLE_HOSTGROUP_PASSIVE_SVC_CHECKS;
  else if (!strcmp(command_id, "DISABLE_HOSTGROUP_PASSIVE_SVC_CHECKS"))
    command_type = CMD_DISABLE_HOSTGROUP_PASSIVE_SVC_CHECKS;

  else if (!strcmp(command_id, "SCHEDULE_HOSTGROUP_HOST_DOWNTIME"))
    command_type = CMD_SCHEDULE_HOSTGROUP_HOST_DOWNTIME;
  else if (!strcmp(command_id, "SCHEDULE_HOSTGROUP_SVC_DOWNTIME"))
    command_type = CMD_SCHEDULE_HOSTGROUP_SVC_DOWNTIME;

  /**********************************/
  /**** SERVICE-RELATED COMMANDS ****/
  /**********************************/

  else if (!strcmp(command_id, "ADD_SVC_COMMENT"))
    command_type = CMD_ADD_SVC_COMMENT;
  else if (!strcmp(command_id, "DEL_SVC_COMMENT"))
    command_type = CMD_DEL_SVC_COMMENT;
  else if (!strcmp(command_id, "DEL_ALL_SVC_COMMENTS"))
    command_type = CMD_DEL_ALL_SVC_COMMENTS;

  else if (!strcmp(command_id, "SCHEDULE_SVC_CHECK"))
    command_type = CMD_SCHEDULE_SVC_CHECK;
  else if (!strcmp(command_id, "SCHEDULE_FORCED_SVC_CHECK"))
    command_type = CMD_SCHEDULE_FORCED_SVC_CHECK;

  else if (!strcmp(command_id, "ENABLE_SVC_CHECK"))
    command_type = CMD_ENABLE_SVC_CHECK;
  else if (!strcmp(command_id, "DISABLE_SVC_CHECK"))
    command_type = CMD_DISABLE_SVC_CHECK;

  else if (!strcmp(command_id, "ENABLE_PASSIVE_SVC_CHECKS"))
    command_type = CMD_ENABLE_PASSIVE_SVC_CHECKS;
  else if (!strcmp(command_id, "DISABLE_PASSIVE_SVC_CHECKS"))
    command_type = CMD_DISABLE_PASSIVE_SVC_CHECKS;

  else if (!strcmp(command_id, "DELAY_SVC_NOTIFICATION"))
    command_type = CMD_DELAY_SVC_NOTIFICATION;
  else if (!strcmp(command_id, "ENABLE_SVC_NOTIFICATIONS"))
    command_type = CMD_ENABLE_SVC_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_SVC_NOTIFICATIONS"))
    command_type = CMD_DISABLE_SVC_NOTIFICATIONS;

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

  else if (!strcmp(command_id, "SCHEDULE_SVC_DOWNTIME"))
    command_type = CMD_SCHEDULE_SVC_DOWNTIME;
  else if (!strcmp(command_id, "DEL_SVC_DOWNTIME"))
    command_type = CMD_DEL_SVC_DOWNTIME;

  else if (!strcmp(command_id, "ACKNOWLEDGE_SVC_PROBLEM"))
    command_type = CMD_ACKNOWLEDGE_SVC_PROBLEM;
  else if (!strcmp(command_id, "REMOVE_SVC_ACKNOWLEDGEMENT"))
    command_type = CMD_REMOVE_SVC_ACKNOWLEDGEMENT;

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

  else if (!strcmp(command_id, "SET_SVC_NOTIFICATION_NUMBER"))
    command_type = CMD_SET_SVC_NOTIFICATION_NUMBER;

  else if (!strcmp(command_id, "CHANGE_SVC_CHECK_TIMEPERIOD"))
    command_type = CMD_CHANGE_SVC_CHECK_TIMEPERIOD;

  else if (!strcmp(command_id, "CHANGE_CUSTOM_SVC_VAR"))
    command_type = CMD_CHANGE_CUSTOM_SVC_VAR;

  else if (!strcmp(command_id, "CHANGE_CUSTOM_CONTACT_VAR"))
    command_type = CMD_CHANGE_CUSTOM_CONTACT_VAR;

  else if (!strcmp(command_id, "SEND_CUSTOM_SVC_NOTIFICATION"))
    command_type = CMD_SEND_CUSTOM_SVC_NOTIFICATION;

  else if (!strcmp(command_id, "CHANGE_SVC_NOTIFICATION_TIMEPERIOD"))
    command_type = CMD_CHANGE_SVC_NOTIFICATION_TIMEPERIOD;

  else if (!strcmp(command_id, "CHANGE_SVC_MODATTR"))
    command_type = CMD_CHANGE_SVC_MODATTR;

  /***************************************/
  /**** SERVICEGROUP-RELATED COMMANDS ****/
  /***************************************/

  else if (!strcmp(command_id, "ENABLE_SERVICEGROUP_HOST_NOTIFICATIONS"))
    command_type = CMD_ENABLE_SERVICEGROUP_HOST_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_SERVICEGROUP_HOST_NOTIFICATIONS"))
    command_type = CMD_DISABLE_SERVICEGROUP_HOST_NOTIFICATIONS;

  else if (!strcmp(command_id, "ENABLE_SERVICEGROUP_SVC_NOTIFICATIONS"))
    command_type = CMD_ENABLE_SERVICEGROUP_SVC_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_SERVICEGROUP_SVC_NOTIFICATIONS"))
    command_type = CMD_DISABLE_SERVICEGROUP_SVC_NOTIFICATIONS;

  else if (!strcmp(command_id, "ENABLE_SERVICEGROUP_HOST_CHECKS"))
    command_type = CMD_ENABLE_SERVICEGROUP_HOST_CHECKS;
  else if (!strcmp(command_id, "DISABLE_SERVICEGROUP_HOST_CHECKS"))
    command_type = CMD_DISABLE_SERVICEGROUP_HOST_CHECKS;

  else if (!strcmp(command_id, "ENABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS"))
    command_type = CMD_ENABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS;
  else if (!strcmp(command_id, "DISABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS"))
    command_type = CMD_DISABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS;

  else if (!strcmp(command_id, "ENABLE_SERVICEGROUP_SVC_CHECKS"))
    command_type = CMD_ENABLE_SERVICEGROUP_SVC_CHECKS;
  else if (!strcmp(command_id, "DISABLE_SERVICEGROUP_SVC_CHECKS"))
    command_type = CMD_DISABLE_SERVICEGROUP_SVC_CHECKS;

  else if (!strcmp(command_id, "ENABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS"))
    command_type = CMD_ENABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS;
  else if (!strcmp(command_id, "DISABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS"))
    command_type = CMD_DISABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS;

  else if (!strcmp(command_id, "SCHEDULE_SERVICEGROUP_HOST_DOWNTIME"))
    command_type = CMD_SCHEDULE_SERVICEGROUP_HOST_DOWNTIME;
  else if (!strcmp(command_id, "SCHEDULE_SERVICEGROUP_SVC_DOWNTIME"))
    command_type = CMD_SCHEDULE_SERVICEGROUP_SVC_DOWNTIME;

  /**********************************/
  /**** CONTACT-RELATED COMMANDS ****/
  /**********************************/

  else if (!strcmp(command_id, "ENABLE_CONTACT_HOST_NOTIFICATIONS"))
    command_type = CMD_ENABLE_CONTACT_HOST_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_CONTACT_HOST_NOTIFICATIONS"))
    command_type = CMD_DISABLE_CONTACT_HOST_NOTIFICATIONS;

  else if (!strcmp(command_id, "ENABLE_CONTACT_SVC_NOTIFICATIONS"))
    command_type = CMD_ENABLE_CONTACT_SVC_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_CONTACT_SVC_NOTIFICATIONS"))
    command_type = CMD_DISABLE_CONTACT_SVC_NOTIFICATIONS;

  else if (!strcmp(command_id, "CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD"))
    command_type = CMD_CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD;

  else if (!strcmp(command_id, "CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD"))
    command_type = CMD_CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD;

  else if (!strcmp(command_id, "CHANGE_CONTACT_MODATTR"))
    command_type = CMD_CHANGE_CONTACT_MODATTR;
  else if (!strcmp(command_id, "CHANGE_CONTACT_MODHATTR"))
    command_type = CMD_CHANGE_CONTACT_MODHATTR;
  else if (!strcmp(command_id, "CHANGE_CONTACT_MODSATTR"))
    command_type = CMD_CHANGE_CONTACT_MODSATTR;

  /***************************************/
  /**** CONTACTGROUP-RELATED COMMANDS ****/
  /***************************************/

  else if (!strcmp(command_id, "ENABLE_CONTACTGROUP_HOST_NOTIFICATIONS"))
    command_type = CMD_ENABLE_CONTACTGROUP_HOST_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_CONTACTGROUP_HOST_NOTIFICATIONS"))
    command_type = CMD_DISABLE_CONTACTGROUP_HOST_NOTIFICATIONS;

  else if (!strcmp(command_id, "ENABLE_CONTACTGROUP_SVC_NOTIFICATIONS"))
    command_type = CMD_ENABLE_CONTACTGROUP_SVC_NOTIFICATIONS;
  else if (!strcmp(command_id, "DISABLE_CONTACTGROUP_SVC_NOTIFICATIONS"))
    command_type = CMD_DISABLE_CONTACTGROUP_SVC_NOTIFICATIONS;

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
    if (config.get_log_passive_checks() == true)
      logger(log_passive_check, basic) << oss.str();
  }
  else {
    if (config.get_log_external_commands() == true)
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
    save_state_information(FALSE);
    break;

  case CMD_READ_STATE_INFORMATION:
    read_initial_state_information();
    break;

  case CMD_ENABLE_NOTIFICATIONS:
    enable_all_notifications();
    break;

  case CMD_DISABLE_NOTIFICATIONS:
    disable_all_notifications();
    break;

  case CMD_START_EXECUTING_SVC_CHECKS:
    start_executing_service_checks();
    break;

  case CMD_STOP_EXECUTING_SVC_CHECKS:
    stop_executing_service_checks();
    break;

  case CMD_START_ACCEPTING_PASSIVE_SVC_CHECKS:
    start_accepting_passive_service_checks();
    break;

  case CMD_STOP_ACCEPTING_PASSIVE_SVC_CHECKS:
    stop_accepting_passive_service_checks();
    break;

  case CMD_START_OBSESSING_OVER_SVC_CHECKS:
    start_obsessing_over_service_checks();
    break;

  case CMD_STOP_OBSESSING_OVER_SVC_CHECKS:
    stop_obsessing_over_service_checks();
    break;

  case CMD_START_EXECUTING_HOST_CHECKS:
    start_executing_host_checks();
    break;

  case CMD_STOP_EXECUTING_HOST_CHECKS:
    stop_executing_host_checks();
    break;

  case CMD_START_ACCEPTING_PASSIVE_HOST_CHECKS:
    start_accepting_passive_host_checks();
    break;

  case CMD_STOP_ACCEPTING_PASSIVE_HOST_CHECKS:
    stop_accepting_passive_host_checks();
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

  case CMD_ENABLE_FAILURE_PREDICTION:
    enable_all_failure_prediction();
    break;

  case CMD_DISABLE_FAILURE_PREDICTION:
    disable_all_failure_prediction();
    break;

  case CMD_ENABLE_PERFORMANCE_DATA:
    enable_performance_data();
    break;

  case CMD_DISABLE_PERFORMANCE_DATA:
    disable_performance_data();
    break;

    /***************************/
    /*****  HOST COMMANDS  *****/
    /***************************/

  case CMD_ENABLE_HOST_CHECK:
  case CMD_DISABLE_HOST_CHECK:
  case CMD_ENABLE_PASSIVE_HOST_CHECKS:
  case CMD_DISABLE_PASSIVE_HOST_CHECKS:
  case CMD_ENABLE_HOST_SVC_CHECKS:
  case CMD_DISABLE_HOST_SVC_CHECKS:
  case CMD_ENABLE_HOST_NOTIFICATIONS:
  case CMD_DISABLE_HOST_NOTIFICATIONS:
  case CMD_ENABLE_ALL_NOTIFICATIONS_BEYOND_HOST:
  case CMD_DISABLE_ALL_NOTIFICATIONS_BEYOND_HOST:
  case CMD_ENABLE_HOST_AND_CHILD_NOTIFICATIONS:
  case CMD_DISABLE_HOST_AND_CHILD_NOTIFICATIONS:
  case CMD_ENABLE_HOST_SVC_NOTIFICATIONS:
  case CMD_DISABLE_HOST_SVC_NOTIFICATIONS:
  case CMD_ENABLE_HOST_FLAP_DETECTION:
  case CMD_DISABLE_HOST_FLAP_DETECTION:
  case CMD_ENABLE_HOST_EVENT_HANDLER:
  case CMD_DISABLE_HOST_EVENT_HANDLER:
  case CMD_START_OBSESSING_OVER_HOST:
  case CMD_STOP_OBSESSING_OVER_HOST:
  case CMD_SET_HOST_NOTIFICATION_NUMBER:
  case CMD_SEND_CUSTOM_HOST_NOTIFICATION:
    process_host_command(cmd, entry_time, args);
    break;

    /*****************************/
    /***** HOSTGROUP COMMANDS ****/
    /*****************************/

  case CMD_ENABLE_HOSTGROUP_HOST_NOTIFICATIONS:
  case CMD_DISABLE_HOSTGROUP_HOST_NOTIFICATIONS:
  case CMD_ENABLE_HOSTGROUP_SVC_NOTIFICATIONS:
  case CMD_DISABLE_HOSTGROUP_SVC_NOTIFICATIONS:
  case CMD_ENABLE_HOSTGROUP_HOST_CHECKS:
  case CMD_DISABLE_HOSTGROUP_HOST_CHECKS:
  case CMD_ENABLE_HOSTGROUP_PASSIVE_HOST_CHECKS:
  case CMD_DISABLE_HOSTGROUP_PASSIVE_HOST_CHECKS:
  case CMD_ENABLE_HOSTGROUP_SVC_CHECKS:
  case CMD_DISABLE_HOSTGROUP_SVC_CHECKS:
  case CMD_ENABLE_HOSTGROUP_PASSIVE_SVC_CHECKS:
  case CMD_DISABLE_HOSTGROUP_PASSIVE_SVC_CHECKS:
    process_hostgroup_command(cmd, entry_time, args);
    break;

    /***************************/
    /***** SERVICE COMMANDS ****/
    /***************************/

  case CMD_ENABLE_SVC_CHECK:
  case CMD_DISABLE_SVC_CHECK:
  case CMD_ENABLE_PASSIVE_SVC_CHECKS:
  case CMD_DISABLE_PASSIVE_SVC_CHECKS:
  case CMD_ENABLE_SVC_NOTIFICATIONS:
  case CMD_DISABLE_SVC_NOTIFICATIONS:
  case CMD_ENABLE_SVC_FLAP_DETECTION:
  case CMD_DISABLE_SVC_FLAP_DETECTION:
  case CMD_ENABLE_SVC_EVENT_HANDLER:
  case CMD_DISABLE_SVC_EVENT_HANDLER:
  case CMD_START_OBSESSING_OVER_SVC:
  case CMD_STOP_OBSESSING_OVER_SVC:
  case CMD_SET_SVC_NOTIFICATION_NUMBER:
  case CMD_SEND_CUSTOM_SVC_NOTIFICATION:
    process_service_command(cmd, entry_time, args);
    break;

    /********************************/
    /***** SERVICEGROUP COMMANDS ****/
    /********************************/

  case CMD_ENABLE_SERVICEGROUP_HOST_NOTIFICATIONS:
  case CMD_DISABLE_SERVICEGROUP_HOST_NOTIFICATIONS:
  case CMD_ENABLE_SERVICEGROUP_SVC_NOTIFICATIONS:
  case CMD_DISABLE_SERVICEGROUP_SVC_NOTIFICATIONS:
  case CMD_ENABLE_SERVICEGROUP_HOST_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_HOST_CHECKS:
  case CMD_ENABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS:
  case CMD_ENABLE_SERVICEGROUP_SVC_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_SVC_CHECKS:
  case CMD_ENABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS:
    process_servicegroup_command(cmd, entry_time, args);
    break;

    /**********************************/
    /**** CONTACT-RELATED COMMANDS ****/
    /**********************************/

  case CMD_ENABLE_CONTACT_HOST_NOTIFICATIONS:
  case CMD_DISABLE_CONTACT_HOST_NOTIFICATIONS:
  case CMD_ENABLE_CONTACT_SVC_NOTIFICATIONS:
  case CMD_DISABLE_CONTACT_SVC_NOTIFICATIONS:
    process_contact_command(cmd, entry_time, args);
    break;

    /***************************************/
    /**** CONTACTGROUP-RELATED COMMANDS ****/
    /***************************************/

  case CMD_ENABLE_CONTACTGROUP_HOST_NOTIFICATIONS:
  case CMD_DISABLE_CONTACTGROUP_HOST_NOTIFICATIONS:
  case CMD_ENABLE_CONTACTGROUP_SVC_NOTIFICATIONS:
  case CMD_DISABLE_CONTACTGROUP_SVC_NOTIFICATIONS:
    process_contactgroup_command(cmd, entry_time, args);
    break;

    /***************************/
    /**** UNSORTED COMMANDS ****/
    /***************************/

  case CMD_ADD_HOST_COMMENT:
  case CMD_ADD_SVC_COMMENT:
    cmd_add_comment(cmd, entry_time, args);
    break;

  case CMD_DEL_HOST_COMMENT:
  case CMD_DEL_SVC_COMMENT:
    cmd_delete_comment(cmd, args);
    break;

  case CMD_DELAY_HOST_NOTIFICATION:
  case CMD_DELAY_SVC_NOTIFICATION:
    cmd_delay_notification(cmd, args);
    break;

  case CMD_SCHEDULE_SVC_CHECK:
  case CMD_SCHEDULE_FORCED_SVC_CHECK:
    cmd_schedule_check(cmd, args);
    break;

  case CMD_SCHEDULE_HOST_SVC_CHECKS:
  case CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS:
    cmd_schedule_check(cmd, args);
    break;

  case CMD_DEL_ALL_HOST_COMMENTS:
  case CMD_DEL_ALL_SVC_COMMENTS:
    cmd_delete_all_comments(cmd, args);
    break;

  case CMD_PROCESS_SERVICE_CHECK_RESULT:
    cmd_process_service_check_result(cmd, entry_time, args);
    break;

  case CMD_PROCESS_HOST_CHECK_RESULT:
    cmd_process_host_check_result(cmd, entry_time, args);
    break;

  case CMD_ACKNOWLEDGE_HOST_PROBLEM:
  case CMD_ACKNOWLEDGE_SVC_PROBLEM:
    cmd_acknowledge_problem(cmd, args);
    break;

  case CMD_REMOVE_HOST_ACKNOWLEDGEMENT:
  case CMD_REMOVE_SVC_ACKNOWLEDGEMENT:
    cmd_remove_acknowledgement(cmd, args);
    break;

  case CMD_SCHEDULE_HOST_DOWNTIME:
  case CMD_SCHEDULE_SVC_DOWNTIME:
  case CMD_SCHEDULE_HOST_SVC_DOWNTIME:
  case CMD_SCHEDULE_HOSTGROUP_HOST_DOWNTIME:
  case CMD_SCHEDULE_HOSTGROUP_SVC_DOWNTIME:
  case CMD_SCHEDULE_SERVICEGROUP_HOST_DOWNTIME:
  case CMD_SCHEDULE_SERVICEGROUP_SVC_DOWNTIME:
  case CMD_SCHEDULE_AND_PROPAGATE_HOST_DOWNTIME:
  case CMD_SCHEDULE_AND_PROPAGATE_TRIGGERED_HOST_DOWNTIME:
    cmd_schedule_downtime(cmd, entry_time, args);
    break;

  case CMD_DEL_HOST_DOWNTIME:
  case CMD_DEL_SVC_DOWNTIME:
    cmd_delete_downtime(cmd, args);
    break;

  case CMD_DEL_DOWNTIME_BY_HOST_NAME:
    cmd_delete_downtime_by_host_name(cmd, args);
    break ;

  case CMD_DEL_DOWNTIME_BY_HOSTGROUP_NAME:
    cmd_delete_downtime_by_hostgroup_name(cmd, args);
    break ;

  case CMD_DEL_DOWNTIME_BY_START_TIME_COMMENT:
    cmd_delete_downtime_by_start_time_comment(cmd, args);
    break ;

  case CMD_CANCEL_ACTIVE_HOST_SVC_DOWNTIME:
  case CMD_CANCEL_PENDING_HOST_SVC_DOWNTIME:
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
  case CMD_CHANGE_HOST_NOTIFICATION_TIMEPERIOD:
  case CMD_CHANGE_SVC_NOTIFICATION_TIMEPERIOD:
  case CMD_CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD:
  case CMD_CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD:
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
  case CMD_CHANGE_CONTACT_MODATTR:
  case CMD_CHANGE_CONTACT_MODHATTR:
  case CMD_CHANGE_CONTACT_MODSATTR:
    cmd_change_object_int_var(cmd, args);
    break;

  case CMD_CHANGE_CUSTOM_HOST_VAR:
  case CMD_CHANGE_CUSTOM_SVC_VAR:
  case CMD_CHANGE_CUSTOM_CONTACT_VAR:
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

    case CMD_ENABLE_HOSTGROUP_HOST_NOTIFICATIONS:
      enable_host_notifications(temp_host);
      break;

    case CMD_DISABLE_HOSTGROUP_HOST_NOTIFICATIONS:
      disable_host_notifications(temp_host);
      break;

    case CMD_ENABLE_HOSTGROUP_HOST_CHECKS:
      enable_host_checks(temp_host);
      break;

    case CMD_DISABLE_HOSTGROUP_HOST_CHECKS:
      disable_host_checks(temp_host);
      break;

    case CMD_ENABLE_HOSTGROUP_PASSIVE_HOST_CHECKS:
      enable_passive_host_checks(temp_host);
      break;

    case CMD_DISABLE_HOSTGROUP_PASSIVE_HOST_CHECKS:
      disable_passive_host_checks(temp_host);
      break;

    default:

      /* loop through all services on the host */
      for (temp_servicesmember = temp_host->services;
           temp_servicesmember != NULL;
           temp_servicesmember = temp_servicesmember->next) {
        if ((temp_service = temp_servicesmember->service_ptr) == NULL)
          continue;

        switch (cmd) {

        case CMD_ENABLE_HOSTGROUP_SVC_NOTIFICATIONS:
          enable_service_notifications(temp_service);
          break;

        case CMD_DISABLE_HOSTGROUP_SVC_NOTIFICATIONS:
          disable_service_notifications(temp_service);
          break;

        case CMD_ENABLE_HOSTGROUP_SVC_CHECKS:
          enable_service_checks(temp_service);
          break;

        case CMD_DISABLE_HOSTGROUP_SVC_CHECKS:
          disable_service_checks(temp_service);
          break;

        case CMD_ENABLE_HOSTGROUP_PASSIVE_SVC_CHECKS:
          enable_passive_service_checks(temp_service);
          break;

        case CMD_DISABLE_HOSTGROUP_PASSIVE_SVC_CHECKS:
          disable_passive_service_checks(temp_service);
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
  char* str = NULL;
  char* buf[2] = { NULL, NULL };
  int intval = 0;

  (void)entry_time;

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == NULL)
    return (ERROR);

  /* find the host */
  if ((temp_host = find_host(host_name)) == NULL)
    return (ERROR);

  switch (cmd) {
  case CMD_ENABLE_HOST_NOTIFICATIONS:
    enable_host_notifications(temp_host);
    break;

  case CMD_DISABLE_HOST_NOTIFICATIONS:
    disable_host_notifications(temp_host);
    break;

  case CMD_ENABLE_HOST_AND_CHILD_NOTIFICATIONS:
    enable_and_propagate_notifications(temp_host, 0, TRUE, TRUE, FALSE);
    break;

  case CMD_DISABLE_HOST_AND_CHILD_NOTIFICATIONS:
    disable_and_propagate_notifications(temp_host, 0, TRUE, TRUE, FALSE);
    break;

  case CMD_ENABLE_ALL_NOTIFICATIONS_BEYOND_HOST:
    enable_and_propagate_notifications(temp_host, 0, FALSE, TRUE, TRUE);
    break;

  case CMD_DISABLE_ALL_NOTIFICATIONS_BEYOND_HOST:
    disable_and_propagate_notifications(temp_host, 0, FALSE, TRUE, TRUE);
    break;

  case CMD_ENABLE_HOST_SVC_NOTIFICATIONS:
  case CMD_DISABLE_HOST_SVC_NOTIFICATIONS:
    for (temp_servicesmember = temp_host->services;
         temp_servicesmember != NULL;
         temp_servicesmember = temp_servicesmember->next) {
      if ((temp_service = temp_servicesmember->service_ptr) == NULL)
        continue;
      if (cmd == CMD_ENABLE_HOST_SVC_NOTIFICATIONS)
        enable_service_notifications(temp_service);
      else
        disable_service_notifications(temp_service);
    }
    break;

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

  case CMD_ENABLE_PASSIVE_HOST_CHECKS:
    enable_passive_host_checks(temp_host);
    break;

  case CMD_DISABLE_PASSIVE_HOST_CHECKS:
    disable_passive_host_checks(temp_host);
    break;

  case CMD_START_OBSESSING_OVER_HOST:
    start_obsessing_over_host(temp_host);
    break;

  case CMD_STOP_OBSESSING_OVER_HOST:
    stop_obsessing_over_host(temp_host);
    break;

  case CMD_SET_HOST_NOTIFICATION_NUMBER:
    if ((str = my_strtok(NULL, ";"))) {
      intval = atoi(str);
      set_host_notification_number(temp_host, intval);
    }
    break;

  case CMD_SEND_CUSTOM_HOST_NOTIFICATION:
    if ((str = my_strtok(NULL, ";")))
      intval = atoi(str);
    str = my_strtok(NULL, ";");
    if (str)
      buf[0] = my_strdup(str);
    str = my_strtok(NULL, ";");
    if (str)
      buf[1] = my_strdup(str);
    if (buf[0] && buf[1])
      host_notification(temp_host, NOTIFICATION_CUSTOM, buf[0], buf[1], intval);
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
  char* str = NULL;
  char* buf[2] = { NULL, NULL };
  int intval = 0;

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
  case CMD_ENABLE_SVC_NOTIFICATIONS:
    enable_service_notifications(temp_service);
    break;

  case CMD_DISABLE_SVC_NOTIFICATIONS:
    disable_service_notifications(temp_service);
    break;

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

  case CMD_ENABLE_PASSIVE_SVC_CHECKS:
    enable_passive_service_checks(temp_service);
    break;

  case CMD_DISABLE_PASSIVE_SVC_CHECKS:
    disable_passive_service_checks(temp_service);
    break;

  case CMD_START_OBSESSING_OVER_SVC:
    start_obsessing_over_service(temp_service);
    break;

  case CMD_STOP_OBSESSING_OVER_SVC:
    stop_obsessing_over_service(temp_service);
    break;

  case CMD_SET_SVC_NOTIFICATION_NUMBER:
    if ((str = my_strtok(NULL, ";"))) {
      intval = atoi(str);
      set_service_notification_number(temp_service, intval);
    }
    break;

  case CMD_SEND_CUSTOM_SVC_NOTIFICATION:
    if ((str = my_strtok(NULL, ";")))
      intval = atoi(str);
    str = my_strtok(NULL, ";");
    if (str)
      buf[0] = my_strdup(str);
    str = my_strtok(NULL, ";");
    if (str)
      buf[1] = my_strdup(str);
    if (buf[0] && buf[1])
      service_notification(temp_service,
                           NOTIFICATION_CUSTOM,
                           buf[0],
                           buf[1],
                           intval);
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

  case CMD_ENABLE_SERVICEGROUP_SVC_NOTIFICATIONS:
  case CMD_DISABLE_SERVICEGROUP_SVC_NOTIFICATIONS:
  case CMD_ENABLE_SERVICEGROUP_SVC_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_SVC_CHECKS:
  case CMD_ENABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS:

    /* loop through all servicegroup members */
    for (temp_member = temp_servicegroup->members;
         temp_member != NULL;
         temp_member = temp_member->next) {

      temp_service = find_service(temp_member->host_name, temp_member->service_description);
      if (temp_service == NULL)
        continue;

      switch (cmd) {

      case CMD_ENABLE_SERVICEGROUP_SVC_NOTIFICATIONS:
        enable_service_notifications(temp_service);
        break;

      case CMD_DISABLE_SERVICEGROUP_SVC_NOTIFICATIONS:
        disable_service_notifications(temp_service);
        break;

      case CMD_ENABLE_SERVICEGROUP_SVC_CHECKS:
        enable_service_checks(temp_service);
        break;

      case CMD_DISABLE_SERVICEGROUP_SVC_CHECKS:
        disable_service_checks(temp_service);
        break;

      case CMD_ENABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS:
        enable_passive_service_checks(temp_service);
        break;

      case CMD_DISABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS:
        disable_passive_service_checks(temp_service);
        break;

      default:
        break;
      }
    }
    break;

  case CMD_ENABLE_SERVICEGROUP_HOST_NOTIFICATIONS:
  case CMD_DISABLE_SERVICEGROUP_HOST_NOTIFICATIONS:
  case CMD_ENABLE_SERVICEGROUP_HOST_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_HOST_CHECKS:
  case CMD_ENABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS:
  case CMD_DISABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS:
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

      case CMD_ENABLE_SERVICEGROUP_HOST_NOTIFICATIONS:
        enable_host_notifications(temp_host);
        break;

      case CMD_DISABLE_SERVICEGROUP_HOST_NOTIFICATIONS:
        disable_host_notifications(temp_host);
        break;

      case CMD_ENABLE_SERVICEGROUP_HOST_CHECKS:
        enable_host_checks(temp_host);
        break;

      case CMD_DISABLE_SERVICEGROUP_HOST_CHECKS:
        disable_host_checks(temp_host);
        break;

      case CMD_ENABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS:
        enable_passive_host_checks(temp_host);
        break;

      case CMD_DISABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS:
        disable_passive_host_checks(temp_host);
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

int process_contact_command(int cmd,
                            time_t entry_time,
                            char* args) {
  char* contact_name = NULL;
  contact* temp_contact = NULL;

  (void)entry_time;

  /* get the contact name */
  if ((contact_name = my_strtok(args, ";")) == NULL)
    return (ERROR);

  /* find the contact */
  if ((temp_contact = find_contact(contact_name)) == NULL)
    return (ERROR);

  switch (cmd) {

  case CMD_ENABLE_CONTACT_HOST_NOTIFICATIONS:
    enable_contact_host_notifications(temp_contact);
    break;

  case CMD_DISABLE_CONTACT_HOST_NOTIFICATIONS:
    disable_contact_host_notifications(temp_contact);
    break;

  case CMD_ENABLE_CONTACT_SVC_NOTIFICATIONS:
    enable_contact_service_notifications(temp_contact);
    break;

  case CMD_DISABLE_CONTACT_SVC_NOTIFICATIONS:
    disable_contact_service_notifications(temp_contact);
    break;

  default:
    break;
  }
  return (OK);
}

int process_contactgroup_command(int cmd,
                                 time_t entry_time,
                                 char* args) {
  char* contactgroup_name = NULL;
  contactgroup* temp_contactgroup = NULL;
  contactsmember* temp_member = NULL;
  contact* temp_contact = NULL;

  (void)entry_time;

  /* get the contactgroup name */
  if ((contactgroup_name = my_strtok(args, ";")) == NULL)
    return (ERROR);

  /* find the contactgroup */
  if ((temp_contactgroup = find_contactgroup(contactgroup_name)) == NULL)
    return (ERROR);

  switch (cmd) {

  case CMD_ENABLE_CONTACTGROUP_HOST_NOTIFICATIONS:
  case CMD_DISABLE_CONTACTGROUP_HOST_NOTIFICATIONS:
  case CMD_ENABLE_CONTACTGROUP_SVC_NOTIFICATIONS:
  case CMD_DISABLE_CONTACTGROUP_SVC_NOTIFICATIONS:

    /* loop through all contactgroup members */
    for (temp_member = temp_contactgroup->members;
         temp_member != NULL;
         temp_member = temp_member->next) {

      if ((temp_contact = temp_member->contact_ptr) == NULL)
        continue;

      switch (cmd) {

      case CMD_ENABLE_CONTACTGROUP_HOST_NOTIFICATIONS:
        enable_contact_host_notifications(temp_contact);
        break;

      case CMD_DISABLE_CONTACTGROUP_HOST_NOTIFICATIONS:
        disable_contact_host_notifications(temp_contact);
        break;

      case CMD_ENABLE_CONTACTGROUP_SVC_NOTIFICATIONS:
        enable_contact_service_notifications(temp_contact);
        break;

      case CMD_DISABLE_CONTACTGROUP_SVC_NOTIFICATIONS:
        disable_contact_service_notifications(temp_contact);
        break;

      default:
        break;
      }
    }
    break;

  default:
    break;
  }
  return (OK);
}
