/*
** Copyright 1999-2008 Ethan Galstad
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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <sys/time.h>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/modules/external_commands/internal.hh"
#include "com/centreon/engine/modules/external_commands/processing.hh"
#include "com/centreon/engine/modules/external_commands/utils.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "mmap.h"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/******************************************************************/
/****************** EXTERNAL COMMAND PROCESSING *******************/
/******************************************************************/

/* checks for the existence of the external command file and processes all commands found in it */
int check_for_external_commands() {
  logger(dbg_functions, basic) << "check_for_external_commands()";

  /* update last command check time */
  last_command_check = time(NULL);

  /* update the status log with new program information */
  /* go easy on the frequency of this if we're checking often - only update program status every 10 seconds.... */
  if (last_command_check >= (last_command_status_update + 10)) {
    last_command_status_update = last_command_check;
    update_program_status();
  }

  /* process all commands found in the buffer */
  char* buffer(NULL);
  while (1) {

    /* get a lock on the buffer */
    pthread_mutex_lock(&external_command_buffer.buffer_lock);

    /* if no items present, bail out */
    if (external_command_buffer.items <= 0) {
      pthread_mutex_unlock(&external_command_buffer.buffer_lock);
      break;
    }

    buffer = ((char**)external_command_buffer.buffer)[external_command_buffer.tail];
    ((char**)external_command_buffer.buffer)[external_command_buffer.tail] = NULL;

    /* adjust tail counter and number of items */
    external_command_buffer.tail = (external_command_buffer.tail + 1)
      % config->external_command_buffer_slots();
    external_command_buffer.items--;

    /* release the lock on the buffer */
    pthread_mutex_unlock(&external_command_buffer.buffer_lock);

    /* process the command */
    process_external_command(buffer);

    /* free memory */
    delete[] buffer;
  }

  return (OK);
}

/**
 *  Processes all external commands in a (regular) file.
 *
 *  @param[in] file        File to process.
 *  @param[in] delete_file If non-zero, delete file after all commands
 *                         have been processed.
 *
 *  @return OK on success.
 */
int process_external_commands_from_file(
      char const* file,
      int delete_file) {
  logger(dbg_functions, basic)
    << "process_external_commands_from_file()";

  if (!file)
    return (ERROR);

  logger(dbg_external_command, more)
    << "Processing commands from file '" << file
    << "'.  File will " << (delete_file ? "be" : "NOT be")
    << " deleted after processing.";

  /* open the config file for reading */
  mmapfile* thefile(NULL);
  if ((thefile = mmap_fopen(file)) == NULL) {
    logger(log_info_message, basic)
      << "Error: Cannot open file '" << file
      << "' to process external commands!";
    return (ERROR);
  }

  /* process all commands in the file */
  char* input(NULL);
  while (1) {

    /* free memory */
    delete[] input;

    /* read the next line */
    if ((input = mmap_fgets(thefile)) == NULL)
      break;

    /* process the command */
    process_external_command(input);
  }

  /* close the file */
  mmap_fclose(thefile);

  /* delete the file */
  if (delete_file)
    ::remove(file);

  return (OK);
}

/* external command processor */
int process_external_command(char const* cmd) {
  modules::external_commands::gl_processor.execute(cmd);
  return (OK);
}

/******************************************************************/
/*************** EXTERNAL COMMAND IMPLEMENTATIONS  ****************/
/******************************************************************/

/* schedules a host check at a particular time */
int cmd_schedule_check(int cmd, char* args) {
  char* temp_ptr(NULL);
  host* temp_host(NULL);
  service* temp_service(NULL);
  servicesmember* temp_servicesmember(NULL);
  char* host_name(NULL);
  char* svc_description(NULL);
  time_t delay_time(0);

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == NULL)
    return (ERROR);

  if (cmd == CMD_SCHEDULE_HOST_CHECK
      || cmd == CMD_SCHEDULE_FORCED_HOST_CHECK
      || cmd == CMD_SCHEDULE_HOST_SVC_CHECKS
      || cmd == CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS) {

    /* verify that the host is valid */
    if ((temp_host = find_host(host_name)) == NULL)
      return (ERROR);
  }

  else {

    /* get the service description */
    if ((svc_description = my_strtok(NULL, ";")) == NULL)
      return (ERROR);

    /* verify that the service is valid */
    if ((temp_service = find_service(host_name, svc_description)) == NULL)
      return (ERROR);
  }

  /* get the next check time */
  if ((temp_ptr = my_strtok(NULL, "\n")) == NULL)
    return (ERROR);
  delay_time = strtoul(temp_ptr, NULL, 10);

  /* schedule the host check */
  if (cmd == CMD_SCHEDULE_HOST_CHECK
      || cmd == CMD_SCHEDULE_FORCED_HOST_CHECK)
    schedule_host_check(
      temp_host,
      delay_time,
      (cmd == CMD_SCHEDULE_FORCED_HOST_CHECK)
      ? CHECK_OPTION_FORCE_EXECUTION : CHECK_OPTION_NONE);

  /* schedule service checks */
  else if (cmd == CMD_SCHEDULE_HOST_SVC_CHECKS
           || cmd == CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS) {
    for (temp_servicesmember = temp_host->services;
         temp_servicesmember != NULL;
         temp_servicesmember = temp_servicesmember->next) {
      if ((temp_service = temp_servicesmember->service_ptr) == NULL)
        continue;
      schedule_service_check(
        temp_service, delay_time,
        (cmd == CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS)
        ? CHECK_OPTION_FORCE_EXECUTION : CHECK_OPTION_NONE);
    }
  }
  else
    schedule_service_check(
      temp_service, delay_time,
      (cmd == CMD_SCHEDULE_FORCED_SVC_CHECK)
      ? CHECK_OPTION_FORCE_EXECUTION : CHECK_OPTION_NONE);

  return (OK);
}

/* schedules all service checks on a host for a particular time */
int cmd_schedule_host_service_checks(int cmd, char* args, int force) {
  char* temp_ptr(NULL);
  service* temp_service(NULL);
  servicesmember* temp_servicesmember(NULL);
  host* temp_host(NULL);
  char* host_name(NULL);
  time_t delay_time(0);

  (void)cmd;

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == NULL)
    return (ERROR);

  /* verify that the host is valid */
  if ((temp_host = find_host(host_name)) == NULL)
    return (ERROR);

  /* get the next check time */
  if ((temp_ptr = my_strtok(NULL, "\n")) == NULL)
    return (ERROR);
  delay_time = strtoul(temp_ptr, NULL, 10);

  /* reschedule all services on the specified host */
  for (temp_servicesmember = temp_host->services;
       temp_servicesmember != NULL;
       temp_servicesmember = temp_servicesmember->next) {
    if ((temp_service = temp_servicesmember->service_ptr) == NULL)
      continue;
    schedule_service_check(
      temp_service,
      delay_time,
      (force) ? CHECK_OPTION_FORCE_EXECUTION : CHECK_OPTION_NONE);
  }

  return (OK);
}

/* schedules a program shutdown or restart */
void cmd_signal_process(int cmd, char* args) {
  time_t scheduled_time(0);
  char* temp_ptr(NULL);

  /* get the time to schedule the event */
  if ((temp_ptr = my_strtok(args, "\n")) == NULL)
    scheduled_time = 0L;
  else
    scheduled_time = strtoul(temp_ptr, NULL, 10);

  /* add a scheduled program shutdown or restart to the event list */
  int event_type;
  switch (cmd) {
  case CMD_SHUTDOWN_PROCESS:
    event_type = EVENT_PROGRAM_SHUTDOWN;
    break ;
  case CMD_RESTART_PROCESS:
    event_type = EVENT_PROGRAM_RESTART;
    break ;
  default:
    event_type = EVENT_PROGRAM_RELOAD;
  };
  schedule_new_event(
    event_type,
    true,
    scheduled_time,
    false,
    0,
    NULL,
    false,
    NULL,
    NULL,
    0);
}

/* processes results of an external service check */
int cmd_process_service_check_result(
      int cmd,
      time_t check_time,
      char* args) {
  (void)cmd;

  /* get the host name */
  char const* host_name(my_strtok(args, ";"));
  if (!host_name)
    return (ERROR);

  /* get the service description */
  char const* svc_description(my_strtok(NULL, ";"));
  if (!svc_description)
    return (ERROR);

  /* get the service check return code */
  char const* temp_ptr(my_strtok(NULL, ";"));
  if (!temp_ptr)
    return (ERROR);
  int return_code(atoi(temp_ptr));

  /* get the plugin output (may be empty) */
  char const* output(my_strtok(NULL, "\n"));
  if (!output)
    output = "";

  /* submit the passive check result */
  int result = process_passive_service_check(
                 check_time,
                 host_name,
                 svc_description,
                 return_code,
                 output);
  return (result);
}

/* submits a passive service check result for later processing */
int process_passive_service_check(
      time_t check_time,
      char const* host_name,
      char const* svc_description,
      int return_code,
      char const* output) {
  host* temp_host(NULL);
  service* temp_service(NULL);
  char const* real_host_name(NULL);

  /* make sure we have all required data */
  if (host_name == NULL || svc_description == NULL || output == NULL)
    return (ERROR);

  /* find the host by its name or address */
  if (find_host(host_name) != NULL)
    real_host_name = host_name;
  else {
    for (temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) {
      if (!strcmp(host_name, temp_host->address)) {
        real_host_name = temp_host->name;
        break;
      }
    }
  }

  /* we couldn't find the host */
  if (real_host_name == NULL) {
    logger(log_runtime_warning, basic)
      << "Warning:  Passive check result was received for service '"
      << svc_description << "' on host '" << host_name
      << "', but the host could not be found!";
    return (ERROR);
  }

  /* make sure the service exists */
  if ((temp_service = find_service(real_host_name, svc_description)) == NULL) {
    logger(log_runtime_warning, basic)
      << "Warning:  Passive check result was received for service '"
      << svc_description << "' on host '" << host_name
      << "', but the service could not be found!";
    return (ERROR);
  }

  timeval tv;
  gettimeofday(&tv, NULL);

  check_result result;
  memset(&result, 0, sizeof(result));
  result.object_check_type = SERVICE_CHECK;
  result.host_name = string::dup(real_host_name);
  result.service_description = string::dup(svc_description);
  result.check_type = SERVICE_CHECK_PASSIVE;
  result.check_options = CHECK_OPTION_NONE;
  result.scheduled_check = false;
  result.reschedule_check = false;
  result.latency = (double)((double)(tv.tv_sec - check_time)
			    + (double)(tv.tv_usec / 1000.0) / 1000.0);
  result.start_time.tv_sec = check_time;
  result.start_time.tv_usec = 0;
  result.finish_time.tv_sec = check_time;
  result.finish_time.tv_usec = 0;
  result.early_timeout = false;
  result.exited_ok = true;
  result.return_code = return_code;
  result.output = string::dup(output);
  result.next = NULL;
  // result.check_time = check_time;

  /* make sure the return code is within bounds */
  if (result.return_code < 0 || result.return_code > 3) {
    result.return_code = STATE_UNKNOWN;
  }

  if (result.latency < 0.0) {
    result.latency = 0.0;
  }

  checks::checker::instance().push_check_result(result);

  return (OK);
}

/* process passive host check result */
int cmd_process_host_check_result(
      int cmd,
      time_t check_time,
      char* args) {
  (void)cmd;

  /* get the host name */
  char const* host_name(my_strtok(args, ";"));
  if (!host_name)
    return (ERROR);

  /* get the host check return code */
  char const* temp_ptr(my_strtok(NULL, ";"));
  if (!temp_ptr)
    return (ERROR);
  int return_code(atoi(temp_ptr));

  /* get the plugin output (may be empty) */
  char const* output(my_strtok(NULL, "\n"));
  if (!output)
    output = "";

  /* submit the check result */
  int result = process_passive_host_check(
                 check_time,
                 host_name,
                 return_code,
                 output);

  return (result);
}

/* process passive host check result */
int process_passive_host_check(
      time_t check_time,
      char const* host_name,
      int return_code,
      char const* output) {
  host const* temp_host(NULL);
  char const* real_host_name(NULL);

  /* make sure we have all required data */
  if (host_name == NULL || output == NULL)
    return (ERROR);

  /* make sure we have a reasonable return code */
  if (return_code < 0 || return_code > 2)
    return (ERROR);

  /* find the host by its name or address */
  if ((temp_host = find_host(host_name)) != NULL)
    real_host_name = host_name;
  else {
    for (temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) {
      if (!strcmp(host_name, temp_host->address)) {
        real_host_name = temp_host->name;
        break;
      }
    }
  }

  /* we couldn't find the host */
  if (temp_host == NULL) {
    logger(log_runtime_warning, basic)
      << "Warning:  Passive check result was received for host '"
      << host_name << "', but the host could not be found!";
    return (ERROR);
  }

  timeval tv;
  gettimeofday(&tv, NULL);

  check_result result;
  memset(&result, 0, sizeof(result));
  result.object_check_type = HOST_CHECK;
  result.host_name = string::dup(real_host_name);
  result.service_description = NULL;
  result.check_type = HOST_CHECK_PASSIVE;
  result.check_options = CHECK_OPTION_NONE;
  result.scheduled_check = false;
  result.reschedule_check = false;
  result.latency = (double)((double)(tv.tv_sec - check_time)
			    + (double)(tv.tv_usec / 1000.0) / 1000.0);
  result.start_time.tv_sec = check_time;
  result.start_time.tv_usec = 0;
  result.finish_time.tv_sec = check_time;
  result.finish_time.tv_usec = 0;
  result.early_timeout = false;
  result.exited_ok = true;
  result.return_code = return_code;
  result.output = string::dup(output);
  result.next = NULL;
  // result.check_time = check_time;

  /* make sure the return code is within bounds */
  if (result.return_code < 0 || result.return_code > 3) {
    result.return_code = STATE_UNKNOWN;
  }

  if (result.latency < 0.0) {
    result.latency = 0.0;
  }

  checks::checker::instance().push_check_result(result);

  return (OK);
}

/* changes a host or service (integer) variable */
int cmd_change_object_int_var(int cmd, char* args) {
  service* temp_service(NULL);
  host* temp_host(NULL);
  char* host_name(NULL);
  char* svc_description(NULL);
  char const* temp_ptr(NULL);
  int intval(0);
  double dval(0.0);
  double old_dval(0.0);
  time_t preferred_time(0);
  time_t next_valid_time(0);
  unsigned long attr(MODATTR_NONE);

  switch (cmd) {

  case CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL:
  case CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL:
  case CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS:
  case CMD_CHANGE_SVC_MODATTR:

    /* get the host name */
    if ((host_name = my_strtok(args, ";")) == NULL)
      return (ERROR);

    /* get the service name */
    if ((svc_description = my_strtok(NULL, ";")) == NULL)
      return (ERROR);

    /* verify that the service is valid */
    if ((temp_service = find_service(host_name, svc_description)) == NULL)
      return (ERROR);
    break;

  case CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL:
  case CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL:
  case CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS:
  case CMD_CHANGE_HOST_MODATTR:
    /* get the host name */
    if ((host_name = my_strtok(args, ";")) == NULL)
      return (ERROR);

    /* verify that the host is valid */
    if ((temp_host = find_host(host_name)) == NULL)
      return (ERROR);
    break;

  default:
    /* unknown command */
    return (ERROR);
    break;
  }

  /* get the value */
  if ((temp_ptr = my_strtok(NULL, ";")) == NULL)
    return (ERROR);
  intval = (int)strtol(temp_ptr, NULL, 0);
  if (intval < 0 || (intval == 0 && errno == EINVAL))
    return (ERROR);
  dval = (int)strtod(temp_ptr, NULL);

  switch (cmd) {

  case CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL:
    /* save the old check interval */
    old_dval = temp_host->check_interval;

    /* modify the check interval */
    temp_host->check_interval = dval;
    attr = MODATTR_NORMAL_CHECK_INTERVAL;

    /* schedule a host check if previous interval was 0 (checks were not regularly scheduled) */
    if (old_dval == 0 && temp_host->checks_enabled) {

      /* set the host check flag */
      temp_host->should_be_scheduled = true;

      /* schedule a check for right now (or as soon as possible) */
      time(&preferred_time);
      if (check_time_against_period(
            preferred_time,
            temp_host->check_period_ptr,
            temp_host->timezone) == ERROR) {
        get_next_valid_time(
          preferred_time,
          &next_valid_time,
          temp_host->check_period_ptr,
          temp_host->timezone);
        temp_host->next_check = next_valid_time;
      }
      else
        temp_host->next_check = preferred_time;

      /* schedule a check if we should */
      if (temp_host->should_be_scheduled)
        schedule_host_check(temp_host, temp_host->next_check, CHECK_OPTION_NONE);
    }
    break;

  case CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL:
    temp_host->retry_interval = dval;
    attr = MODATTR_RETRY_CHECK_INTERVAL;
    break;

  case CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS:
    temp_host->max_attempts = intval;
    attr = MODATTR_MAX_CHECK_ATTEMPTS;

    /* adjust current attempt number if in a hard state */
    if (temp_host->state_type == HARD_STATE
        && temp_host->current_state != HOST_UP
        && temp_host->current_attempt > 1)
      temp_host->current_attempt = temp_host->max_attempts;
    break;

  case CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL:
    /* save the old check interval */
    old_dval = temp_service->check_interval;

    /* modify the check interval */
    temp_service->check_interval = dval;
    attr = MODATTR_NORMAL_CHECK_INTERVAL;

    /* schedule a service check if previous interval was 0 (checks were not regularly scheduled) */
    if (old_dval == 0 && temp_service->checks_enabled
        && temp_service->check_interval != 0) {

      /* set the service check flag */
      temp_service->should_be_scheduled = true;

      /* schedule a check for right now (or as soon as possible) */
      time(&preferred_time);
      if (check_time_against_period(
            preferred_time,
            temp_service->check_period_ptr,
            temp_service->timezone) == ERROR) {
        get_next_valid_time(
          preferred_time,
          &next_valid_time,
          temp_service->check_period_ptr,
          temp_service->timezone);
        temp_service->next_check = next_valid_time;
      }
      else
        temp_service->next_check = preferred_time;

      /* schedule a check if we should */
      if (temp_service->should_be_scheduled)
        schedule_service_check(temp_service, temp_service->next_check, CHECK_OPTION_NONE);
    }
    break;

  case CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL:
    temp_service->retry_interval = dval;
    attr = MODATTR_RETRY_CHECK_INTERVAL;
    break;

  case CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS:
    temp_service->max_attempts = intval;
    attr = MODATTR_MAX_CHECK_ATTEMPTS;

    /* adjust current attempt number if in a hard state */
    if (temp_service->state_type == HARD_STATE
        && temp_service->current_state != STATE_OK
        && temp_service->current_attempt > 1)
      temp_service->current_attempt = temp_service->max_attempts;
    break;

  case CMD_CHANGE_HOST_MODATTR:
  case CMD_CHANGE_SVC_MODATTR:
    attr = intval;
    break;

  default:
    break;
  }

  /* send data to event broker and update status file */
  switch (cmd) {

  case CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL:
  case CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL:
  case CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS:
  case CMD_CHANGE_SVC_MODATTR:

    /* set the modified service attribute */
    if (cmd == CMD_CHANGE_SVC_MODATTR)
      temp_service->modified_attributes = attr;
    else
      temp_service->modified_attributes |= attr;

    /* send data to event broker */
    broker_adaptive_service_data(
      NEBTYPE_ADAPTIVESERVICE_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      temp_service,
      cmd, attr,
      temp_service->modified_attributes,
      NULL);

    // Update the status log with the service info.
    update_service_status(temp_service);
    break;

  case CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL:
  case CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL:
  case CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS:
  case CMD_CHANGE_HOST_MODATTR:
    /* set the modified host attribute */
    if (cmd == CMD_CHANGE_HOST_MODATTR)
      temp_host->modified_attributes = attr;
    else
      temp_host->modified_attributes |= attr;

    /* send data to event broker */
    broker_adaptive_host_data(
      NEBTYPE_ADAPTIVEHOST_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      temp_host,
      cmd,
      attr,
      temp_host->modified_attributes,
      NULL);

    // Update the status log with the host info.
    update_host_status(temp_host);
    break;

  default:
    break;
  }

  return (OK);
}

/* changes a host or service (char) variable */
int cmd_change_object_char_var(int cmd, char* args) {
  service* temp_service(NULL);
  host* temp_host(NULL);
  timeperiod* temp_timeperiod(NULL);
  command* temp_command(NULL);
  char* host_name(NULL);
  char* svc_description(NULL);
  char* charval(NULL);
  char* temp_ptr(NULL);
  char* temp_ptr2(NULL);
  unsigned long attr(MODATTR_NONE);

  /* SECURITY PATCH - disable these for the time being */
  switch (cmd) {
  case CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER:
  case CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER:
  case CMD_CHANGE_HOST_EVENT_HANDLER:
  case CMD_CHANGE_SVC_EVENT_HANDLER:
  case CMD_CHANGE_HOST_CHECK_COMMAND:
  case CMD_CHANGE_SVC_CHECK_COMMAND:
    return (ERROR);
  }

  /* get the command arguments */
  switch (cmd) {

  case CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER:
  case CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER:
    if ((charval = my_strtok(args, "\n")) == NULL)
      return (ERROR);
    break;

  case CMD_CHANGE_HOST_EVENT_HANDLER:
  case CMD_CHANGE_HOST_CHECK_COMMAND:
  case CMD_CHANGE_HOST_CHECK_TIMEPERIOD:
    /* get the host name */
    if ((host_name = my_strtok(args, ";")) == NULL)
      return (ERROR);

    /* verify that the host is valid */
    if ((temp_host = find_host(host_name)) == NULL)
      return (ERROR);

    if ((charval = my_strtok(NULL, "\n")) == NULL)
      return (ERROR);
    break;

  case CMD_CHANGE_SVC_EVENT_HANDLER:
  case CMD_CHANGE_SVC_CHECK_COMMAND:
  case CMD_CHANGE_SVC_CHECK_TIMEPERIOD:
    /* get the host name */
    if ((host_name = my_strtok(args, ";")) == NULL)
      return (ERROR);

    /* get the service name */
    if ((svc_description = my_strtok(NULL, ";")) == NULL)
      return (ERROR);

    /* verify that the service is valid */
    if ((temp_service = find_service(host_name, svc_description)) == NULL)
      return (ERROR);

    if ((charval = my_strtok(NULL, "\n")) == NULL)
      return (ERROR);
    break;

  default:
    /* invalid command */
    return (ERROR);
    break;
  }

  temp_ptr = string::dup(charval);

  /* do some validation */
  switch (cmd) {

  case CMD_CHANGE_HOST_CHECK_TIMEPERIOD:
  case CMD_CHANGE_SVC_CHECK_TIMEPERIOD:
    /* make sure the timeperiod is valid */
    if ((temp_timeperiod = find_timeperiod(temp_ptr)) == NULL) {
      delete[] temp_ptr;
      return (ERROR);
    }
    break;

  case CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER:
  case CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER:
  case CMD_CHANGE_HOST_EVENT_HANDLER:
  case CMD_CHANGE_SVC_EVENT_HANDLER:
  case CMD_CHANGE_HOST_CHECK_COMMAND:
  case CMD_CHANGE_SVC_CHECK_COMMAND:
    /* make sure the command exists */
    temp_ptr2 = my_strtok(temp_ptr, "!");
    if ((temp_command = find_command(temp_ptr2)) == NULL) {
      delete[] temp_ptr;
      return (ERROR);
    }

    delete[] temp_ptr;
    temp_ptr = string::dup(charval);
    break;

  default:
    break;
  }

  /* update the variable */
  switch (cmd) {

  case CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER:
    config->global_host_event_handler(temp_ptr);
    global_host_event_handler_ptr = temp_command;
    attr = MODATTR_EVENT_HANDLER_COMMAND;
    break;

  case CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER:
    config->global_service_event_handler(temp_ptr);
    global_service_event_handler_ptr = temp_command;
    attr = MODATTR_EVENT_HANDLER_COMMAND;
    break;

  case CMD_CHANGE_HOST_EVENT_HANDLER:
    delete[] temp_host->event_handler;
    temp_host->event_handler = temp_ptr;
    temp_host->event_handler_ptr = temp_command;
    attr = MODATTR_EVENT_HANDLER_COMMAND;
    break;

  case CMD_CHANGE_HOST_CHECK_COMMAND:
    delete[] temp_host->host_check_command;
    temp_host->host_check_command = temp_ptr;
    temp_host->check_command_ptr = temp_command;
    attr = MODATTR_CHECK_COMMAND;
    break;

  case CMD_CHANGE_HOST_CHECK_TIMEPERIOD:
    delete[] temp_host->check_period;
    temp_host->check_period = temp_ptr;
    temp_host->check_period_ptr = temp_timeperiod;
    attr = MODATTR_CHECK_TIMEPERIOD;
    break;

  case CMD_CHANGE_SVC_EVENT_HANDLER:
    delete[] temp_service->event_handler;
    temp_service->event_handler = temp_ptr;
    temp_service->event_handler_ptr = temp_command;
    attr = MODATTR_EVENT_HANDLER_COMMAND;
    break;

  case CMD_CHANGE_SVC_CHECK_COMMAND:
    delete[] temp_service->service_check_command;
    temp_service->service_check_command = temp_ptr;
    temp_service->check_command_ptr = temp_command;
    attr = MODATTR_CHECK_COMMAND;
    break;

  case CMD_CHANGE_SVC_CHECK_TIMEPERIOD:
    delete[] temp_service->check_period;
    temp_service->check_period = temp_ptr;
    temp_service->check_period_ptr = temp_timeperiod;
    attr = MODATTR_CHECK_TIMEPERIOD;
    break;

  default:
    break;
  }

  /* send data to event broker and update status file */
  switch (cmd) {

  case CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER:
    /* set the modified host attribute */
    modified_host_process_attributes |= attr;

    /* send data to event broker */
    broker_adaptive_program_data(
      NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cmd,
      attr,
      modified_host_process_attributes,
      MODATTR_NONE,
      modified_service_process_attributes,
      NULL);
    // Update program status.
    update_program_status();
    break;

  case CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER:
    /* set the modified service attribute */
    modified_service_process_attributes |= attr;

    /* send data to event broker */
    broker_adaptive_program_data(
      NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cmd,
      MODATTR_NONE,
      modified_host_process_attributes,
      attr,
      modified_service_process_attributes,
      NULL);

    // Update program status.
    update_program_status();
    break;

  case CMD_CHANGE_SVC_EVENT_HANDLER:
  case CMD_CHANGE_SVC_CHECK_COMMAND:
  case CMD_CHANGE_SVC_CHECK_TIMEPERIOD:

    /* set the modified service attribute */
    temp_service->modified_attributes |= attr;

    /* send data to event broker */
    broker_adaptive_service_data(
      NEBTYPE_ADAPTIVESERVICE_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      temp_service,
      cmd,
      attr,
      temp_service->modified_attributes,
      NULL);

    // Update the status log with the service info.
    update_service_status(temp_service);
    break;

  case CMD_CHANGE_HOST_EVENT_HANDLER:
  case CMD_CHANGE_HOST_CHECK_COMMAND:
  case CMD_CHANGE_HOST_CHECK_TIMEPERIOD:
    /* set the modified host attribute */
    temp_host->modified_attributes |= attr;

    /* send data to event broker */
    broker_adaptive_host_data(
      NEBTYPE_ADAPTIVEHOST_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      temp_host,
      cmd,
      attr,
      temp_host->modified_attributes,
      NULL);

    // Update the status log with the host info.
    update_host_status(temp_host);
    break;

  default:
    break;
  }

  return (OK);
}

/* changes a custom host or service variable */
int cmd_change_object_custom_var(int cmd, char* args) {
  host* temp_host(NULL);
  service* temp_service(NULL);
  customvariablesmember* temp_customvariablesmember(NULL);
  char* temp_ptr(NULL);
  char* name1(NULL);
  char* name2(NULL);
  char* varname(NULL);
  char* varvalue(NULL);
  int x(0);

  /* get the host name */
  if ((temp_ptr = my_strtok(args, ";")) == NULL)
    return (ERROR);
  name1 = string::dup(temp_ptr);

  /* get the service description if necessary */
  if (cmd == CMD_CHANGE_CUSTOM_SVC_VAR) {
    if ((temp_ptr = my_strtok(NULL, ";")) == NULL) {
      delete[] name1;
      return (ERROR);
    }
    name2 = string::dup(temp_ptr);
  }

  /* get the custom variable name */
  if ((temp_ptr = my_strtok(NULL, ";")) == NULL) {
    delete[] name1;
    delete[] name2;
    return (ERROR);
  }
  varname = string::dup(temp_ptr);

  /* get the custom variable value */
  if ((temp_ptr = my_strtok(NULL, ";")) == NULL) {
    delete[] name1;
    delete[] name2;
    delete[] varname;
    return (ERROR);
  }
  varvalue = string::dup(temp_ptr);

  /* find the object */
  switch (cmd) {

  case CMD_CHANGE_CUSTOM_HOST_VAR:
    if ((temp_host = find_host(name1)) == NULL)
      return (ERROR);
    temp_customvariablesmember = temp_host->custom_variables;
    break;

  case CMD_CHANGE_CUSTOM_SVC_VAR:
    if ((temp_service = find_service(name1, name2)) == NULL)
      return (ERROR);
    temp_customvariablesmember = temp_service->custom_variables;
    break;

  default:
    break;
  }

  /* capitalize the custom variable name */
  for (x = 0; varname[x] != '\x0'; x++)
    varname[x] = toupper(varname[x]);

  /* find the proper variable */
  for (; temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next) {

    /* we found the variable, so update the value */
    if (!strcmp(varname, temp_customvariablesmember->variable_name)) {

      /* update the value */
      delete[] temp_customvariablesmember->variable_value;
      temp_customvariablesmember->variable_value = string::dup(varvalue);

      /* mark the variable value as having been changed */
      temp_customvariablesmember->has_been_modified = true;

      break;
    }
  }

  /* free memory */
  delete[] name1;
  delete[] name2;
  delete[] varname;
  delete[] varvalue;

  /* set the modified attributes and update the status of the object */
  switch (cmd) {

  case CMD_CHANGE_CUSTOM_HOST_VAR:
    temp_host->modified_attributes |= MODATTR_CUSTOM_VARIABLE;
    update_host_status(temp_host);
    break;

  case CMD_CHANGE_CUSTOM_SVC_VAR:
    temp_service->modified_attributes |= MODATTR_CUSTOM_VARIABLE;
    update_service_status(temp_service);
    break;

  default:
    break;
  }

  return (OK);
}

/* processes an external host command */
int cmd_process_external_commands_from_file(int cmd, char* args) {
  char* fname(NULL);
  char* temp_ptr(NULL);
  int delete_file(false);

  (void)cmd;

  /* get the file name */
  if ((temp_ptr = my_strtok(args, ";")) == NULL)
    return (ERROR);
  fname = string::dup(temp_ptr);

  /* find the deletion option */
  if ((temp_ptr = my_strtok(NULL, "\n")) == NULL) {
    delete[] fname;
    return (ERROR);
  }
  if (atoi(temp_ptr) == 0)
    delete_file = false;
  else
    delete_file = true;

  /* process the file */
  process_external_commands_from_file(fname, delete_file);

  /* free memory */
  delete[] fname;

  return (OK);
}

/******************************************************************/
/*************** INTERNAL COMMAND IMPLEMENTATIONS  ****************/
/******************************************************************/

/* temporarily disables a service check */
void disable_service_checks(service* svc) {
  unsigned long attr(MODATTR_ACTIVE_CHECKS_ENABLED);

  /* checks are already disabled */
  if (svc->checks_enabled == false)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* disable the service check... */
  svc->checks_enabled = false;
  svc->should_be_scheduled = false;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    NULL);

  // Update the status log to reflect the new service state.
  update_service_status(svc);
}

/* enables a service check */
void enable_service_checks(service* svc) {
  time_t preferred_time(0);
  time_t next_valid_time(0);
  unsigned long attr(MODATTR_ACTIVE_CHECKS_ENABLED);

  /* checks are already enabled */
  if (svc->checks_enabled)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* enable the service check... */
  svc->checks_enabled = true;
  svc->should_be_scheduled = true;

  /* services with no check intervals don't get checked */
  if (svc->check_interval == 0)
    svc->should_be_scheduled = false;

  /* schedule a check for right now (or as soon as possible) */
  time(&preferred_time);
  if (check_time_against_period(
        preferred_time,
        svc->check_period_ptr,
        svc->timezone) == ERROR) {
    get_next_valid_time(
      preferred_time,
      &next_valid_time,
      svc->check_period_ptr,
      svc->timezone);
    svc->next_check = next_valid_time;
  }
  else
    svc->next_check = preferred_time;

  /* schedule a check if we should */
  if (svc->should_be_scheduled)
    schedule_service_check(svc, svc->next_check, CHECK_OPTION_NONE);

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    NULL);

  // Update the status log to reflect the new service state.
  update_service_status(svc);
}

/* enables event handlers on a program-wide basis */
void start_using_event_handlers(void) {
  unsigned long attr(MODATTR_EVENT_HANDLER_ENABLED);

  /* no change */
  if (config->enable_event_handlers())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* set the event handler flag */
  config->enable_event_handlers(true);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* disables event handlers on a program-wide basis */
void stop_using_event_handlers(void) {
  unsigned long attr(MODATTR_EVENT_HANDLER_ENABLED);

  /* no change */
  if (config->enable_event_handlers() == false)
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* set the event handler flag */
  config->enable_event_handlers(false);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* enables the event handler for a particular service */
void enable_service_event_handler(service* svc) {
  unsigned long attr(MODATTR_EVENT_HANDLER_ENABLED);

  /* no change */
  if (svc->event_handler_enabled)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* set the event handler flag */
  svc->event_handler_enabled = true;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    NULL);

  // Update the status log with the service info.
  update_service_status(svc);
}

/* disables the event handler for a particular service */
void disable_service_event_handler(service* svc) {
  unsigned long attr(MODATTR_EVENT_HANDLER_ENABLED);

  /* no change */
  if (svc->event_handler_enabled == false)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* set the event handler flag */
  svc->event_handler_enabled = false;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    NULL);

  // Update the status log with the service info.
  update_service_status(svc);
}

/* enables the event handler for a particular host */
void enable_host_event_handler(host* hst) {
  unsigned long attr(MODATTR_EVENT_HANDLER_ENABLED);

  /* no change */
  if (hst->event_handler_enabled)
    return;

  /* set the attribute modified flag */
  hst->modified_attributes |= attr;

  /* set the event handler flag */
  hst->event_handler_enabled = true;

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->modified_attributes,
    NULL);

  // Update the status log with the host info.
  update_host_status(hst);
}

/* disables the event handler for a particular host */
void disable_host_event_handler(host* hst) {
  unsigned long attr(MODATTR_EVENT_HANDLER_ENABLED);

  /* no change */
  if (hst->event_handler_enabled == false)
    return;

  /* set the attribute modified flag */
  hst->modified_attributes |= attr;

  /* set the event handler flag */
  hst->event_handler_enabled = false;

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->modified_attributes,
    NULL);

  // Update the status log with the host info.
  update_host_status(hst);
}

/* disables checks of a particular host */
void disable_host_checks(host* hst) {
  unsigned long attr(MODATTR_ACTIVE_CHECKS_ENABLED);

  /* checks are already disabled */
  if (hst->checks_enabled == false)
    return;

  /* set the attribute modified flag */
  hst->modified_attributes |= attr;

  /* set the host check flag */
  hst->checks_enabled = false;
  hst->should_be_scheduled = false;

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->modified_attributes,
    NULL);

  // Update the status log with the host info.
  update_host_status(hst);
}

/* enables checks of a particular host */
void enable_host_checks(host* hst) {
  time_t preferred_time(0);
  time_t next_valid_time(0);
  unsigned long attr(MODATTR_ACTIVE_CHECKS_ENABLED);

  /* checks are already enabled */
  if (hst->checks_enabled)
    return;

  /* set the attribute modified flag */
  hst->modified_attributes |= attr;

  /* set the host check flag */
  hst->checks_enabled = true;
  hst->should_be_scheduled = true;

  /* hosts with no check intervals don't get checked */
  if (hst->check_interval == 0)
    hst->should_be_scheduled = false;

  /* schedule a check for right now (or as soon as possible) */
  time(&preferred_time);
  if (check_time_against_period(
        preferred_time,
        hst->check_period_ptr,
        hst->timezone) == ERROR) {
    get_next_valid_time(
      preferred_time,
      &next_valid_time,
      hst->check_period_ptr,
      hst->timezone);
    hst->next_check = next_valid_time;
  }
  else
    hst->next_check = preferred_time;

  /* schedule a check if we should */
  if (hst->should_be_scheduled)
    schedule_host_check(hst, hst->next_check, CHECK_OPTION_NONE);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->modified_attributes,
    NULL);

  // Update the status log with the host info.
  update_host_status(hst);
}

/* start obsessing over service check results */
void start_obsessing_over_service_checks(void) {
  unsigned long attr(MODATTR_OBSESSIVE_HANDLER_ENABLED);

  /* no change */
  if (config->obsess_over_services())
    return;

  /* set the attribute modified flag */
  modified_service_process_attributes |= attr;

  /* set the service obsession flag */
  config->obsess_over_services(true);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    MODATTR_NONE,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* stop obsessing over service check results */
void stop_obsessing_over_service_checks(void) {
  unsigned long attr(MODATTR_OBSESSIVE_HANDLER_ENABLED);

  /* no change */
  if (config->obsess_over_services() == false)
    return;

  /* set the attribute modified flag */
  modified_service_process_attributes |= attr;

  /* set the service obsession flag */
  config->obsess_over_services(false);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    MODATTR_NONE,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* start obsessing over host check results */
void start_obsessing_over_host_checks(void) {
  unsigned long attr = MODATTR_OBSESSIVE_HANDLER_ENABLED;

  /* no change */
  if (config->obsess_over_hosts())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;

  /* set the host obsession flag */
  config->obsess_over_hosts(true);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    MODATTR_NONE,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* stop obsessing over host check results */
void stop_obsessing_over_host_checks(void) {
  unsigned long attr(MODATTR_OBSESSIVE_HANDLER_ENABLED);

  /* no change */
  if (config->obsess_over_hosts() == false)
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;

  /* set the host obsession flag */
  config->obsess_over_hosts(false);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    MODATTR_NONE,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* enables service freshness checking */
void enable_service_freshness_checks(void) {
  unsigned long attr(MODATTR_FRESHNESS_CHECKS_ENABLED);

  /* no change */
  if (config->check_service_freshness())
    return;

  /* set the attribute modified flag */
  modified_service_process_attributes |= attr;

  /* set the freshness check flag */
  config->check_service_freshness(true);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    MODATTR_NONE,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* disables service freshness checking */
void disable_service_freshness_checks(void) {
  unsigned long attr(MODATTR_FRESHNESS_CHECKS_ENABLED);

  /* no change */
  if (config->check_service_freshness() == false)
    return;

  /* set the attribute modified flag */
  modified_service_process_attributes |= attr;

  /* set the freshness check flag */
  config->check_service_freshness(false);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    MODATTR_NONE,
    modified_host_process_attributes,
    attr,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* enables host freshness checking */
void enable_host_freshness_checks(void) {
  unsigned long attr(MODATTR_FRESHNESS_CHECKS_ENABLED);

  /* no change */
  if (config->check_host_freshness())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;

  /* set the freshness check flag */
  config->check_host_freshness(true);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    MODATTR_NONE,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* disables host freshness checking */
void disable_host_freshness_checks(void) {
  unsigned long attr(MODATTR_FRESHNESS_CHECKS_ENABLED);

  /* no change */
  if (config->check_host_freshness() == false)
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;

  /* set the freshness check flag */
  config->check_host_freshness(false);

  /* send data to event broker */
  broker_adaptive_program_data(
    NEBTYPE_ADAPTIVEPROGRAM_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    attr,
    modified_host_process_attributes,
    MODATTR_NONE,
    modified_service_process_attributes,
    NULL);

  // Update the status log with the program info.
  update_program_status();
}

/* start obsessing over a particular service */
void start_obsessing_over_service(service* svc) {
  unsigned long attr(MODATTR_OBSESSIVE_HANDLER_ENABLED);

  /* no change */
  if (svc->obsess_over_service)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* set the obsess over service flag */
  svc->obsess_over_service = true;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    NULL);

  // Update the status log with the service info.
  update_service_status(svc);
}

/* stop obsessing over a particular service */
void stop_obsessing_over_service(service* svc) {
  unsigned long attr(MODATTR_OBSESSIVE_HANDLER_ENABLED);

  /* no change */
  if (svc->obsess_over_service == false)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* set the obsess over service flag */
  svc->obsess_over_service = false;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    NULL);

  // Update the status log with the service info.
  update_service_status(svc);
}

/* start obsessing over a particular host */
void start_obsessing_over_host(host* hst) {
  unsigned long attr(MODATTR_OBSESSIVE_HANDLER_ENABLED);

  /* no change */
  if (hst->obsess_over_host)
    return;

  /* set the attribute modified flag */
  hst->modified_attributes |= attr;

  /* set the obsess over host flag */
  hst->obsess_over_host = true;

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->modified_attributes,
    NULL);

  // Update the status log with the host info.
  update_host_status(hst);
}

/* stop obsessing over a particular host */
void stop_obsessing_over_host(host* hst) {
  unsigned long attr(MODATTR_OBSESSIVE_HANDLER_ENABLED);

  /* no change */
  if (hst->obsess_over_host == false)
    return;

  /* set the attribute modified flag */
  hst->modified_attributes |= attr;

  /* set the obsess over host flag */
  hst->obsess_over_host = false;

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->modified_attributes,
    NULL);

  // Update the status log with the host info.
  update_host_status(hst);
}
