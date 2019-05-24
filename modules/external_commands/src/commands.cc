/*
** Copyright 1999-2008           Ethan Galstad
** Copyright 2011-2013,2015-2019 Centreon
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
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/downtimes/downtime_finder.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/modules/external_commands/internal.hh"
#include "com/centreon/engine/modules/external_commands/processing.hh"
#include "com/centreon/engine/modules/external_commands/utils.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/downtimes/downtime.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "mmap.h"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::logging;

/******************************************************************/
/****************** EXTERNAL COMMAND PROCESSING *******************/
/******************************************************************/

/* checks for the existence of the external command file and processes all commands found in it */
int check_for_external_commands() {
  logger(dbg_functions, basic) << "check_for_external_commands()";

  /* bail out if we shouldn't be checking for external commands */
  if (!config->check_external_commands())
    return ERROR;

  /* update last command check time */
  last_command_check = time(nullptr);

  /* update the status log with new program information */
  /* go easy on the frequency of this if we're checking often - only update program status every 10 seconds.... */
  if (last_command_check >= (last_command_status_update + 10)) {
    last_command_status_update = last_command_check;
    update_program_status(false);
  }

  /* process all commands found in the buffer */
  char* buffer(nullptr);
  while (1) {

    /* get a lock on the buffer */
    pthread_mutex_lock(&external_command_buffer.buffer_lock);

    /* if no items present, bail out */
    if (external_command_buffer.items <= 0) {
      pthread_mutex_unlock(&external_command_buffer.buffer_lock);
      break;
    }

    buffer = ((char**)external_command_buffer.buffer)[external_command_buffer.tail];
    ((char**)external_command_buffer.buffer)[external_command_buffer.tail] = nullptr;

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

  return OK;
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
    return ERROR;

  logger(dbg_external_command, more)
    << "Processing commands from file '" << file
    << "'.  File will " << (delete_file ? "be" : "NOT be")
    << " deleted after processing.";

  /* open the config file for reading */
  mmapfile* thefile(nullptr);
  if ((thefile = mmap_fopen(file)) == nullptr) {
    logger(log_info_message, basic)
      << "Error: Cannot open file '" << file
      << "' to process external commands!";
    return ERROR;
  }

  /* process all commands in the file */
  char* input(nullptr);
  while (1) {

    /* free memory */
    delete[] input;

    /* read the next line */
    if ((input = mmap_fgets(thefile)) == nullptr)
      break;

    /* process the command */
    process_external_command(input);
  }

  /* close the file */
  mmap_fclose(thefile);

  /* delete the file */
  if (delete_file)
    ::remove(file);

  return OK;
}

/* external command processor */
int process_external_command(char const* cmd) {
  modules::external_commands::gl_processor.execute(cmd);
  return OK;
}

/******************************************************************/
/*************** EXTERNAL COMMAND IMPLEMENTATIONS  ****************/
/******************************************************************/

/* adds a host or service comment to the status log */
int cmd_add_comment(int cmd, time_t entry_time, char* args) {
  char* temp_ptr(nullptr);
  com::centreon::engine::host* temp_host(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  char* host_name(nullptr);
  char* svc_description(nullptr);
  char* user(nullptr);
  char* comment_data(nullptr);
  int persistent(0);
  int result(0);

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == nullptr)
    return ERROR;

  /* if we're adding a service comment...  */
  if (cmd == CMD_ADD_SVC_COMMENT) {

    /* get the service description */
    if ((svc_description = my_strtok(nullptr, ";")) == nullptr)
      return ERROR;

    /* verify that the service is valid */
    if ((temp_service = find_service(host_name, svc_description)) == nullptr)
      return ERROR;
  }

  /* else verify that the host is valid */
  temp_host = nullptr;
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
  if (it != configuration::applier::state::instance().hosts().end())
    temp_host = it->second.get();
  if (temp_host  == nullptr)
    return ERROR;

  /* get the persistent flag */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  persistent = atoi(temp_ptr);
  if (persistent > 1)
    persistent = 1;
  else if (persistent < 0)
    persistent = 0;

  /* get the name of the user who entered the comment */
  if ((user = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;

  /* get the comment */
  if ((comment_data = my_strtok(nullptr, "\n")) == nullptr)
    return ERROR;

  /* add the comment */
  std::shared_ptr<comment> com =
    std::make_shared<comment>(
             (cmd == CMD_ADD_HOST_COMMENT) ? comment::host : comment::service,
             comment::user,
             host_name,
             svc_description,
             entry_time,
             user,
             comment_data,
             persistent,
             comment::external,
             false,
             (time_t)0);
  comment::comments.insert({com->get_comment_id(), com});

  return OK;
}

/* removes a host or service comment from the status log */
int cmd_delete_comment(int cmd, char* args) {
  uint64_t comment_id{0};

  /* get the comment id we should delete */
  if ((comment_id = strtoul(args, nullptr, 10)) == 0)
    return ERROR;

  /* delete the specified comment */
  comment::delete_comment(comment_id);

  return OK;
}

/* removes all comments associated with a host or service from the status log */
int cmd_delete_all_comments(int cmd, char* args) {
  com::centreon::engine::service* temp_service(nullptr);
  com::centreon::engine::host* temp_host(nullptr);
  char* host_name(nullptr);
  char* svc_description(nullptr);

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == nullptr)
    return ERROR;

  /* if we're deleting service comments...  */
  if (cmd == CMD_DEL_ALL_SVC_COMMENTS) {

    /* get the service description */
    if ((svc_description = my_strtok(nullptr, ";")) == nullptr)
      return ERROR;

    /* verify that the service is valid */
    if ((temp_service = find_service(host_name, svc_description)) == nullptr)
      return ERROR;
  }

  /* else verify that the host is valid */
  temp_host = nullptr;
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
  if (it != configuration::applier::state::instance().hosts().end())
    temp_host = it->second.get();
  if (temp_host  == nullptr)
    return ERROR;

  /* delete comments */
  if (cmd == CMD_DEL_ALL_HOST_COMMENTS)
    comment::delete_host_comments(host_name);
  else
    comment::delete_service_comments(host_name, svc_description);

  return OK;
}

/* delays a host or service notification for given number of minutes */
int cmd_delay_notification(int cmd, char* args) {
  char* temp_ptr(nullptr);
  com::centreon::engine::host* temp_host(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  char* host_name(nullptr);
  char* svc_description(nullptr);
  time_t delay_time(0);

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == nullptr)
    return ERROR;

  /* if this is a service notification delay...  */
  if (cmd == CMD_DELAY_SVC_NOTIFICATION) {

    /* get the service description */
    if ((svc_description = my_strtok(nullptr, ";")) == nullptr)
      return ERROR;

    /* verify that the service is valid */
    if ((temp_service = find_service(host_name, svc_description)) == nullptr)
      return ERROR;
  }

  /* else verify that the host is valid */
  else {
    temp_host = nullptr;
    umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
    if (it != configuration::applier::state::instance().hosts().end())
      temp_host = it->second.get();
    if (temp_host  == nullptr)
      return ERROR;
  }

  /* get the time that we should delay until... */
  if ((temp_ptr = my_strtok(nullptr, "\n")) == nullptr)
    return ERROR;
  delay_time = strtoul(temp_ptr, nullptr, 10);

  /* delay the next notification... */
  if (cmd == CMD_DELAY_HOST_NOTIFICATION)
    temp_host->set_next_host_notification(delay_time);
  else
    temp_service->next_notification = delay_time;

  return OK;
}

/* schedules a host check at a particular time */
int cmd_schedule_check(int cmd, char* args) {
  char* temp_ptr(nullptr);
  com::centreon::engine::host* temp_host(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  servicesmember* temp_servicesmember(nullptr);
  char* host_name(nullptr);
  char* svc_description(nullptr);
  time_t delay_time(0);

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == nullptr)
    return ERROR;

  if (cmd == CMD_SCHEDULE_HOST_CHECK
      || cmd == CMD_SCHEDULE_FORCED_HOST_CHECK
      || cmd == CMD_SCHEDULE_HOST_SVC_CHECKS
      || cmd == CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS) {

    /* verify that the host is valid */
    temp_host = nullptr;
    umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
     it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
    if (it != configuration::applier::state::instance().hosts().end())
      temp_host = it->second.get();
    if (temp_host  == nullptr)
      return ERROR;
  }

  else {

    /* get the service description */
    if ((svc_description = my_strtok(nullptr, ";")) == nullptr)
      return ERROR;

    /* verify that the service is valid */
    if ((temp_service = find_service(host_name, svc_description)) == nullptr)
      return ERROR;
  }

  /* get the next check time */
  if ((temp_ptr = my_strtok(nullptr, "\n")) == nullptr)
    return ERROR;
  delay_time = strtoul(temp_ptr, nullptr, 10);

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
         temp_servicesmember != nullptr;
         temp_servicesmember = temp_servicesmember->next) {
      if ((temp_service = temp_servicesmember->service_ptr) == nullptr)
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

  return OK;
}

/* schedules all service checks on a host for a particular time */
int cmd_schedule_host_service_checks(int cmd, char* args, int force) {
  char* temp_ptr(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  servicesmember* temp_servicesmember(nullptr);
  com::centreon::engine::host* temp_host(nullptr);
  char* host_name(nullptr);
  time_t delay_time(0);

  (void)cmd;

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == nullptr)
    return ERROR;

  /* verify that the host is valid */
  temp_host = nullptr;
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
   it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
  if (it != configuration::applier::state::instance().hosts().end())
    temp_host = it->second.get();
  if (temp_host  == nullptr)
    return ERROR;

  /* get the next check time */
  if ((temp_ptr = my_strtok(nullptr, "\n")) == nullptr)
    return ERROR;
  delay_time = strtoul(temp_ptr, nullptr, 10);

  /* reschedule all services on the specified host */
  for (temp_servicesmember = temp_host->services;
       temp_servicesmember != nullptr;
       temp_servicesmember = temp_servicesmember->next) {
    if ((temp_service = temp_servicesmember->service_ptr) == nullptr)
      continue;
    schedule_service_check(
      temp_service,
      delay_time,
      (force) ? CHECK_OPTION_FORCE_EXECUTION : CHECK_OPTION_NONE);
  }

  return OK;
}

/* schedules a program shutdown or restart */
void cmd_signal_process(int cmd, char* args) {
  time_t scheduled_time(0);
  char* temp_ptr(nullptr);

  /* get the time to schedule the event */
  if ((temp_ptr = my_strtok(args, "\n")) == nullptr)
    scheduled_time = 0L;
  else
    scheduled_time = strtoul(temp_ptr, nullptr, 10);

  /* add a scheduled program shutdown or restart to the event list */
  schedule_new_event(
    (cmd == CMD_SHUTDOWN_PROCESS) ? EVENT_PROGRAM_SHUTDOWN : EVENT_PROGRAM_RESTART,
    true,
    scheduled_time,
    false,
    0,
    nullptr,
    false,
    nullptr,
    nullptr,
    0);
}

/**
 *  Processes results of an external service check.
 *
 *  @param[in]     cmd         Command ID.
 *  @param[in]     check_time  Check time.
 *  @param[in,out] args        Command arguments.
 *
 *  @return OK on success.
 */
int cmd_process_service_check_result(
      int cmd,
      time_t check_time,
      char* args) {
  (void)cmd;

  if (!args)
    return ERROR;
  char* delimiter;

  // Get the host name.
  char* host_name(args);

  // Get the service description.
  delimiter = strchr(host_name, ';');
  if (!delimiter)
    return ERROR;
  *delimiter = '\0';
  ++delimiter;
  char* svc_description(delimiter);

  // Get the service check return code and output.
  delimiter = strchr(svc_description, ';');
  if (!delimiter)
    return ERROR;
  *delimiter = '\0';
  ++delimiter;
  char const* output(strchr(delimiter, ';'));
  if (output) {
    *const_cast<char*>(output) = '\0';
    ++output;
  }
  else
    output = "";
  int return_code(strtol(delimiter, nullptr, 0));

  // Submit the passive check result.
  return (process_passive_service_check(
            check_time,
            host_name,
            svc_description,
            return_code,
            output));
}

/* submits a passive service check result for later processing */
int process_passive_service_check(
      time_t check_time,
      char const* host_name,
      char const* svc_description,
      int return_code,
      char const* output) {
  com::centreon::engine::host* temp_host(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  char const* real_host_name(nullptr);

  /* skip this service check result if we aren't accepting passive service checks */
  if (config->accept_passive_service_checks() == false)
    return ERROR;

  /* make sure we have all required data */
  if (host_name == nullptr || svc_description == nullptr || output == nullptr)
    return ERROR;

  /* find the host by its name or address */
  temp_host = nullptr;
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
  if (it != configuration::applier::state::instance().hosts().end())
    temp_host = it->second.get();
  if (temp_host != nullptr)
    real_host_name = host_name;
  else {
    for (host_map::iterator
           it(com::centreon::engine::host::hosts.begin()),
           end(com::centreon::engine::host::hosts.end());
         it != end;
         ++it) {
      if (it->second->get_address() == host_name) {
        real_host_name = it->first.c_str();
        break ;
      }
    }
  }

  /* we couldn't find the host */
  if (real_host_name == nullptr) {
    logger(log_runtime_warning, basic)
      << "Warning:  Passive check result was received for service '"
      << svc_description << "' on host '" << host_name
      << "', but the host could not be found!";
    return ERROR;
  }

  /* make sure the service exists */
  if ((temp_service = find_service(real_host_name, svc_description)) == nullptr) {
    logger(log_runtime_warning, basic)
      << "Warning:  Passive check result was received for service '"
      << svc_description << "' on host '" << host_name
      << "', but the service could not be found!";
    return ERROR;
  }

  /* skip this is we aren't accepting passive checks for this service */
  if (temp_service->accept_passive_service_checks == false)
    return ERROR;

  timeval tv;
  gettimeofday(&tv, nullptr);

  check_result result;
  result.object_check_type = SERVICE_CHECK;
  result.host_name = string::dup(real_host_name);
  result.service_description = string::dup(svc_description);
  result.check_type = SERVICE_CHECK_PASSIVE;
  result.check_options = CHECK_OPTION_NONE;
  result.scheduled_check = false;
  result.reschedule_check = false;
  result.output_file = nullptr;
  result.output_file_fp = nullptr;
  result.output_file_fd = -1;
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
  result.next = nullptr;
  // result.check_time = check_time;

  /* make sure the return code is within bounds */
  if (result.return_code < 0 || result.return_code > 3) {
    result.return_code = STATE_UNKNOWN;
  }

  if (result.latency < 0.0) {
    result.latency = 0.0;
  }

  checks::checker::instance().push_check_result(result);

  return OK;
}

/**
 *  Processes results of an external host check.
 *
 *  @param[in]     cmd         Command ID.
 *  @param[in]     check_time  Check time.
 *  @param[in,out] args        Command arguments.
 *
 *  @return OK on success.
 */
int cmd_process_host_check_result(
      int cmd,
      time_t check_time,
      char* args) {
  (void)cmd;

  if (!args)
    return ERROR;

  // Get the host name.
  char* host_name(args);

  // Get the host check return code and output.
  char* delimiter(strchr(host_name, ';'));
  if (!delimiter)
    return ERROR;
  *delimiter = '\0';
  ++delimiter;
  char const* output(strchr(delimiter, ';'));
  if (output) {
    *const_cast<char*>(output) = '\0';
    ++output;
  }
  else
    output = "";
  int return_code(strtol(delimiter, nullptr, 0));

  // Submit the check result.
  return (process_passive_host_check(
            check_time,
            host_name,
            return_code,
            output));
}

/* process passive host check result */
int process_passive_host_check(
      time_t check_time,
      char const* host_name,
      int return_code,
      char const* output) {
  host const* temp_host(nullptr);
  char const* real_host_name(nullptr);

  /* skip this host check result if we aren't accepting passive host checks */
  if (config->accept_passive_service_checks() == false)
    return ERROR;

  /* make sure we have all required data */
  if (host_name == nullptr || output == nullptr)
    return ERROR;

  /* make sure we have a reasonable return code */
  if (return_code < 0 || return_code > 2)
    return ERROR;

  /* find the host by its name or address */
  temp_host = nullptr;
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
  if (it != configuration::applier::state::instance().hosts().end())
    temp_host = it->second.get();
  if (temp_host != nullptr)
    real_host_name = host_name;
  else {
    for (host_map::iterator
           it(com::centreon::engine::host::hosts.begin()),
           end(com::centreon::engine::host::hosts.end());
         it != end;
         ++it) {
      if (it->second->get_address() == host_name) {
        real_host_name = it->first.c_str();
        break;
      }
    }
  }

  /* we couldn't find the host */
  if (temp_host == nullptr) {
    logger(log_runtime_warning, basic)
      << "Warning:  Passive check result was received for host '"
      << host_name << "', but the host could not be found!";
    return ERROR;
  }

  /* skip this is we aren't accepting passive checks for this host */
  if (!temp_host->get_accept_passive_host_checks())
    return ERROR;

  timeval tv;
  gettimeofday(&tv, nullptr);

  check_result result;
  result.object_check_type = HOST_CHECK;
  result.host_name = string::dup(real_host_name);
  result.service_description = nullptr;
  result.check_type = HOST_CHECK_PASSIVE;
  result.check_options = CHECK_OPTION_NONE;
  result.scheduled_check = false;
  result.reschedule_check = false;
  result.output_file = nullptr;
  result.output_file_fp = nullptr;
  result.output_file_fd = -1;
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
  result.next = nullptr;
  // result.check_time = check_time;

  /* make sure the return code is within bounds */
  if (result.return_code < 0 || result.return_code > 3) {
    result.return_code = STATE_UNKNOWN;
  }

  if (result.latency < 0.0) {
    result.latency = 0.0;
  }

  checks::checker::instance().push_check_result(result);

  return OK;
}

/* acknowledges a host or service problem */
int cmd_acknowledge_problem(int cmd, char* args) {
  com::centreon::engine::service* temp_service(nullptr);
  com::centreon::engine::host* temp_host(nullptr);
  char* host_name(nullptr);
  char* svc_description(nullptr);
  char* ack_author(nullptr);
  char* ack_data(nullptr);
  char* temp_ptr(nullptr);
  int type(ACKNOWLEDGEMENT_NORMAL);
  int notify(true);
  int persistent(true);

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == nullptr)
    return ERROR;

  /* verify that the host is valid */
  temp_host = nullptr;
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
  if (it != configuration::applier::state::instance().hosts().end())
    temp_host = it->second.get();
  if (temp_host  == nullptr)
    return ERROR;

  /* this is a service acknowledgement */
  if (cmd == CMD_ACKNOWLEDGE_SVC_PROBLEM) {

    /* get the service name */
    if ((svc_description = my_strtok(nullptr, ";")) == nullptr)
      return ERROR;

    /* verify that the service is valid */
    if ((temp_service = find_service(temp_host->get_name().c_str(), svc_description)) == nullptr)
      return ERROR;
  }

  /* get the type */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  type = atoi(temp_ptr);

  /* get the notification option */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  notify = (atoi(temp_ptr) > 0) ? true : false;

  /* get the persistent option */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  persistent = (atoi(temp_ptr) > 0) ? true : false;

  /* get the acknowledgement author */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  ack_author = string::dup(temp_ptr);

  /* get the acknowledgement data */
  if ((temp_ptr = my_strtok(nullptr, "\n")) == nullptr) {
    delete[] ack_author;
    return ERROR;
  }
  ack_data = string::dup(temp_ptr);

  /* acknowledge the host problem */
  if (cmd == CMD_ACKNOWLEDGE_HOST_PROBLEM)
    acknowledge_host_problem(
      temp_host,
      ack_author,
      ack_data,
      type,
      notify,
      persistent);
  /* acknowledge the service problem */
  else
    acknowledge_service_problem(
      temp_service,
      ack_author,
      ack_data,
      type,
      notify,
      persistent);

  /* free memory */
  delete[] ack_author;
  delete[] ack_data;

  return OK;
}

/* removes a host or service acknowledgement */
int cmd_remove_acknowledgement(int cmd, char* args) {
  com::centreon::engine::service* temp_service(nullptr);
  com::centreon::engine::host* temp_host(nullptr);
  char* host_name(nullptr);
  char* svc_description(nullptr);

  /* get the host name */
  if ((host_name = my_strtok(args, ";")) == nullptr)
    return ERROR;

  /* verify that the host is valid */
  temp_host = nullptr;
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
  if (it != configuration::applier::state::instance().hosts().end())
    temp_host = it->second.get();
  if (temp_host  == nullptr)
    return ERROR;

  /* we are removing a service acknowledgement */
  if (cmd == CMD_REMOVE_SVC_ACKNOWLEDGEMENT) {

    /* get the service name */
    if ((svc_description = my_strtok(nullptr, ";")) == nullptr)
      return ERROR;

    /* verify that the service is valid */
    if (!(temp_service = find_service(temp_host->get_name().c_str(), svc_description)))
      return ERROR;
  }

  /* acknowledge the host problem */
  if (cmd == CMD_REMOVE_HOST_ACKNOWLEDGEMENT)
    remove_host_acknowledgement(temp_host);

  /* acknowledge the service problem */
  else
    remove_service_acknowledgement(temp_service);

  return OK;
}

/* schedules downtime for a specific host or service */
int cmd_schedule_downtime(int cmd, time_t entry_time, char* args) {
  servicesmember* temp_servicesmember(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  com::centreon::engine::host* temp_host(nullptr);
  com::centreon::engine::host* last_host(nullptr);
  hostgroup* temp_hostgroup(nullptr);
  servicegroup* temp_servicegroup(nullptr);
  servicesmember* temp_sgmember(nullptr);
  char* host_name(nullptr);
  char* hostgroup_name(nullptr);
  char* servicegroup_name(nullptr);
  char* svc_description(nullptr);
  char* temp_ptr(nullptr);
  time_t start_time(0);
  time_t end_time(0);
  int fixed(0);
  uint64_t triggered_by(0);
  unsigned long duration(0);
  char* author(nullptr);
  char* comment_data(nullptr);
  uint64_t downtime_id{0};

  if (cmd == CMD_SCHEDULE_HOSTGROUP_HOST_DOWNTIME
      || cmd == CMD_SCHEDULE_HOSTGROUP_SVC_DOWNTIME) {

    /* get the hostgroup name */
    if ((hostgroup_name = my_strtok(args, ";")) == nullptr)
      return ERROR;

    temp_hostgroup = nullptr;
    hostgroup_map::const_iterator
      it(state::instance().hostgroups().find(hostgroup_name));
    if (it != state::instance().hostgroups().end())
      temp_hostgroup = it->second.get();
    /* verify that the hostgroup is valid */
    if (temp_hostgroup == nullptr)
      return ERROR;
  }

  else if (cmd == CMD_SCHEDULE_SERVICEGROUP_HOST_DOWNTIME
           || cmd == CMD_SCHEDULE_SERVICEGROUP_SVC_DOWNTIME) {

    /* get the servicegroup name */
    if ((servicegroup_name = my_strtok(args, ";")) == nullptr)
      return ERROR;

    /* verify that the servicegroup is valid */
    if ((temp_servicegroup = ::find_servicegroup(servicegroup_name)) == nullptr)
      return ERROR;
  }

  else {

    /* get the host name */
    if ((host_name = my_strtok(args, ";")) == nullptr)
      return ERROR;

    /* verify that the host is valid */
    temp_host = nullptr;
    umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it(configuration::applier::state::instance().hosts().find(get_host_id(host_name)));
    if (it != configuration::applier::state::instance().hosts().end())
      temp_host = it->second.get();
    if (temp_host  == nullptr)
      return ERROR;

    /* this is a service downtime */
    if (cmd == CMD_SCHEDULE_SVC_DOWNTIME) {

      /* get the service name */
      if ((svc_description = my_strtok(nullptr, ";")) == nullptr)
        return ERROR;

      /* verify that the service is valid */
      if ((temp_service = find_service(temp_host->get_name().c_str(), svc_description)) == nullptr)
        return ERROR;
    }
  }

  /* get the start time */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  start_time = (time_t)strtoul(temp_ptr, nullptr, 10);

  /* get the end time */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  end_time = (time_t)strtoul(temp_ptr, nullptr, 10);

  /* get the fixed flag */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  fixed = atoi(temp_ptr);

  /* get the trigger id */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  triggered_by = strtoul(temp_ptr, nullptr, 10);

  /* get the duration */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  duration = strtoul(temp_ptr, nullptr, 10);

  /* get the author */
  if ((author = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;

  /* get the comment */
  if ((comment_data = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;

  /* check if flexible downtime demanded and duration set to non-zero.
  ** according to the documentation, a flexible downtime is started
  ** between start and end time and will last for "duration" seconds.
  ** strtoul converts a nullptr value to 0 so if set to 0, bail out as a
  ** duration>0 is needed.
  */
  if ((0 == fixed) && (0 == duration))
    return ERROR;

  /* duration should be auto-calculated, not user-specified */
  if (fixed > 0)
    duration = (unsigned long)(end_time - start_time);

  /* schedule downtime */
  switch (cmd) {

  case CMD_SCHEDULE_HOST_DOWNTIME:
    downtime_manager::instance().schedule_downtime(
      HOST_DOWNTIME,
      host_name,
      nullptr,
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      triggered_by,
      duration,
      &downtime_id);
    break;

  case CMD_SCHEDULE_SVC_DOWNTIME:
    downtime_manager::instance().schedule_downtime(
      SERVICE_DOWNTIME,
      host_name,
      svc_description,
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      triggered_by,
      duration,
      &downtime_id);
    break;

  case CMD_SCHEDULE_HOST_SVC_DOWNTIME:
    for (temp_servicesmember = temp_host->services;
         temp_servicesmember != nullptr;
         temp_servicesmember = temp_servicesmember->next) {
      if ((temp_service = temp_servicesmember->service_ptr) == nullptr)
        continue;
      downtime_manager::instance().schedule_downtime(
        SERVICE_DOWNTIME,
        host_name,
        temp_service->get_description(),
        entry_time,
        author,
        comment_data,
        start_time,
        end_time,
        fixed,
        triggered_by,
        duration,
        &downtime_id);
    }
    break;

  case CMD_SCHEDULE_HOSTGROUP_HOST_DOWNTIME:
    for (host_map::iterator
           it(temp_hostgroup->members.begin()),
           end(temp_hostgroup->members.end());
         it != end;
         ++it)
      downtime_manager::instance().schedule_downtime(
        HOST_DOWNTIME,
        it->first,
        nullptr,
        entry_time,
        author,
        comment_data,
        start_time,
        end_time,
        fixed,
        triggered_by,
        duration,
        &downtime_id);
    break;

  case CMD_SCHEDULE_HOSTGROUP_SVC_DOWNTIME:
    for (host_map::iterator
           it(temp_hostgroup->members.begin()),
           end(temp_hostgroup->members.end());
         it != end;
         ++it) {
      if (it->second == nullptr)
        continue;
      for (temp_servicesmember = it->second->services;
           temp_servicesmember != nullptr;
           temp_servicesmember = temp_servicesmember->next) {
        if ((temp_service = temp_servicesmember->service_ptr) == nullptr)
          continue;
        downtime_manager::instance().schedule_downtime(
          SERVICE_DOWNTIME,
          temp_service->get_hostname(),
          temp_service->get_description(),
          entry_time, author,
          comment_data,
          start_time,
          end_time,
          fixed,
          triggered_by,
          duration,
          &downtime_id);
      }
    }
    break;

  case CMD_SCHEDULE_SERVICEGROUP_HOST_DOWNTIME:
    last_host = nullptr;
    for (temp_sgmember = temp_servicegroup->members;
         temp_sgmember != nullptr;
         temp_sgmember = temp_sgmember->next) {
      temp_host = nullptr;
      umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
        it(configuration::applier::state::instance().hosts().find(get_host_id(temp_sgmember->host_name)));
      if (it != configuration::applier::state::instance().hosts().end())
        temp_host = it->second.get();
      if (temp_host  == nullptr)
        continue;
      if (last_host == temp_host)
        continue;
      downtime_manager::instance().schedule_downtime(
        HOST_DOWNTIME,
        temp_sgmember->host_name,
        nullptr,
        entry_time,
        author,
        comment_data,
        start_time,
        end_time,
        fixed,
        triggered_by,
        duration,
        &downtime_id);
      last_host = temp_host;
    }
    break;

  case CMD_SCHEDULE_SERVICEGROUP_SVC_DOWNTIME:
    for (temp_sgmember = temp_servicegroup->members;
         temp_sgmember != nullptr;
         temp_sgmember = temp_sgmember->next)
      downtime_manager::instance().schedule_downtime(
        SERVICE_DOWNTIME,
        temp_sgmember->host_name,
        temp_sgmember->service_description,
        entry_time, author,
        comment_data,
        start_time,
        end_time,
        fixed,
        triggered_by,
        duration,
        &downtime_id);
    break;

  case CMD_SCHEDULE_AND_PROPAGATE_HOST_DOWNTIME:
    /* schedule downtime for "parent" host */
    downtime_manager::instance().schedule_downtime(
      HOST_DOWNTIME,
      host_name,
      nullptr,
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      triggered_by,
      duration,
      &downtime_id);

    /* schedule (non-triggered) downtime for all child hosts */
    schedule_and_propagate_downtime(
      temp_host,
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      0,
      duration);
    break;

  case CMD_SCHEDULE_AND_PROPAGATE_TRIGGERED_HOST_DOWNTIME:
    /* schedule downtime for "parent" host */
    downtime_manager::instance().schedule_downtime(
      HOST_DOWNTIME,
      host_name,
      nullptr,
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      triggered_by,
      duration,
      &downtime_id);

    /* schedule triggered downtime for all child hosts */
    schedule_and_propagate_downtime(
      temp_host,
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      downtime_id,
      duration);
    break;

  default:
    break;
  }
  return OK;
}

/* deletes scheduled host or service downtime */
int cmd_delete_downtime(int cmd, char* args) {
  uint64_t downtime_id(0);
  char* temp_ptr(nullptr);

  /* Get the id of the downtime to delete. */
  if (nullptr == (temp_ptr = my_strtok(args,"\n")))
    return ERROR;

  downtime_id = strtoul(temp_ptr, nullptr, 10);

  if (CMD_DEL_HOST_DOWNTIME == cmd)
    downtime_manager::instance().unschedule_downtime(HOST_DOWNTIME, downtime_id);
  else
    downtime_manager::instance().unschedule_downtime(SERVICE_DOWNTIME, downtime_id);

  return OK;
}

/**
 *  Delete scheduled host or service downtime, according to some criterias.
 *
 *  @param[in] cmd   Command ID.
 *  @param[in] args  Command arguments.
 */
int cmd_delete_downtime_full(int cmd, char* args) {
  char* temp_ptr(nullptr);
  downtime_finder::criteria_set criterias;

  // Host name.
  if (!(temp_ptr = my_strtok(args, ";")))
    return ERROR;
  if (*temp_ptr)
    criterias.push_back(downtime_finder::criteria("host", temp_ptr));
  // Service description and downtime type.
  int downtime_type;
  if (cmd == CMD_DEL_SVC_DOWNTIME_FULL) {
    downtime_type = SERVICE_DOWNTIME;
    if (!(temp_ptr = my_strtok(nullptr, ";")))
      return ERROR;
    if (*temp_ptr)
      criterias.push_back(downtime_finder::criteria("service", temp_ptr));
  }
  else {
    downtime_type = HOST_DOWNTIME;
    criterias.push_back(downtime_finder::criteria("service", ""));
  }
  // Start time.
  if (!(temp_ptr = my_strtok(nullptr, ";")))
    return ERROR;
  if (*temp_ptr)
    criterias.push_back(downtime_finder::criteria("start", temp_ptr));
  // End time.
  if (!(temp_ptr = my_strtok(nullptr, ";")))
    return ERROR;
  if (*temp_ptr)
    criterias.push_back(downtime_finder::criteria("end", temp_ptr));
  // Fixed.
  if (!(temp_ptr = my_strtok(nullptr, ";")))
    return ERROR;
  if (*temp_ptr)
    criterias.push_back(downtime_finder::criteria("fixed", temp_ptr));
  // Trigger ID.
  if (!(temp_ptr = my_strtok(nullptr, ";")))
    return ERROR;
  if (*temp_ptr)
    criterias.push_back(downtime_finder::criteria("triggered_by", temp_ptr));
  // Duration.
  if (!(temp_ptr = my_strtok(nullptr, ";")))
    return ERROR;
  if (*temp_ptr)
    criterias.push_back(downtime_finder::criteria("duration", temp_ptr));
  // Author.
  if (!(temp_ptr = my_strtok(nullptr, ";")))
    return ERROR;
  if (*temp_ptr)
    criterias.push_back(downtime_finder::criteria("author", temp_ptr));
  // Comment.
  if (!(temp_ptr = my_strtok(nullptr, ";")))
    return ERROR;
  if (*temp_ptr)
    criterias.push_back(downtime_finder::criteria("comment", temp_ptr));

  // Find downtimes.
  downtime_finder dtf(downtimes::downtime_manager::instance().get_scheduled_downtimes());
  downtime_finder::result_set result(dtf.find_matching_all(criterias));
  for (downtime_finder::result_set::const_iterator
         it(result.begin()), end(result.end());
       it != end;
       ++it) {
    downtime_manager::instance().unschedule_downtime(downtime_type, *it);
  }

  return OK;
}

/*
** Some of these commands are now "distributable" as no downtime ids are
** used. Deletes scheduled host and service downtime based on hostname
** and optionally other filter arguments.
*/
int cmd_delete_downtime_by_host_name(int cmd, char* args) {
  char *temp_ptr(nullptr);
  char *end_ptr(nullptr);
  char *hostname(nullptr);
  char *service_description(nullptr);
  char *downtime_comment(nullptr);
  time_t downtime_start_time(0L);
  int deleted(0);

  (void)cmd;

  /* Get the host name of the downtime to delete. */
  temp_ptr = my_strtok(args, ";");
  if (nullptr == temp_ptr)
    return ERROR;
  hostname = temp_ptr;

  /* Get the optional service name. */
  temp_ptr = my_strtok(nullptr,";");
  if (temp_ptr != nullptr) {
    if (*temp_ptr != '\0')
      service_description = temp_ptr;

    /* Get the optional start time. */
    temp_ptr = my_strtok(nullptr, ";");
    if (temp_ptr != nullptr) {
      downtime_start_time = strtoul(temp_ptr, &end_ptr, 10);

      /* Get the optional comment. */
      temp_ptr = my_strtok(nullptr, ";");
      if (temp_ptr != nullptr) {
        if (*temp_ptr != '\0')
          downtime_comment = temp_ptr;
      }
    }
  }

  deleted = downtime_manager::instance().delete_downtime_by_hostname_service_description_start_time_comment(
              hostname,
              service_description,
              downtime_start_time,
              downtime_comment);
  if (0 == deleted)
    return ERROR;
  return OK;
}

/* Deletes scheduled host and service downtime based on hostgroup and optionally other filter arguments. */
int cmd_delete_downtime_by_hostgroup_name(int cmd, char* args) {
  char *temp_ptr(nullptr);
  char *end_ptr(nullptr);
  host *temp_host(nullptr);
  hostgroup *temp_hostgroup(nullptr);
  char *service_description(nullptr);
  char *downtime_comment(nullptr);
  char *host_name(nullptr);
  time_t downtime_start_time(0L);
  int deleted(0);

  (void)cmd;

  /* Get the host group name of the downtime to delete. */
  temp_ptr = my_strtok(args, ";");
  if (nullptr == temp_ptr)
    return ERROR;

  temp_hostgroup = nullptr;
  hostgroup_map::const_iterator
    it(state::instance().hostgroups().find(temp_ptr));
  if (it != state::instance().hostgroups().end())
    temp_hostgroup = it->second.get();
  if (temp_hostgroup == nullptr)
    return ERROR;

  /* Get the optional host name. */
  temp_ptr = my_strtok(nullptr, ";");
  if (temp_ptr != nullptr) {
    if (*temp_ptr != '\0')
      host_name = temp_ptr;

    /* Get the optional service name. */
    temp_ptr = my_strtok(nullptr, ";");
    if (temp_ptr != nullptr) {
      if (*temp_ptr != '\0')
        service_description = temp_ptr;

      /* Get the optional start time. */
      temp_ptr = my_strtok(nullptr, ";");
      if (temp_ptr != nullptr) {
        downtime_start_time = strtoul(temp_ptr, &end_ptr, 10);

        /* Get the optional comment. */
        temp_ptr = my_strtok(nullptr, ";");
        if (temp_ptr != nullptr) {
          if (*temp_ptr != '\0')
            downtime_comment = temp_ptr;
        }
      }
    }

    /* Get the optional service name. */
    temp_ptr = my_strtok(nullptr, ";");
    if (temp_ptr != nullptr) {
      if (*temp_ptr != '\0')
        service_description = temp_ptr;

      /* Get the optional start time. */
      temp_ptr = my_strtok(nullptr, ";");
      if (temp_ptr != nullptr) {
        downtime_start_time = strtoul(temp_ptr, &end_ptr, 10);

        /* Get the optional comment. */
        temp_ptr = my_strtok(nullptr, ";");
        if (temp_ptr != nullptr) {
          if (*temp_ptr != '\0')
            downtime_comment = temp_ptr;
        }
      }
    }
  }

  for (host_map::iterator
         it(temp_hostgroup->members.begin()),
         end(temp_hostgroup->members.end());
       it != end;
       ++it) {
    if (it->second == nullptr)
      continue ;
    if (host_name != nullptr && it->first != host_name)
      continue ;
    deleted = downtime_manager::instance().delete_downtime_by_hostname_service_description_start_time_comment(
                it->first.c_str(),
                service_description,
                downtime_start_time,
                downtime_comment);
  }

  if (0 == deleted)
    return ERROR;

  return OK;
}

/* Delete downtimes based on start time and/or comment. */
int cmd_delete_downtime_by_start_time_comment(int cmd, char* args){
  time_t downtime_start_time(0L);
  char *downtime_comment(nullptr);
  char *temp_ptr(nullptr);
  char *end_ptr(nullptr);
  int deleted(0);

  (void)cmd;

  /* Get start time if set. */
  temp_ptr = my_strtok(args, ";");
  if (temp_ptr != nullptr)
    /* This will be set to 0 if no start_time is entered or data is bad. */
    downtime_start_time = strtoul(temp_ptr, &end_ptr, 10);

  /* Get comment - not sure if this should be also tokenised by ; */
  temp_ptr = my_strtok(nullptr, "\n");
  if ((temp_ptr != nullptr) && (*temp_ptr != '\0'))
    downtime_comment = temp_ptr;

  /* No args should give an error. */
  if ((0 == downtime_start_time) && (nullptr == downtime_comment))
    return ERROR;

  deleted = downtime_manager::instance().delete_downtime_by_hostname_service_description_start_time_comment(
              "",
              nullptr,
              downtime_start_time,
              downtime_comment);

  if (0 == deleted)
    return ERROR;

  return OK;
}

/* changes a host or service (integer) variable */
int cmd_change_object_int_var(int cmd, char* args) {
  com::centreon::engine::service* temp_service(nullptr);
  com::centreon::engine::host* temp_host(nullptr);
  contact* temp_contact(nullptr);
  char* host_name(nullptr);
  char* svc_description(nullptr);
  char* contact_name(nullptr);
  char const* temp_ptr(nullptr);
  int intval(0);
  double dval(0.0);
  double old_dval(0.0);
  time_t preferred_time(0);
  time_t next_valid_time(0);
  unsigned long attr(MODATTR_NONE);
  unsigned long hattr(MODATTR_NONE);
  unsigned long sattr(MODATTR_NONE);
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it;

  switch (cmd) {
    case CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL:
    case CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL:
    case CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS:
    case CMD_CHANGE_SVC_MODATTR:

      /* get the host name */
      if ((host_name = my_strtok(args, ";")) == nullptr)
        return ERROR;

      /* get the service name */
      if ((svc_description = my_strtok(nullptr, ";")) == nullptr)
        return ERROR;

      /* verify that the service is valid */
      if ((temp_service = find_service(host_name, svc_description)) == nullptr)
        return ERROR;
      break;

    case CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL:
    case CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL:
    case CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS:
    case CMD_CHANGE_HOST_MODATTR:
      /* get the host name */
      if ((host_name = my_strtok(args, ";")) == nullptr)
        return ERROR;

      /* verify that the host is valid */
      temp_host = nullptr;
      it = configuration::applier::state::instance().hosts().find(
          get_host_id(host_name));
      if (it != configuration::applier::state::instance().hosts().end())
        temp_host = it->second.get();
      if (temp_host == nullptr)
        return ERROR;
      break;

    case CMD_CHANGE_CONTACT_MODATTR:
    case CMD_CHANGE_CONTACT_MODHATTR:
    case CMD_CHANGE_CONTACT_MODSATTR:
      /* get the contact name */
      if ((contact_name = my_strtok(args, ";")) == nullptr)
        return ERROR;

      /* verify that the contact is valid */
      if ((temp_contact =
               configuration::applier::state::instance().find_contact(
                   contact_name)) == nullptr)
        return ERROR;
      break;

    default:
      /* unknown command */
      return ERROR;
      break;
  }

  /* get the value */
  if ((temp_ptr = my_strtok(nullptr, ";")) == nullptr)
    return ERROR;
  intval = (int)strtol(temp_ptr, nullptr, 0);
  if (intval < 0 || (intval == 0 && errno == EINVAL))
    return ERROR;
  dval = (int)strtod(temp_ptr, nullptr);

  switch (cmd) {

  case CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL:
    /* save the old check interval */
    old_dval = temp_host->get_check_interval();

    /* modify the check interval */
    temp_host->set_check_interval(dval);
    attr = MODATTR_NORMAL_CHECK_INTERVAL;

    /* schedule a host check if previous interval was 0 (checks were not regularly scheduled) */
    if (old_dval == 0 && temp_host->get_checks_enabled()) {

      /* set the host check flag */
      temp_host->set_should_be_scheduled(true);

      /* schedule a check for right now (or as soon as possible) */
      time(&preferred_time);
      if (check_time_against_period(
            preferred_time,
            temp_host->check_period_ptr) == ERROR) {
        get_next_valid_time(
          preferred_time,
          &next_valid_time,
          temp_host->check_period_ptr);
        temp_host->set_next_check(next_valid_time);
      }
      else
        temp_host->set_next_check(preferred_time);

      /* schedule a check if we should */
      if (temp_host->get_should_be_scheduled())
        schedule_host_check(temp_host, temp_host->get_next_check(), CHECK_OPTION_NONE);
    }
    break;

  case CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL:
    temp_host->set_retry_interval(dval);
    attr = MODATTR_RETRY_CHECK_INTERVAL;
    break;

  case CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS:
    temp_host->set_max_attempts(intval);
    attr = MODATTR_MAX_CHECK_ATTEMPTS;

    /* adjust current attempt number if in a hard state */
    if (temp_host->get_state_type() == HARD_STATE
        && temp_host->get_current_state() != HOST_UP
        && temp_host->get_current_attempt() > 1)
      temp_host->set_current_attempt(temp_host->get_max_attempts());
    break;

  case CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL:
    /* save the old check interval */
    old_dval = temp_service->get_check_interval();

    /* modify the check interval */
    temp_service->set_check_interval(dval);
    attr = MODATTR_NORMAL_CHECK_INTERVAL;

    /* schedule a service check if previous interval was 0 (checks were not regularly scheduled) */
    if (old_dval == 0 && temp_service->checks_enabled
        && temp_service->get_check_interval() != 0) {

      /* set the service check flag */
      temp_service->should_be_scheduled = true;

      /* schedule a check for right now (or as soon as possible) */
      time(&preferred_time);
      if (check_time_against_period(
            preferred_time,
            temp_service->check_period_ptr) == ERROR) {
        get_next_valid_time(
          preferred_time,
          &next_valid_time,
          temp_service->check_period_ptr);
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
  case CMD_CHANGE_CONTACT_MODATTR:
    attr = intval;
    break;

  case CMD_CHANGE_CONTACT_MODHATTR:
    hattr = intval;
    break;

  case CMD_CHANGE_CONTACT_MODSATTR:
    sattr = intval;
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
      nullptr);

    /* update the status log with the service info */
    update_service_status(temp_service, false);
    break;

  case CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL:
  case CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL:
  case CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS:
  case CMD_CHANGE_HOST_MODATTR:
    /* set the modified host attribute */
    if (cmd == CMD_CHANGE_HOST_MODATTR)
      temp_host->set_modified_attributes(attr);
    else
      temp_host->set_modified_attributes(
        temp_host->get_modified_attributes() | attr);

    /* send data to event broker */
    broker_adaptive_host_data(
      NEBTYPE_ADAPTIVEHOST_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      temp_host,
      cmd,
      attr,
      temp_host->get_modified_attributes(),
      nullptr);

    /* update the status log with the host info */
    update_host_status(temp_host, false);
    break;

  case CMD_CHANGE_CONTACT_MODATTR:
  case CMD_CHANGE_CONTACT_MODHATTR:
  case CMD_CHANGE_CONTACT_MODSATTR:
    /* set the modified attribute */
    switch (cmd) {
    case CMD_CHANGE_CONTACT_MODATTR:
      temp_contact->set_modified_attributes(attr);
      break;

    case CMD_CHANGE_CONTACT_MODHATTR:
      temp_contact->set_modified_host_attributes(hattr);
      break;

    case CMD_CHANGE_CONTACT_MODSATTR:
      temp_contact->set_modified_service_attributes(sattr);
      break;

    default:
      break;
    }

    /* send data to event broker */
    broker_adaptive_contact_data(
      NEBTYPE_ADAPTIVECONTACT_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      temp_contact,
      cmd, attr,
      temp_contact->get_modified_attributes(),
      hattr,
      temp_contact->get_modified_host_attributes(),
      sattr,
      temp_contact->get_modified_service_attributes(),
      nullptr);

    /* update the status log with the contact info */
    temp_contact->update_status_info(false);
    break;

  default:
    break;
  }

  return OK;
}

/* changes a host or service (char) variable */
int cmd_change_object_char_var(int cmd, char* args) {
  com::centreon::engine::service* temp_service{nullptr};
  com::centreon::engine::host* temp_host{nullptr};
  contact* temp_contact{nullptr};
  timeperiod* temp_timeperiod{nullptr};
  commands::command* temp_command{nullptr};
  char* host_name{nullptr};
  char* svc_description{nullptr};
  char* contact_name{nullptr};
  char* charval{nullptr};
  char* temp_ptr{nullptr};
  char* temp_ptr2{nullptr};
  unsigned long attr{MODATTR_NONE};
  unsigned long hattr{MODATTR_NONE};
  unsigned long sattr{MODATTR_NONE};
  umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it;

  /* SECURITY PATCH - disable these for the time being */
  switch (cmd) {
  case CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER:
  case CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER:
  case CMD_CHANGE_HOST_EVENT_HANDLER:
  case CMD_CHANGE_SVC_EVENT_HANDLER:
  case CMD_CHANGE_HOST_CHECK_COMMAND:
  case CMD_CHANGE_SVC_CHECK_COMMAND:
    return ERROR;
  }

  /* get the command arguments */
  switch (cmd) {

  case CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER:
  case CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER:
    if ((charval = my_strtok(args, "\n")) == nullptr)
      return ERROR;
    break;

  case CMD_CHANGE_HOST_EVENT_HANDLER:
  case CMD_CHANGE_HOST_CHECK_COMMAND:
  case CMD_CHANGE_HOST_CHECK_TIMEPERIOD:
  case CMD_CHANGE_HOST_NOTIFICATION_TIMEPERIOD:
    /* get the host name */
    if ((host_name = my_strtok(args, ";")) == nullptr)
      return ERROR;

    /* verify that the host is valid */
    temp_host = nullptr;
    it = configuration::applier::state::instance().hosts().find(get_host_id(host_name));
    if (it != configuration::applier::state::instance().hosts().end())
      temp_host = it->second.get();
    if (temp_host  == nullptr)
      return ERROR;

    if ((charval = my_strtok(nullptr, "\n")) == nullptr)
      return ERROR;
    break;

  case CMD_CHANGE_SVC_EVENT_HANDLER:
  case CMD_CHANGE_SVC_CHECK_COMMAND:
  case CMD_CHANGE_SVC_CHECK_TIMEPERIOD:
  case CMD_CHANGE_SVC_NOTIFICATION_TIMEPERIOD:
    /* get the host name */
    if ((host_name = my_strtok(args, ";")) == nullptr)
      return ERROR;

    /* get the service name */
    if ((svc_description = my_strtok(nullptr, ";")) == nullptr)
      return ERROR;

    /* verify that the service is valid */
    if ((temp_service = find_service(host_name, svc_description)) == nullptr)
      return ERROR;

    if ((charval = my_strtok(nullptr, "\n")) == nullptr)
      return ERROR;
    break;

  case CMD_CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD:
  case CMD_CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD:
    /* get the contact name */
    if ((contact_name = my_strtok(args, ";")) == nullptr)
      return ERROR;

    /* verify that the contact is valid */
    if ((temp_contact = configuration::applier::state::instance().find_contact(contact_name)) == nullptr)
      return ERROR;

    if ((charval = my_strtok(nullptr, "\n")) == nullptr)
      return ERROR;
    break;

  default:
    /* invalid command */
    return ERROR;
    break;
  }

  temp_ptr = string::dup(charval);

  /* do some validation */
  switch (cmd) {

  case CMD_CHANGE_HOST_CHECK_TIMEPERIOD:
  case CMD_CHANGE_SVC_CHECK_TIMEPERIOD:
  case CMD_CHANGE_HOST_NOTIFICATION_TIMEPERIOD:
  case CMD_CHANGE_SVC_NOTIFICATION_TIMEPERIOD:
  case CMD_CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD:
  case CMD_CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD:
    /* make sure the timeperiod is valid */
    if ((temp_timeperiod = find_timeperiod(temp_ptr)) == nullptr) {
      delete[] temp_ptr;
      return ERROR;
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
    if ((temp_command = configuration::applier::state::instance().find_command(temp_ptr2)) == nullptr) {
      delete[] temp_ptr;
      return ERROR;
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
    temp_host->set_event_handler(temp_ptr);
    temp_host->event_handler_ptr = temp_command;
    attr = MODATTR_EVENT_HANDLER_COMMAND;
    break;

  case CMD_CHANGE_HOST_CHECK_COMMAND:
    temp_host->set_check_command(temp_ptr);
    temp_host->check_command_ptr = temp_command;
    attr = MODATTR_CHECK_COMMAND;
    break;

  case CMD_CHANGE_HOST_CHECK_TIMEPERIOD:
    temp_host->set_check_period(temp_ptr);
    temp_host->check_period_ptr = temp_timeperiod;
    attr = MODATTR_CHECK_TIMEPERIOD;
    break;

  case CMD_CHANGE_HOST_NOTIFICATION_TIMEPERIOD:
    temp_host->set_notification_period(temp_ptr);
    delete[] temp_ptr;
    temp_host->notification_period_ptr = temp_timeperiod;
    attr = MODATTR_NOTIFICATION_TIMEPERIOD;
    break;

  case CMD_CHANGE_SVC_EVENT_HANDLER:
    delete[] temp_service->event_handler;
    temp_service->event_handler = temp_ptr;
    temp_service->event_handler_ptr = temp_command;
    attr = MODATTR_EVENT_HANDLER_COMMAND;
    break;

  case CMD_CHANGE_SVC_CHECK_COMMAND:
    temp_service->set_check_command(temp_ptr);
    delete[] temp_ptr;
    temp_service->check_command_ptr = temp_command;
    attr = MODATTR_CHECK_COMMAND;
    break;

  case CMD_CHANGE_SVC_CHECK_TIMEPERIOD:
    delete[] temp_service->check_period;
    temp_service->check_period = temp_ptr;
    temp_service->check_period_ptr = temp_timeperiod;
    attr = MODATTR_CHECK_TIMEPERIOD;
    break;

  case CMD_CHANGE_SVC_NOTIFICATION_TIMEPERIOD:
    delete[] temp_service->notification_period;
    temp_service->notification_period = temp_ptr;
    temp_service->notification_period_ptr = temp_timeperiod;
    attr = MODATTR_NOTIFICATION_TIMEPERIOD;
    break;

  case CMD_CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD:
    temp_contact->set_host_notification_period(temp_ptr);
    temp_contact->host_notification_period_ptr = temp_timeperiod;
    hattr = MODATTR_NOTIFICATION_TIMEPERIOD;
    break;

  case CMD_CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD:
    temp_contact->set_service_notification_period(temp_ptr);
    temp_contact->service_notification_period_ptr = temp_timeperiod;
    sattr = MODATTR_NOTIFICATION_TIMEPERIOD;
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
      nullptr);
    /* update program status */
    update_program_status(false);
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
      nullptr);

    /* update program status */
    update_program_status(false);
    break;

  case CMD_CHANGE_SVC_EVENT_HANDLER:
  case CMD_CHANGE_SVC_CHECK_COMMAND:
  case CMD_CHANGE_SVC_CHECK_TIMEPERIOD:
  case CMD_CHANGE_SVC_NOTIFICATION_TIMEPERIOD:

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
      nullptr);

    /* update the status log with the service info */
    update_service_status(temp_service, false);
    break;

  case CMD_CHANGE_HOST_EVENT_HANDLER:
  case CMD_CHANGE_HOST_CHECK_COMMAND:
  case CMD_CHANGE_HOST_CHECK_TIMEPERIOD:
  case CMD_CHANGE_HOST_NOTIFICATION_TIMEPERIOD:
    /* set the modified host attribute */
    temp_host->set_modified_attributes(
      temp_host->get_modified_attributes() | attr);

    /* send data to event broker */
    broker_adaptive_host_data(
      NEBTYPE_ADAPTIVEHOST_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      temp_host,
      cmd,
      attr,
      temp_host->get_modified_attributes(),
      nullptr);

    /* update the status log with the host info */
    update_host_status(temp_host, false);
    break;

  case CMD_CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD:
  case CMD_CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD:
    /* set the modified attributes */
    temp_contact->set_modified_host_attributes(
                    temp_contact->get_modified_host_attributes()
                    | hattr);
    temp_contact->set_modified_service_attributes(
                    temp_contact->get_modified_service_attributes()
                    | sattr);

    /* send data to event broker */
    broker_adaptive_contact_data(
      NEBTYPE_ADAPTIVECONTACT_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      temp_contact,
      cmd,
      attr,
      temp_contact->get_modified_attributes(),
      hattr,
      temp_contact->get_modified_host_attributes(),
      sattr,
      temp_contact->get_modified_service_attributes(),
      nullptr);

    /* update the status log with the contact info */
    temp_contact->update_status_info(false);
    break;

  default:
    break;
  }

  return OK;
}

/* changes a custom host or service variable */
int cmd_change_object_custom_var(int cmd, char* args) {
  com::centreon::engine::host* temp_host(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  contact* temp_contact(nullptr);

  /* get the host or contact name */
  char* temp_ptr(index(args, ';'));
  if (!temp_ptr)
    return ERROR;
  int pos(temp_ptr - args);
  std::string name1(args, pos);
  args += pos + 1;

  /* get the service name */
  std::string name2;
  if (cmd == CMD_CHANGE_CUSTOM_SVC_VAR) {
    temp_ptr = index(args, ';');
    if (!temp_ptr)
      return ERROR;
    pos = temp_ptr - args;
    name2 = std::string(args, pos);
    args += pos + 1;
  }

  /* get the custom variable name */
  temp_ptr = index(args, ';');
  if (!temp_ptr)
    return ERROR;
  pos = temp_ptr - args;
  std::string varname(args, pos);
  args += pos + 1;

  /* get the custom variable value */
  temp_ptr = index(args, ';');
  std::string varvalue;
  if (temp_ptr) {
    pos = temp_ptr - args;
    varvalue = std::string(args, pos);
  }

  std::transform(varname.begin(), varname.end(), varname.begin(), ::toupper);

  /* find the object */
  switch (cmd) {
  case CMD_CHANGE_CUSTOM_HOST_VAR:
    {
      temp_host = nullptr;
      umap<uint64_t, std::shared_ptr<com::centreon::engine::host>>::const_iterator
      it_h(configuration::applier::state::instance().hosts().find(get_host_id(name1.c_str())));
      if (it_h != configuration::applier::state::instance().hosts().end())
        temp_host = it_h->second.get();
      if (temp_host  == nullptr)
        return ERROR;
      std::unordered_map<std::string, customvariable>::iterator it(temp_host->custom_variables.find(varname));
      if (it == temp_host->custom_variables.end())
        temp_host->custom_variables.insert({std::move(varname), customvariable(std::move(varvalue))});
      else
        it->second.update(std::move(varvalue));

      /* set the modified attributes and update the status of the object */
      temp_host->set_modified_attributes(
        temp_host->get_modified_attributes() | MODATTR_CUSTOM_VARIABLE);
      update_host_status(temp_host, false);
    }
    break;
  case CMD_CHANGE_CUSTOM_SVC_VAR:
    {
      if ((temp_service = find_service(name1.c_str(), name2.c_str())) == nullptr)
        return ERROR;
      std::unordered_map<std::string, customvariable>::iterator it(temp_service->custom_variables.find(varname));
      if (it == temp_service->custom_variables.end())
        temp_service->custom_variables.insert({std::move(varname), customvariable(std::move(varvalue))});
      else
        it->second.update(std::move(varvalue));

      /* set the modified attributes and update the status of the object */
      temp_service->custom_variables.insert(
          {std::move(varname), customvariable(std::move(varvalue))});
      temp_service->modified_attributes |= MODATTR_CUSTOM_VARIABLE;
      update_service_status(temp_service, false);
    }
    break;
  case CMD_CHANGE_CUSTOM_CONTACT_VAR:
    {
      if ((temp_contact = configuration::applier::state::instance().find_contact(name1)) == nullptr)
        return ERROR;
      std::unordered_map<std::string, customvariable>::iterator it(temp_contact->custom_variables.find(varname));
      if (it == temp_contact->custom_variables.end())
        temp_contact->custom_variables.insert({std::move(varname), customvariable(std::move(varvalue))});
      else
        it->second.update(std::move(varvalue));

      /* set the modified attributes and update the status of the object */
      temp_contact->custom_variables.insert(
          {std::move(varname), customvariable(std::move(varvalue))});
      temp_contact->set_modified_attributes(
          temp_contact->get_modified_attributes()
          | MODATTR_CUSTOM_VARIABLE);
      temp_contact->update_status_info(false);
    }
    break;
  default:
    break;
  }

  return OK;
}

/* processes an external host command */
int cmd_process_external_commands_from_file(int cmd, char* args) {
  char* fname(nullptr);
  char* temp_ptr(nullptr);
  int delete_file(false);

  (void)cmd;

  /* get the file name */
  if ((temp_ptr = my_strtok(args, ";")) == nullptr)
    return ERROR;
  fname = string::dup(temp_ptr);

  /* find the deletion option */
  if ((temp_ptr = my_strtok(nullptr, "\n")) == nullptr) {
    delete[] fname;
    return ERROR;
  }
  if (atoi(temp_ptr) == 0)
    delete_file = false;
  else
    delete_file = true;

  /* process the file */
  process_external_commands_from_file(fname, delete_file);

  /* free memory */
  delete[] fname;

  return OK;
}

/******************************************************************/
/*************** INTERNAL COMMAND IMPLEMENTATIONS  ****************/
/******************************************************************/

/* temporarily disables a service check */
void disable_service_checks(com::centreon::engine::service* svc) {
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
    nullptr);

  /* update the status log to reflect the new service state */
  update_service_status(svc, false);
}

/* enables a service check */
void enable_service_checks(com::centreon::engine::service* svc) {
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
  if (svc->get_check_interval() == 0)
    svc->should_be_scheduled = false;

  /* schedule a check for right now (or as soon as possible) */
  time(&preferred_time);
  if (check_time_against_period(
        preferred_time,
        svc->check_period_ptr) == ERROR) {
    get_next_valid_time(
      preferred_time,
      &next_valid_time,
      svc->check_period_ptr);
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
    nullptr);

  /* update the status log to reflect the new service state */
  update_service_status(svc, false);
}

/* enable notifications on a program-wide basis */
void enable_all_notifications(void) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* bail out if we're already set... */
  if (config->enable_notifications())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* update notification status */
  config->enable_notifications(true);

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
    nullptr);

  /* update the status log */
  update_program_status(false);
}

/* disable notifications on a program-wide basis */
void disable_all_notifications(void) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* bail out if we're already set... */
  if (config->enable_notifications() == false)
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  /* update notification status */
  config->enable_notifications(false);

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
    nullptr);

  /* update the status log */
  update_program_status(false);
}

/* enables notifications for a service */
void enable_service_notifications(com::centreon::engine::service* svc) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (svc->notifications_enabled)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* enable the service notifications... */
  svc->notifications_enabled = true;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    nullptr);

  /* update the status log to reflect the new service state */
  update_service_status(svc, false);
}

/* disables notifications for a service */
void disable_service_notifications(com::centreon::engine::service* svc) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (svc->notifications_enabled == false)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* disable the service notifications... */
  svc->notifications_enabled = false;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    nullptr);

  /* update the status log to reflect the new service state */
  update_service_status(svc, false);
}

/* enables notifications for a host */
void enable_host_notifications(com::centreon::engine::host* hst) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (hst->get_notifications_enabled())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* enable the host notifications... */
  hst->set_notifications_enabled(true);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log to reflect the new host state */
  update_host_status(hst, false);
}

/* disables notifications for a host */
void disable_host_notifications(com::centreon::engine::host* hst) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (!hst->get_notifications_enabled())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* disable the host notifications... */
  hst->set_notifications_enabled(false);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log to reflect the new host state */
  update_host_status(hst, false);
}

/* enables notifications for all hosts and services "beyond" a given host */
void enable_and_propagate_notifications(
       com::centreon::engine::host* hst,
       int level,
       int affect_top_host,
       int affect_hosts,
       int affect_services) {
  com::centreon::engine::host* child_host(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  servicesmember* temp_servicesmember(nullptr);

  /* enable notification for top level host */
  if (affect_top_host && level == 0)
    enable_host_notifications(hst);

  /* check all child hosts... */
  for (host_map::iterator
         it(hst->child_hosts.begin()),
         end(hst->child_hosts.end());
       it != end;
       ++it) {

    if (it->second == nullptr)
      continue;

    /* recurse... */
    enable_and_propagate_notifications(
      it->second.get(),
      level + 1,
      affect_top_host,
      affect_hosts,
      affect_services);

    /* enable notifications for this host */
    if (affect_hosts)
      enable_host_notifications(it->second.get());

    /* enable notifications for all services on this host... */
    if (affect_services) {
      for (temp_servicesmember = it->second->services;
           temp_servicesmember != nullptr;
           temp_servicesmember = temp_servicesmember->next) {
        if ((temp_service = temp_servicesmember->service_ptr) == nullptr)
          continue;
        enable_service_notifications(temp_service);
      }
    }
  }
}

/* disables notifications for all hosts and services "beyond" a given host */
void disable_and_propagate_notifications(
       com::centreon::engine::host* hst,
       int level,
       int affect_top_host,
       int affect_hosts,
       int affect_services) {
  com::centreon::engine::host* child_host(nullptr);
  com::centreon::engine::service* temp_service(nullptr);
  servicesmember* temp_servicesmember(nullptr);

  if (hst == nullptr)
    return;

  /* disable notifications for top host */
  if (affect_top_host && level == 0)
    disable_host_notifications(hst);

  /* check all child hosts... */
  for (host_map::iterator
         it(hst->child_hosts.begin()),
         end(hst->child_hosts.begin());
       it != end;
       ++it) {

    if (it->second == nullptr)
      continue;

    /* recurse... */
    disable_and_propagate_notifications(
      it->second.get(),
      level + 1,
      affect_top_host,
      affect_hosts,
      affect_services);

    /* disable notifications for this host */
    if (affect_hosts)
      disable_host_notifications(it->second.get());

    /* disable notifications for all services on this host... */
    if (affect_services) {
      for (temp_servicesmember = it->second->services;
           temp_servicesmember != nullptr;
           temp_servicesmember = temp_servicesmember->next) {
        if ((temp_service = temp_servicesmember->service_ptr) == nullptr)
          continue;
        disable_service_notifications(temp_service);
      }
    }
  }
}

/* enables host notifications for a contact */
void enable_contact_host_notifications(contact* cntct) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (cntct->get_host_notifications_enabled())
    return;

  /* set the attribute modified flag */
  cntct->set_modified_host_attributes(
           cntct->get_modified_host_attributes() | attr);

  /* enable the host notifications... */
  cntct->set_host_notifications_enabled(true);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    cntct,
    CMD_NONE,
    MODATTR_NONE,
    cntct->get_modified_attributes(),
    attr,
    cntct->get_modified_host_attributes(),
    MODATTR_NONE,
    cntct->get_modified_service_attributes(),
    nullptr);

  /* update the status log to reflect the new contact state */
  cntct->update_status_info(false);
}

/* disables host notifications for a contact */
void disable_contact_host_notifications(contact* cntct) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (!cntct->get_host_notifications_enabled())
    return;

  /* set the attribute modified flag */
  cntct->set_modified_host_attributes(
      cntct->get_modified_host_attributes()
      | attr);

  /* enable the host notifications... */
  cntct->set_host_notifications_enabled(false);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    cntct,
    CMD_NONE,
    MODATTR_NONE,
    cntct->get_modified_attributes(),
    attr,
    cntct->get_modified_host_attributes(),
    MODATTR_NONE,
    cntct->get_modified_service_attributes(),
    nullptr);

  /* update the status log to reflect the new contact state */
  cntct->update_status_info(false);
}

/* enables service notifications for a contact */
void enable_contact_service_notifications(contact* cntct) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (cntct->get_service_notifications_enabled())
    return;

  /* set the attribute modified flag */
  cntct->set_modified_service_attributes(
      cntct->get_modified_service_attributes()
      | attr);

  /* enable the host notifications... */
  cntct->set_service_notifications_enabled(true);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    cntct,
    CMD_NONE,
    MODATTR_NONE,
    cntct->get_modified_attributes(),
    MODATTR_NONE,
    cntct->get_modified_host_attributes(),
    attr,
    cntct->get_modified_service_attributes(),
    nullptr);

  /* update the status log to reflect the new contact state */
  cntct->update_status_info(false);
}

/* disables service notifications for a contact */
void disable_contact_service_notifications(contact* cntct) {
  unsigned long attr(MODATTR_NOTIFICATIONS_ENABLED);

  /* no change */
  if (!cntct->get_service_notifications_enabled())
    return;

  /* set the attribute modified flag */
  cntct->set_modified_service_attributes(
      cntct->get_modified_service_attributes()
      | attr);

  /* enable the host notifications... */
  cntct->set_service_notifications_enabled(false);

  /* send data to event broker */
  broker_adaptive_contact_data(
    NEBTYPE_ADAPTIVECONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    cntct,
    CMD_NONE,
    MODATTR_NONE,
    cntct->get_modified_attributes(),
    MODATTR_NONE,
    cntct->get_modified_host_attributes(),
    attr,
    cntct->get_modified_service_attributes(),
    nullptr);

  /* update the status log to reflect the new contact state */
  cntct->update_status_info(false);
}

/* schedules downtime for all hosts "beyond" a given host */
void schedule_and_propagate_downtime(
       com::centreon::engine::host* temp_host,
       time_t entry_time,
       char const* author,
       char const* comment_data,
       time_t start_time,
       time_t end_time,
       int fixed,
       unsigned long triggered_by,
       unsigned long duration) {
  com::centreon::engine::host* child_host(nullptr);

  /* check all child hosts... */
  for (host_map::iterator
         it(temp_host->child_hosts.begin()),
         end(temp_host->child_hosts.end());
       it != end;
       ++it) {

    if (it->second == nullptr)
      continue;

    /* recurse... */
    schedule_and_propagate_downtime(
      it->second.get(),
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      triggered_by,
      duration);

    /* schedule downtime for this host */
    downtime_manager::instance().schedule_downtime(
      HOST_DOWNTIME,
      it->first,
      nullptr,
      entry_time,
      author,
      comment_data,
      start_time,
      end_time,
      fixed,
      triggered_by,
      duration,
      nullptr);
  }
}

/* acknowledges a host problem */
void acknowledge_host_problem(
       com::centreon::engine::host* hst,
       char* ack_author,
       char* ack_data,
       int type,
       int notify,
       int persistent) {
  /* cannot acknowledge a non-existent problem */
  if (hst->get_current_state() == HOST_UP)
    return;

  /* set the acknowledgement flag */
  hst->set_problem_has_been_acknowledged(true);

  /* set the acknowledgement type */
  hst->set_acknowledgement_type((type == ACKNOWLEDGEMENT_STICKY)
    ? ACKNOWLEDGEMENT_STICKY : ACKNOWLEDGEMENT_NORMAL);

  /* schedule acknowledgement expiration */
  time_t current_time(time(nullptr));
  host_other_props[hst->get_name()].last_acknowledgement = current_time;
  schedule_acknowledgement_expiration(hst);

  /* send data to event broker */
  broker_acknowledgement_data(
    NEBTYPE_ACKNOWLEDGEMENT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_ACKNOWLEDGEMENT,
    (void*)hst,
    ack_author,
    ack_data,
    type,
    notify,
    persistent,
    nullptr);

  /* send out an acknowledgement notification */
  if (notify)
    host_notification(
      hst,
      NOTIFICATION_ACKNOWLEDGEMENT,
      ack_author,
      ack_data,
      NOTIFICATION_OPTION_NONE);

  /* update the status log with the host info */
  update_host_status(hst, false);

  /* add a comment for the acknowledgement */
  std::shared_ptr<comment> com =
    std::make_shared<comment>(
      comment::host,
      comment::acknowledgment,
      hst->get_name().c_str(),
      "",
      current_time,
      ack_author,
      ack_data,
      persistent,
      comment::internal,
      false,
      (time_t)0);
  comment::comments.insert({com->get_comment_id(), com});
}

/* acknowledges a service problem */
void acknowledge_service_problem(
       com::centreon::engine::service* svc,
       char* ack_author,
       char* ack_data,
       int type,
       int notify,
       int persistent) {
  /* cannot acknowledge a non-existent problem */
  if (svc->current_state == STATE_OK)
    return;

  /* set the acknowledgement flag */
  svc->problem_has_been_acknowledged = true;

  /* set the acknowledgement type */
  svc->acknowledgement_type = (type == ACKNOWLEDGEMENT_STICKY)
    ? ACKNOWLEDGEMENT_STICKY : ACKNOWLEDGEMENT_NORMAL;

  /* schedule acknowledgement expiration */
  time_t current_time(time(nullptr));
  service_other_props[{svc->host_ptr->get_name(), svc->get_description()}]
      .last_acknowledgement = current_time;
  schedule_acknowledgement_expiration(svc);

  /* send data to event broker */
  broker_acknowledgement_data(
    NEBTYPE_ACKNOWLEDGEMENT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_ACKNOWLEDGEMENT,
    (void*)svc,
    ack_author,
    ack_data,
    type,
    notify,
    persistent,
    nullptr);

  /* send out an acknowledgement notification */
  if (notify)
    service_notification(
      svc,
      NOTIFICATION_ACKNOWLEDGEMENT,
      ack_author,
      ack_data,
      NOTIFICATION_OPTION_NONE);

  /* update the status log with the service info */
  update_service_status(svc, false);

  /* add a comment for the acknowledgement */
  std::shared_ptr<comment> com =
    std::make_shared<comment>(
      comment::service,
      comment::acknowledgment,
      svc->get_hostname(),
      svc->get_description(),
      current_time,
      ack_author,
      ack_data,
      persistent,
      comment::internal,
      false,
      (time_t)0);
  comment::comments.insert({com->get_comment_id(), com});
}

/* removes a host acknowledgement */
void remove_host_acknowledgement(com::centreon::engine::host* hst) {
  /* set the acknowledgement flag */
  hst->set_problem_has_been_acknowledged(false);

  /* update the status log with the host info */
  update_host_status(hst, false);

  /* remove any non-persistant comments associated with the ack */
  comment::delete_host_acknowledgement_comments(hst);
}

/* removes a service acknowledgement */
void remove_service_acknowledgement(com::centreon::engine::service* svc) {
  /* set the acknowledgement flag */
  svc->problem_has_been_acknowledged = false;

  /* update the status log with the service info */
  update_service_status(svc, false);

  /* remove any non-persistant comments associated with the ack */
  comment::delete_service_acknowledgement_comments(svc);
}

/* starts executing service checks */
void start_executing_service_checks(void) {
  unsigned long attr(MODATTR_ACTIVE_CHECKS_ENABLED);

  /* bail out if we're already executing services */
  if (config->execute_service_checks())
    return;

  /* set the attribute modified flag */
  modified_service_process_attributes |= attr;

  /* set the service check execution flag */
  config->execute_service_checks(true);

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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
}

/* stops executing service checks */
void stop_executing_service_checks(void) {
  unsigned long attr = MODATTR_ACTIVE_CHECKS_ENABLED;

  /* bail out if we're already not executing services */
  if (config->execute_service_checks() == false)
    return;

  /* set the attribute modified flag */
  modified_service_process_attributes |= attr;

  /* set the service check execution flag */
  config->execute_service_checks(false);

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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
}

/* starts accepting passive service checks */
void start_accepting_passive_service_checks(void) {
  unsigned long attr(MODATTR_PASSIVE_CHECKS_ENABLED);

  /* bail out if we're already accepting passive services */
  if (config->accept_passive_service_checks())
    return;

  /* set the attribute modified flag */
  modified_service_process_attributes |= attr;

  /* set the service check flag */
  config->accept_passive_service_checks(true);

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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
}

/* stops accepting passive service checks */
void stop_accepting_passive_service_checks(void) {
  unsigned long attr(MODATTR_PASSIVE_CHECKS_ENABLED);

  /* bail out if we're already not accepting passive services */
  if (config->accept_passive_service_checks() == false)
    return;

  /* set the attribute modified flag */
  modified_service_process_attributes |= attr;

  /* set the service check flag */
  config->accept_passive_service_checks(false);

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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
}

/* enables passive service checks for a particular service */
void enable_passive_service_checks(com::centreon::engine::service* svc) {
  unsigned long attr(MODATTR_PASSIVE_CHECKS_ENABLED);

  /* no change */
  if (svc->accept_passive_service_checks)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* set the passive check flag */
  svc->accept_passive_service_checks = true;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    nullptr);

  /* update the status log with the service info */
  update_service_status(svc, false);
}

/* disables passive service checks for a particular service */
void disable_passive_service_checks(com::centreon::engine::service* svc) {
  unsigned long attr(MODATTR_PASSIVE_CHECKS_ENABLED);

  /* no change */
  if (svc->accept_passive_service_checks == false)
    return;

  /* set the attribute modified flag */
  svc->modified_attributes |= attr;

  /* set the passive check flag */
  svc->accept_passive_service_checks = false;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    CMD_NONE,
    attr,
    svc->modified_attributes,
    nullptr);

  /* update the status log with the service info */
  update_service_status(svc, false);
}

/* starts executing host checks */
void start_executing_host_checks(void) {
  unsigned long attr(MODATTR_ACTIVE_CHECKS_ENABLED);

  /* bail out if we're already executing hosts */
  if (config->execute_host_checks())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;

  /* set the host check execution flag */
  config->execute_host_checks(true);

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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
}

/* stops executing host checks */
void stop_executing_host_checks(void) {
  unsigned long attr(MODATTR_ACTIVE_CHECKS_ENABLED);

  /* bail out if we're already not executing hosts */
  if (config->execute_host_checks() == false)
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;

  /* set the host check execution flag */
  config->execute_host_checks(false);

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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
}

/* starts accepting passive host checks */
void start_accepting_passive_host_checks(void) {
  unsigned long attr(MODATTR_PASSIVE_CHECKS_ENABLED);

  /* bail out if we're already accepting passive hosts */
  if (config->accept_passive_host_checks())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;

  /* set the host check flag */
  config->accept_passive_host_checks(true);

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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
}

/* stops accepting passive host checks */
void stop_accepting_passive_host_checks(void) {
  unsigned long attr(MODATTR_PASSIVE_CHECKS_ENABLED);

  /* bail out if we're already not accepting passive hosts */
  if (config->accept_passive_host_checks() == false)
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;

  /* set the host check flag */
  config->accept_passive_host_checks(false);

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
    nullptr);
  /* update the status log with the program info */
  update_program_status(false);
}

/* enables passive host checks for a particular host */
void enable_passive_host_checks(com::centreon::engine::host* hst) {
  unsigned long attr(MODATTR_PASSIVE_CHECKS_ENABLED);

  /* no change */
  if (hst->get_accept_passive_host_checks())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* set the passive check flag */
  hst->set_accept_passive_host_checks(true);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log with the host info */
  update_host_status(hst, false);
}

/* disables passive host checks for a particular host */
void disable_passive_host_checks(com::centreon::engine::host* hst) {
  unsigned long attr(MODATTR_PASSIVE_CHECKS_ENABLED);

  /* no change */
  if (!hst->get_accept_passive_host_checks())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* set the passive check flag */
  hst->set_accept_passive_host_checks(false);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log with the host info */
  update_host_status(hst, false);
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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
}

/* enables the event handler for a particular service */
void enable_service_event_handler(com::centreon::engine::service* svc) {
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
    nullptr);

  /* update the status log with the service info */
  update_service_status(svc, false);
}

/* disables the event handler for a particular service */
void disable_service_event_handler(com::centreon::engine::service* svc) {
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
    nullptr);

  /* update the status log with the service info */
  update_service_status(svc, false);
}

/* enables the event handler for a particular host */
void enable_host_event_handler(com::centreon::engine::host* hst) {
  unsigned long attr(MODATTR_EVENT_HANDLER_ENABLED);

  /* no change */
  if (hst->get_event_handler_enabled())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* set the event handler flag */
  hst->set_event_handler_enabled(true);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log with the host info */
  update_host_status(hst, false);
}

/* disables the event handler for a particular host */
void disable_host_event_handler(com::centreon::engine::host* hst) {
  unsigned long attr(MODATTR_EVENT_HANDLER_ENABLED);

  /* no change */
  if (!hst->get_event_handler_enabled())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* set the event handler flag */
  hst->set_event_handler_enabled(false);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log with the host info */
  update_host_status(hst, false);
}

/* disables checks of a particular host */
void disable_host_checks(com::centreon::engine::host* hst) {
  unsigned long attr(MODATTR_ACTIVE_CHECKS_ENABLED);

  /* checks are already disabled */
  if (!hst->get_checks_enabled())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* set the host check flag */
  hst->set_checks_enabled(false);
  hst->set_should_be_scheduled(false);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log with the host info */
  update_host_status(hst, false);
}

/* enables checks of a particular host */
void enable_host_checks(com::centreon::engine::host* hst) {
  time_t preferred_time(0);
  time_t next_valid_time(0);
  unsigned long attr(MODATTR_ACTIVE_CHECKS_ENABLED);

  /* checks are already enabled */
  if (hst->get_checks_enabled())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* set the host check flag */
  hst->set_checks_enabled(true);
  hst->set_should_be_scheduled(true);

  /* hosts with no check intervals don't get checked */
  if (hst->get_check_interval() == 0)
    hst->set_should_be_scheduled(false);

  /* schedule a check for right now (or as soon as possible) */
  time(&preferred_time);
  if (check_time_against_period(preferred_time, hst->check_period_ptr) == ERROR) {
    get_next_valid_time(preferred_time, &next_valid_time, hst->check_period_ptr);
    hst->set_next_check(next_valid_time);
  }
  else
    hst->set_next_check(preferred_time);

  /* schedule a check if we should */
  if (hst->get_should_be_scheduled())
    schedule_host_check(hst, hst->get_next_check(), CHECK_OPTION_NONE);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log with the host info */
  update_host_status(hst, false);
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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
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
    nullptr);
  /* update the status log with the program info */
  update_program_status(false);
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
    nullptr);

  /* update the status log with the program info */
  update_program_status(false);
}

/* enable performance data on a program-wide basis */
void enable_performance_data(void) {
  unsigned long attr(MODATTR_PERFORMANCE_DATA_ENABLED);

  /* bail out if we're already set... */
  if (config->process_performance_data())
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  config->process_performance_data(true);

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
    nullptr);

  /* update the status log */
  update_program_status(false);
}

/* disable performance data on a program-wide basis */
void disable_performance_data(void) {
  unsigned long attr(MODATTR_PERFORMANCE_DATA_ENABLED);

  /* bail out if we're already set... */
  if (config->process_performance_data() == false)
    return;

  /* set the attribute modified flag */
  modified_host_process_attributes |= attr;
  modified_service_process_attributes |= attr;

  config->process_performance_data(false);

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
    nullptr);

  /* update the status log */
  update_program_status(false);
}

/* start obsessing over a particular service */
void start_obsessing_over_service(com::centreon::engine::service* svc) {
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
    nullptr);

  /* update the status log with the service info */
  update_service_status(svc, false);
}

/* stop obsessing over a particular service */
void stop_obsessing_over_service(com::centreon::engine::service* svc) {
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
    nullptr);

  /* update the status log with the service info */
  update_service_status(svc, false);
}

/* start obsessing over a particular host */
void start_obsessing_over_host(com::centreon::engine::host* hst) {
  unsigned long attr(MODATTR_OBSESSIVE_HANDLER_ENABLED);

  /* no change */
  if (hst->get_obsess_over_host())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* set the obsess over host flag */
  hst->set_obsess_over_host(true);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log with the host info */
  update_host_status(hst, false);
}

/* stop obsessing over a particular host */
void stop_obsessing_over_host(com::centreon::engine::host* hst) {
  unsigned long attr(MODATTR_OBSESSIVE_HANDLER_ENABLED);

  /* no change */
  if (!hst->get_obsess_over_host())
    return;

  /* set the attribute modified flag */
  hst->set_modified_attributes(
    hst->get_modified_attributes() | attr);

  /* set the obsess over host flag */
  hst->set_obsess_over_host(false);

  /* send data to event broker */
  broker_adaptive_host_data(
    NEBTYPE_ADAPTIVEHOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    attr,
    hst->get_modified_attributes(),
    nullptr);

  /* update the status log with the host info */
  update_host_status(hst, false);
}

/* sets the current notification number for a specific host */
void set_host_notification_number(com::centreon::engine::host* hst, int num) {
  /* set the notification number */
  hst->set_current_notification_number(num);

  /* update the status log with the host info */
  update_host_status(hst, false);
}

/* sets the current notification number for a specific service */
void set_service_notification_number(com::centreon::engine::service* svc, int num) {
  /* set the notification number */
  svc->current_notification_number = num;

  /* update the status log with the service info */
  update_service_status(svc, false);
}
