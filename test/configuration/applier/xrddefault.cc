/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2009      Nagios Core Development Team and Community Contributors
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
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/utils.hh"
#include "com/centreon/io/file_entry.hh"
#include "mmap.h"
#include "xrddefault.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

static char* xrddefault_retention_file = NULL;
static int   xrddefault_retention_file_fd = -1;

/******************************************************************/
/********************* CONFIG INITIALIZATION  *********************/
/******************************************************************/

int xrddefault_grab_config_info(char const* main_config_file) {
  char* input = NULL;
  mmapfile* thefile = NULL;
  nagios_macros* mac;

  mac = get_global_macros();

  /* open the main config file for reading */
  if ((thefile = mmap_fopen(main_config_file)) == NULL) {
    logger(dbg_retentiondata, most)
      << "Error: Cannot open main configuration file '"
      << main_config_file << "' for reading!";

    delete[] xrddefault_retention_file;
    xrddefault_retention_file = NULL;

    return (ERROR);
  }

  /* read in all lines from the main config file */
  while (1) {
    /* free memory */
    delete[] input;

    /* read the next line */
    if ((input = mmap_fgets_multiline(thefile)) == NULL)
      break;

    strip(input);

    /* skip blank lines and comments */
    if (input[0] == '#' || input[0] == '\x0')
      continue;

    xrddefault_grab_config_directives(main_config_file, input);
  }

  /* free memory and close the file */
  delete[] input;
  mmap_fclose(thefile);

  /* initialize locations if necessary  */
  if (xrddefault_retention_file == NULL)
    xrddefault_retention_file = string::dup(DEFAULT_RETENTION_FILE);

  /* save the retention file macro */
  delete[] mac->x[MACRO_RETENTIONDATAFILE];
  mac->x[MACRO_RETENTIONDATAFILE] = string::dup(xrddefault_retention_file);
  strip(mac->x[MACRO_RETENTIONDATAFILE]);

  return (OK);
}

/* process a single config directive */
int xrddefault_grab_config_directives(
      char const* filename,
      char* input) {
  char const* temp_ptr = NULL;
  char* varname = NULL;
  char* varvalue = NULL;

  /* get the variable name */
  if ((temp_ptr = my_strtok(input, "=")) == NULL)
    return (ERROR);
  varname = string::dup(temp_ptr);

  /* get the variable value */
  if ((temp_ptr = my_strtok(NULL, "\n")) == NULL) {
    delete[] varname;
    return (ERROR);
  }
  varvalue = string::dup(temp_ptr);

  /* retention file definition */
  if (!strcmp(varname, "xrddefault_retention_file")
      || !strcmp(varname, "state_retention_file")) {
    if (varvalue[0] == '/')
      xrddefault_retention_file = string::dup(varvalue);
    else {
      io::file_entry fe(filename);
      std::string base_name(fe.directory_name());
      xrddefault_retention_file = string::dup(base_name + "/" + varvalue);
    }
  }
  /* free memory */
  delete[] varname;
  delete[] varvalue;

  return (OK);
}

/******************************************************************/
/********************* INIT/CLEANUP FUNCTIONS *********************/
/******************************************************************/

/* initialize retention data */
int xrddefault_initialize_retention_data(char const* config_file) {
  /* grab configuration data */
  if (xrddefault_grab_config_info(config_file) == ERROR)
    return (ERROR);

  if (xrddefault_retention_file_fd == -1) {
    if ((xrddefault_retention_file_fd = open(
                                          xrddefault_retention_file,
                                          O_WRONLY | O_CREAT,
                                          S_IRUSR | S_IWUSR)) == -1) {
      logger(log_runtime_error, basic)
        << "Error: Unable to open retention file '"
        << xrddefault_retention_file << "': " << strerror(errno);
      return (ERROR);
    }
    set_cloexec(xrddefault_retention_file_fd);
  }

  return (OK);
}

/* cleanup retention data before terminating */
int xrddefault_cleanup_retention_data(char const* config_file) {
  (void)config_file;

  /* free memory */
  delete[] xrddefault_retention_file;
  xrddefault_retention_file = NULL;

  if (xrddefault_retention_file_fd != -1) {
    close(xrddefault_retention_file_fd);
    xrddefault_retention_file_fd = -1;
  }

  return (OK);
}

/******************************************************************/
/**************** DEFAULT STATE OUTPUT FUNCTION *******************/
/******************************************************************/

int xrddefault_save_state_information() {
  logger(dbg_functions, basic)
    << "xrddefault_save_state_information()";

  /* make sure we have everything */
  if (xrddefault_retention_file == NULL
      || xrddefault_retention_file_fd == -1) {
    logger(log_runtime_error, basic)
      << "Error: We don't have the required file names to store "
      "retention data!";
    return (ERROR);
  }

  std::ostringstream stream;
  std::streamsize ss(stream.precision());

  /* write version info to status file */
  stream << "##############################################\n"
         << "#    CENTREON ENGINE STATE RETENTION FILE    #\n"
         << "#                                            #\n"
         << "# THIS FILE IS AUTOMATICALLY GENERATED BY    #\n"
         << "# CENTREON ENGINE. DO NOT MODIFY THIS FILE ! #\n"
         << "##############################################\n";

  time_t current_time;
  time(&current_time);

  /* write file info */
  stream << "info {\n"
         << "created=" << static_cast<unsigned long>(current_time) << "\n"
         << "}\n";

  /* save program state information */
  stream << "program {\n"
         << "modified_host_attributes=" << modified_host_process_attributes << "\n"
         << "modified_service_attributes=" << modified_service_process_attributes << "\n"
         << "enable_event_handlers=" << enable_event_handlers << "\n"
         << "obsess_over_services=" << obsess_over_services << "\n"
         << "obsess_over_hosts=" << obsess_over_hosts << "\n"
         << "check_service_freshness=" << check_service_freshness << "\n"
         << "check_host_freshness=" << check_host_freshness << "\n"
         << "enable_flap_detection=" << enable_flap_detection << "\n"
         << "global_host_event_handler=" << global_host_event_handler << "\n"
         << "global_service_event_handler=" << global_service_event_handler << "\n"
         << "next_event_id=" << next_event_id << "\n"
         << "next_problem_id=" << next_problem_id << "\n"
         << "}\n";

  /* save host state information */
  for (host* temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {

    stream << "host {\n"
           << "host_name=" << temp_host->name << "\n"
           << "modified_attributes=" << temp_host->modified_attributes << "\n"
           << "check_command=" << (temp_host->host_check_command ? temp_host->host_check_command : "") << "\n"
           << "check_period=" << (temp_host->check_period ? temp_host->check_period : "") << "\n"
           << "event_handler=" << (temp_host->event_handler ? temp_host->event_handler : "") << "\n"
           << "has_been_checked=" << temp_host->has_been_checked << "\n"
           << "check_execution_time=" << std::setprecision(3) << std::fixed<< temp_host->execution_time << std::setprecision(ss) << "\n"
           << "check_latency=" << std::setprecision(3) << std::fixed << temp_host->latency << std::setprecision(ss) << "\n"
           << "check_type=" << temp_host->check_type << "\n"
           << "current_state=" << temp_host->current_state << "\n"
           << "last_state=" << temp_host->last_state << "\n"
           << "last_hard_state=" << temp_host->last_hard_state << "\n"
           << "last_event_id=" << temp_host->last_event_id << "\n"
           << "current_event_id=" << temp_host->current_event_id << "\n"
           << "current_problem_id=" << temp_host->current_problem_id << "\n"
           << "last_problem_id=" << temp_host->last_problem_id << "\n"
           << "plugin_output=" << (temp_host->plugin_output ? temp_host->plugin_output : "") << "\n"
           << "long_plugin_output=" << (temp_host->long_plugin_output ? temp_host->long_plugin_output : "") << "\n"
           << "performance_data=" << (temp_host->perf_data ? temp_host->perf_data : "") << "\n"
           << "last_check=" << static_cast<unsigned long>(temp_host->last_check) << "\n"
           << "next_check=" << static_cast<unsigned long>(temp_host->next_check) << "\n"
           << "check_options=" << temp_host->check_options << "\n"
           << "current_attempt=" << temp_host->current_attempt << "\n"
           << "max_attempts=" << temp_host->max_attempts << "\n"
           << "normal_check_interval=" << temp_host->check_interval << "\n"
           << "retry_check_interval=" << temp_host->check_interval << "\n"
           << "state_type=" << temp_host->state_type << "\n"
           << "last_state_change=" << static_cast<unsigned long>(temp_host->last_state_change) << "\n"
           << "last_hard_state_change=" << static_cast<unsigned long>(temp_host->last_hard_state_change) << "\n"
           << "last_time_up=" << static_cast<unsigned long>(temp_host->last_time_up) << "\n"
           << "last_time_down=" << static_cast<unsigned long>(temp_host->last_time_down) << "\n"
           << "last_time_unreachable=" << static_cast<unsigned long>(temp_host->last_time_unreachable) << "\n"
           << "active_checks_enabled=" << temp_host->checks_enabled << "\n"
           << "event_handler_enabled=" << temp_host->event_handler_enabled << "\n"
           << "flap_detection_enabled=" << temp_host->flap_detection_enabled << "\n"
           << "obsess_over_host=" << temp_host->obsess_over_host << "\n"
           << "is_flapping=" << temp_host->is_flapping << "\n"
           << "percent_state_change=" << std::setprecision(2) << std::fixed << temp_host->percent_state_change << std::setprecision(ss) << "\n";
    stream << "state_history=";
    for (unsigned int x = 0; x < MAX_STATE_HISTORY_ENTRIES; x++)
      stream << (x > 0 ? "," : "") << temp_host->state_history[(x + temp_host->state_history_index) % MAX_STATE_HISTORY_ENTRIES];
    stream << "\n";

    /* custom variables */
    for (customvariablesmember* temp_customvariablesmember = temp_host->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      if (temp_customvariablesmember->variable_name)
        stream << "_" << temp_customvariablesmember->variable_name << "="
               << temp_customvariablesmember->has_been_modified << ";"
               << (temp_customvariablesmember->variable_value
                   ? temp_customvariablesmember->variable_value : "") << "\n";
    }

    stream << "}\n";
  }

  /* save service state information */
  for (service* temp_service = service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    stream << "service {\n"
           << "host_name=" << temp_service->host_name << "\n"
           << "service_description=" << temp_service->description << "\n"
           << "modified_attributes=" << temp_service->modified_attributes << "\n"
           << "check_command=" << (temp_service->service_check_command ? temp_service->service_check_command : "") << "\n"
           << "check_period=" << (temp_service->check_period ? temp_service->check_period : "") << "\n"
           << "event_handler=" << (temp_service->event_handler ? temp_service->event_handler : "") << "\n"
           << "has_been_checked=" << temp_service->has_been_checked << "\n"
           << "check_execution_time=" << std::setprecision(3) << std::fixed << temp_service->execution_time << std::setprecision(ss) << "\n"
           << "check_latency=" << std::setprecision(3) << std::fixed << temp_service->latency << std::setprecision(ss) << "\n"
           << "check_type=" << temp_service->check_type << "\n"
           << "current_state=" << temp_service->current_state << "\n"
           << "last_state=" << temp_service->last_state << "\n"
           << "last_hard_state=" << temp_service->last_hard_state << "\n"
           << "last_event_id=" << temp_service->last_event_id << "\n"
           << "current_event_id=" << temp_service->current_event_id << "\n"
           << "current_problem_id=" << temp_service->current_problem_id << "\n"
           << "last_problem_id=" << temp_service->last_problem_id << "\n"
           << "current_attempt=" << temp_service->current_attempt << "\n"
           << "max_attempts=" << temp_service->max_attempts << "\n"
           << "normal_check_interval=" << temp_service->check_interval << "\n"
           << "retry_check_interval=" << temp_service->retry_interval << "\n"
           << "state_type=" << temp_service->state_type << "\n"
           << "last_state_change=" << static_cast<unsigned long>(temp_service->last_state_change) << "\n"
           << "last_hard_state_change=" << static_cast<unsigned long>(temp_service->last_hard_state_change) << "\n"
           << "last_time_ok=" << static_cast<unsigned long>(temp_service->last_time_ok) << "\n"
           << "last_time_warning=" << static_cast<unsigned long>(temp_service->last_time_warning) << "\n"
           << "last_time_unknown=" << static_cast<unsigned long>(temp_service->last_time_unknown) << "\n"
           << "last_time_critical=" << static_cast<unsigned long>(temp_service->last_time_critical) << "\n"
           << "plugin_output=" << (temp_service->plugin_output ? temp_service->plugin_output : "") << "\n"
           << "long_plugin_output=" << (temp_service->long_plugin_output ? temp_service->long_plugin_output : "") << "\n"
           << "performance_data=" << (temp_service->perf_data ? temp_service->perf_data : "") << "\n"
           << "last_check=" << static_cast<unsigned long>(temp_service->last_check) << "\n"
           << "next_check=" << static_cast<unsigned long>(temp_service->next_check) << "\n"
           << "check_options=" << temp_service->check_options << "\n"
           << "active_checks_enabled=" << temp_service->checks_enabled << "\n"
           << "event_handler_enabled=" << temp_service->event_handler_enabled << "\n"
           << "flap_detection_enabled=" << temp_service->flap_detection_enabled << "\n"
           << "obsess_over_service=" << temp_service->obsess_over_service << "\n"
           << "is_flapping=" << temp_service->is_flapping << "\n"
           << "percent_state_change=" << std::setprecision(2) << std::fixed << temp_service->percent_state_change << std::setprecision(ss) << "\n";

    stream << "state_history=";
    for (unsigned int x = 0; x < MAX_STATE_HISTORY_ENTRIES; x++)
      stream << (x > 0 ? "," : "") << temp_service->state_history[(x + temp_service->state_history_index) % MAX_STATE_HISTORY_ENTRIES];
    stream << "\n";

    /* custom variables */
    for (customvariablesmember* temp_customvariablesmember = temp_service->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      if (temp_customvariablesmember->variable_name)
        stream << "_" << temp_customvariablesmember->variable_name << "="
               << temp_customvariablesmember->has_been_modified << ";"
               << (temp_customvariablesmember->variable_value
                   ? temp_customvariablesmember->variable_value : "") << "\n";
    }
    stream << "}\n";
  }

  // Write data in buffer.
  stream.flush();

  // Prepare retention file for overwrite.
  if ((ftruncate(xrddefault_retention_file_fd, 0) == -1)
      || (fsync(xrddefault_retention_file_fd) == -1)
      || (lseek(xrddefault_retention_file_fd, 0, SEEK_SET)
          == (off_t)-1)) {
    char const* msg(strerror(errno));
    logger(log_runtime_error, basic)
      << "Error: Unable to update retention file '"
      << xrddefault_retention_file << "': " << msg;
    return (ERROR);
  }

  // Write retention file.
  std::string data(stream.str());
  char const* data_ptr(data.c_str());
  unsigned int size(data.size());
  while (size > 0) {
    ssize_t wb(write(xrddefault_retention_file_fd, data_ptr, size));
    if (wb <= 0) {
      char const* msg(strerror(errno));
      logger(log_runtime_error, basic)
        << "Error: Unable to update retention file '"
        << xrddefault_retention_file << "': " << msg;
      return (ERROR);
    }
    data_ptr += wb;
    size -= wb;
  }

  return (OK);
}

/******************************************************************/
/***************** DEFAULT STATE INPUT FUNCTION *******************/
/******************************************************************/

int xrddefault_read_state_information() {
  char* input = NULL;
  char* inputbuf = NULL;
  char* temp_ptr = NULL;
  mmapfile* thefile;
  char* host_name = NULL;
  char* service_description = NULL;
  char* author = NULL;
  char* comment_data = NULL;
  unsigned int data_type = XRDDEFAULT_NO_DATA;
  unsigned int x = 0;
  host* temp_host = NULL;
  service* temp_service = NULL;
  command* temp_command = NULL;
  timeperiod* temp_timeperiod = NULL;
  customvariablesmember* temp_customvariablesmember = NULL;
  char* customvarname = NULL;
  char* var = NULL;
  char* val = NULL;
  char* tempval = NULL;
  char* ch = NULL;
  int persistent = FALSE;
  int expires = FALSE;
  time_t expire_time = 0L;
  time_t entry_time = 0L;
  time_t creation_time;
  time_t current_time;
  int scheduling_info_is_ok = FALSE;
  time_t start_time = 0L;
  time_t end_time = 0L;
  int fixed = FALSE;
  unsigned long triggered_by = 0;
  unsigned long duration = 0L;
  int was_flapping = FALSE;
  struct timeval tv[2];
  double runtime[2];
  int found_directive = FALSE;

  logger(dbg_functions, basic)
    << "xrddefault_read_state_information()";

  /* make sure we have what we need */
  if (xrddefault_retention_file == NULL) {
    logger(log_runtime_error, basic)
      << "Error: We don't have a filename for retention data!";
    return (ERROR);
  }

  if (test_scheduling == TRUE)
    gettimeofday(&tv[0], NULL);

  /* open the retention file for reading */
  if ((thefile = mmap_fopen(xrddefault_retention_file)) == NULL)
    return (ERROR);

  /* read all lines in the retention file */
  while (1) {
    /* free memory */
    delete[] inputbuf;

    /* read the next line */
    if ((inputbuf = mmap_fgets(thefile)) == NULL)
      break;

    input = inputbuf;
    /* far better than strip()ing */
    if (input[0] == '\t')
      input++;

    strip(input);

    if (!strcmp(input, "service {"))
      data_type = XRDDEFAULT_SERVICESTATUS_DATA;
    else if (!strcmp(input, "host {"))
      data_type = XRDDEFAULT_HOSTSTATUS_DATA;
    else if (!strcmp(input, "info {"))
      data_type = XRDDEFAULT_INFO_DATA;
    else if (!strcmp(input, "program {"))
      data_type = XRDDEFAULT_PROGRAMSTATUS_DATA;
    else if (!strcmp(input, "}")) {
      switch (data_type) {
      case XRDDEFAULT_INFO_DATA:
        break;

      case XRDDEFAULT_HOSTSTATUS_DATA:
        if (temp_host != NULL) {
          /* adjust modified attributes if no custom variables have been changed */
          if (temp_host->modified_attributes & MODATTR_CUSTOM_VARIABLE) {
            for (temp_customvariablesmember = temp_host->custom_variables;
                 temp_customvariablesmember != NULL;
                 temp_customvariablesmember = temp_customvariablesmember->next) {
              if (temp_customvariablesmember->has_been_modified == TRUE)
                break;
            }
            if (temp_customvariablesmember == NULL)
              temp_host->modified_attributes -= MODATTR_CUSTOM_VARIABLE;
          }

          /* ADDED 01/23/2009 adjust current check attempts if host in hard problem state (max attempts may have changed in config since restart) */
          if (temp_host->current_state != HOST_UP
              && temp_host->state_type == HARD_STATE)
            temp_host->current_attempt = temp_host->max_attempts;

          // Assume same flapping state.
          temp_host->is_flapping = was_flapping;

          /* handle new vars added in 2.x */
          if (temp_host->last_hard_state_change == (time_t)0)
            temp_host->last_hard_state_change = temp_host->last_state_change;

          /* update host status */
          // update_host_status(temp_host, FALSE);
        }

        /* reset vars */
        was_flapping = FALSE;

        delete[] host_name;
        host_name = NULL;
        temp_host = NULL;
        break;

      case XRDDEFAULT_SERVICESTATUS_DATA:
        if (temp_service != NULL) {
          /* adjust modified attributes if no custom variables have been changed */
          if (temp_service->modified_attributes & MODATTR_CUSTOM_VARIABLE) {
            for (temp_customvariablesmember = temp_service->custom_variables;
                 temp_customvariablesmember != NULL;
                 temp_customvariablesmember = temp_customvariablesmember->next) {
              if (temp_customvariablesmember->has_been_modified == TRUE)
                break;
            }
            if (temp_customvariablesmember == NULL)
              temp_service->modified_attributes -= MODATTR_CUSTOM_VARIABLE;
          }

          /* fix old vars */
          if (temp_service->has_been_checked == FALSE
              && temp_service->state_type == SOFT_STATE)
            temp_service->state_type = HARD_STATE;

          /* ADDED 01/23/2009 adjust current check attempt if service is in hard problem state (max attempts may have changed in config since restart) */
          if (temp_service->current_state != STATE_OK
              && temp_service->state_type == HARD_STATE)
            temp_service->current_attempt = temp_service->max_attempts;


          // Assume same flapping state if large install tweaks enabled.
          temp_service->is_flapping = was_flapping;

          /* handle new vars added in 2.x */
          if (temp_service->last_hard_state_change == (time_t)0)
            temp_service->last_hard_state_change = temp_service->last_state_change;

          /* update service status */
          // update_service_status(temp_service, FALSE);
        }

        /* reset vars */
        was_flapping = FALSE;

        delete[] host_name;
        delete[] service_description;
        host_name = NULL;
        service_description = NULL;
        temp_service = NULL;
        break;

      default:
        break;
      }

      data_type = XRDDEFAULT_NO_DATA;
    }
    else if (data_type != XRDDEFAULT_NO_DATA) {
      /* slightly faster than strtok () */
      var = input;
      if ((val = strchr(input, '=')) == NULL)
        continue;
      val[0] = '\x0';
      val++;
      found_directive = TRUE;

      switch (data_type) {
      case XRDDEFAULT_INFO_DATA:
        if (!strcmp(var, "created")) {
          creation_time = strtoul(val, NULL, 10);
          time(&current_time);
          scheduling_info_is_ok = TRUE;
        }
        // Ignore update-related fields.
        else if (!strcmp(var, "version")
                 || !strcmp(var, "update_available")
                 || !strcmp(var, "update_uid")
                 || !strcmp(var, "last_version")
                 || !strcmp(var, "new_version"))
          break;

      case XRDDEFAULT_PROGRAMSTATUS_DATA:
        if (!strcmp(var, "modified_host_attributes")) {
          modified_host_process_attributes = strtoul(val, NULL, 10);
        }
        else if (!strcmp(var, "modified_service_attributes")) {
          modified_service_process_attributes = strtoul(val, NULL, 10);
        }
        else if (!strcmp(var, "enable_event_handlers")) {
          if (modified_host_process_attributes & MODATTR_EVENT_HANDLER_ENABLED)
            config->enable_event_handlers((atoi(val) > 0) ? TRUE : FALSE);
        }
        else if (!strcmp(var, "obsess_over_services")) {
          if (modified_service_process_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED)
            config->obsess_over_services((atoi(val) > 0) ? TRUE : FALSE);
        }
        else if (!strcmp(var, "obsess_over_hosts")) {
          if (modified_host_process_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED)
            config->obsess_over_hosts((atoi(val) > 0) ? TRUE : FALSE);
        }
        else if (!strcmp(var, "check_service_freshness")) {
          if (modified_service_process_attributes & MODATTR_FRESHNESS_CHECKS_ENABLED)
            config->check_service_freshness((atoi(val) > 0) ? TRUE : FALSE);
        }
        else if (!strcmp(var, "check_host_freshness")) {
          if (modified_host_process_attributes & MODATTR_FRESHNESS_CHECKS_ENABLED)
            config->check_host_freshness((atoi(val) > 0) ? TRUE : FALSE);
        }
        else if (!strcmp(var, "enable_flap_detection")) {
          if (modified_host_process_attributes & MODATTR_FLAP_DETECTION_ENABLED)
            config->enable_flap_detection((atoi(val) > 0) ? TRUE : FALSE);
        }
        else if (!strcmp(var, "global_host_event_handler")) {
          if (modified_host_process_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
            /* make sure the check command still exists... */
            tempval = string::dup(val);
            temp_ptr = my_strtok(tempval, "!");
            temp_command = find_command(temp_ptr);
            temp_ptr = string::dup(val);
            delete[] tempval;

            if (temp_command != NULL && temp_ptr != NULL) {
              config->global_host_event_handler(temp_ptr);
            }
          }
        }
        else if (!strcmp(var, "global_service_event_handler")) {
          if (modified_service_process_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
            /* make sure the check command still exists... */
            tempval = string::dup(val);
            temp_ptr = my_strtok(tempval, "!");
            temp_command = find_command(temp_ptr);
            temp_ptr = string::dup(val);
            delete[] tempval;

            if (temp_command != NULL && temp_ptr != NULL) {
              config->global_service_event_handler(temp_ptr);
            }
          }
        }
        else if (!strcmp(var, "next_event_id"))
          next_event_id = strtoul(val, NULL, 10);
        else if (!strcmp(var, "next_problem_id"))
          next_problem_id = strtoul(val, NULL, 10);
        break;

      case XRDDEFAULT_HOSTSTATUS_DATA:
        if (temp_host == NULL) {
          if (!strcmp(var, "host_name")) {
            host_name = string::dup(val);
            temp_host = find_host(host_name);
          }
        }
        else {
          if (!strcmp(var, "modified_attributes")) {
            temp_host->modified_attributes = strtoul(val, NULL, 10);
            break;
          }
          else if (!strcmp(var, "has_been_checked"))
            temp_host->has_been_checked = (atoi(val) > 0) ? TRUE : FALSE;
          else if (!strcmp(var, "check_execution_time"))
            temp_host->execution_time = strtod(val, NULL);
          else if (!strcmp(var, "check_latency"))
            temp_host->latency = strtod(val, NULL);
          else if (!strcmp(var, "check_type"))
            temp_host->check_type = atoi(val);
          else if (!strcmp(var, "current_state"))
            temp_host->current_state = atoi(val);
          else if (!strcmp(var, "last_state"))
            temp_host->last_state = atoi(val);
          else if (!strcmp(var, "last_hard_state"))
            temp_host->last_hard_state = atoi(val);
          else if (!strcmp(var, "plugin_output")) {
            delete[] temp_host->plugin_output;
            temp_host->plugin_output = string::dup(val);
          }
          else if (!strcmp(var, "long_plugin_output")) {
            delete[] temp_host->long_plugin_output;
            temp_host->long_plugin_output = string::dup(val);
          }
          else if (!strcmp(var, "performance_data")) {
            delete[] temp_host->perf_data;
            temp_host->perf_data = string::dup(val);
          }
          else if (!strcmp(var, "last_check"))
            temp_host->last_check = strtoul(val, NULL, 10);
          else if (!strcmp(var, "next_check")) {
            if (scheduling_info_is_ok == TRUE)
              temp_host->next_check = strtoul(val, NULL, 10);
          }
          else if (!strcmp(var, "check_options")) {
            if (scheduling_info_is_ok == TRUE)
              temp_host->check_options = atoi(val);
          }
          else if (!strcmp(var, "current_attempt"))
            temp_host->current_attempt = atoi(val);
          else if (!strcmp(var, "current_event_id"))
            temp_host->current_event_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_event_id"))
            temp_host->last_event_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "current_problem_id"))
            temp_host->current_problem_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_problem_id"))
            temp_host->last_problem_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "state_type"))
            temp_host->state_type = atoi(val);
          else if (!strcmp(var, "last_state_change"))
            temp_host->last_state_change = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_hard_state_change"))
            temp_host->last_hard_state_change = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_time_up"))
            temp_host->last_time_up = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_time_down"))
            temp_host->last_time_down = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_time_unreachable"))
            temp_host->last_time_unreachable = strtoul(val, NULL, 10);
          else if (!strcmp(var, "is_flapping"))
            was_flapping = atoi(val);
          else if (!strcmp(var, "percent_state_change"))
            temp_host->percent_state_change = strtod(val, NULL);
          else
            if (!strcmp(var, "state_history")) {
              temp_ptr = val;
              for (x = 0; x < MAX_STATE_HISTORY_ENTRIES; x++) {
                if ((ch = my_strsep(&temp_ptr, ",")) != NULL)
                  temp_host->state_history[x] = atoi(ch);
                else
                  break;
              }
              temp_host->state_history_index = 0;
            }
            else
              found_directive = FALSE;

          /* null-op speeds up logic */
          if (found_directive == TRUE);

          else if (!strcmp(var, "active_checks_enabled")) {
            if (temp_host->modified_attributes & MODATTR_ACTIVE_CHECKS_ENABLED)
              temp_host->checks_enabled = (atoi(val) > 0) ? TRUE : FALSE;
          }
          else if (!strcmp(var, "event_handler_enabled")) {
            if (temp_host->modified_attributes & MODATTR_EVENT_HANDLER_ENABLED)
              temp_host->event_handler_enabled = (atoi(val) > 0) ? TRUE : FALSE;
          }
          else if (!strcmp(var, "flap_detection_enabled")) {
            if (temp_host->modified_attributes & MODATTR_FLAP_DETECTION_ENABLED)
              temp_host->flap_detection_enabled = (atoi(val) > 0) ? TRUE : FALSE;
          }
          else if (!strcmp(var, "obsess_over_host")) {
            if (temp_host->modified_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED)
              temp_host->obsess_over_host = (atoi(val) > 0) ? TRUE : FALSE;
          }
          else if (!strcmp(var, "check_command")) {
            if (temp_host->modified_attributes & MODATTR_CHECK_COMMAND) {
              /* make sure the check command still exists... */
              tempval = string::dup(val);
              temp_ptr = my_strtok(tempval, "!");
              temp_command = find_command(temp_ptr);
              temp_ptr = string::dup(val);
              delete[] tempval;

              if (temp_command != NULL && temp_ptr != NULL) {
                delete[] temp_host->host_check_command;
                temp_host->host_check_command = temp_ptr;
              }
              else
                temp_host->modified_attributes -= MODATTR_CHECK_COMMAND;
            }
          }
          else if (!strcmp(var, "check_period")) {
            if (temp_host->modified_attributes & MODATTR_CHECK_TIMEPERIOD) {
              /* make sure the timeperiod still exists... */
              temp_timeperiod = find_timeperiod(val);
              temp_ptr = string::dup(val);

              if (temp_timeperiod != NULL && temp_ptr != NULL) {
                delete[] temp_host->check_period;
                temp_host->check_period = temp_ptr;
              }
              else
                temp_host->modified_attributes -= MODATTR_CHECK_TIMEPERIOD;
            }
          }
          else if (!strcmp(var, "event_handler")) {
            if (temp_host->modified_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
              /* make sure the check command still exists... */
              tempval = string::dup(val);
              temp_ptr = my_strtok(tempval, "!");
              temp_command = find_command(temp_ptr);
              temp_ptr = string::dup(val);
              delete[] tempval;

              if (temp_command != NULL && temp_ptr != NULL) {
                delete[] temp_host->event_handler;
                temp_host->event_handler = temp_ptr;
              }
              else
                temp_host->modified_attributes -= MODATTR_EVENT_HANDLER_COMMAND;
            }
          }
          else if (!strcmp(var, "normal_check_interval")) {
            if (temp_host->modified_attributes & MODATTR_NORMAL_CHECK_INTERVAL
                && strtod(val, NULL) >= 0)
              temp_host->check_interval = strtod(val, NULL);
          }
          else if (!strcmp(var, "retry_check_interval")) {
            if (temp_host->modified_attributes & MODATTR_RETRY_CHECK_INTERVAL
                && strtod(val, NULL) >= 0)
              temp_host->retry_interval = strtod(val, NULL);
          }
          else if (!strcmp(var, "max_attempts")) {
            if (temp_host->modified_attributes & MODATTR_MAX_CHECK_ATTEMPTS
                && atoi(val) >= 1) {
              temp_host->max_attempts = atoi(val);

              /* adjust current attempt number if in a hard state */
              if (temp_host->state_type == HARD_STATE
                  && temp_host->current_state != HOST_UP
                  && temp_host->current_attempt > 1)
                temp_host->current_attempt = temp_host->max_attempts;
            }
          }
          /* custom variables */
          else if (var[0] == '_') {
            if (temp_host->modified_attributes & MODATTR_CUSTOM_VARIABLE) {

              /* get the variable name */
              customvarname = var + 1;

              for (temp_customvariablesmember = temp_host->custom_variables;
                   temp_customvariablesmember != NULL;
                   temp_customvariablesmember = temp_customvariablesmember->next) {
                if (!strcmp(customvarname, temp_customvariablesmember->variable_name)) {
                  if ((x = atoi(val)) > 0 && strlen(val) > 3) {
                    delete[] temp_customvariablesmember->variable_value;
                    temp_customvariablesmember->variable_value = string::dup(val + 2);
                    temp_customvariablesmember->has_been_modified = (x > 0) ? TRUE : FALSE;
                  }
                  break;
                }
              }
            }
          }
        }
        break;

      case XRDDEFAULT_SERVICESTATUS_DATA:
        if (temp_service == NULL) {
          if (!strcmp(var, "host_name")) {
            host_name = string::dup(val);

            /*temp_service=find_service(host_name,service_description); */

            /* break out */
            break;
          }
          else if (!strcmp(var, "service_description")) {
            service_description = string::dup(val);
            temp_service = find_service(host_name, service_description);

            /* break out */
            break;
          }
        }
        else {
          if (!strcmp(var, "modified_attributes")) {
            temp_service->modified_attributes = strtoul(val, NULL, 10);
          }
          else if (!strcmp(var, "has_been_checked"))
            temp_service->has_been_checked = (atoi(val) > 0) ? TRUE : FALSE;
          else if (!strcmp(var, "check_execution_time"))
            temp_service->execution_time = strtod(val, NULL);
          else if (!strcmp(var, "check_latency"))
            temp_service->latency = strtod(val, NULL);
          else if (!strcmp(var, "check_type"))
            temp_service->check_type = atoi(val);
          else if (!strcmp(var, "current_state"))
            temp_service->current_state = atoi(val);
          else if (!strcmp(var, "last_state"))
            temp_service->last_state = atoi(val);
          else if (!strcmp(var, "last_hard_state"))
            temp_service->last_hard_state = atoi(val);
          else if (!strcmp(var, "current_attempt"))
            temp_service->current_attempt = atoi(val);
          else if (!strcmp(var, "current_event_id"))
            temp_service->current_event_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_event_id"))
            temp_service->last_event_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "current_problem_id"))
            temp_service->current_problem_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_problem_id"))
            temp_service->last_problem_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "state_type"))
            temp_service->state_type = atoi(val);
          else if (!strcmp(var, "last_state_change"))
            temp_service->last_state_change = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_hard_state_change"))
            temp_service->last_hard_state_change = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_time_ok"))
            temp_service->last_time_ok = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_time_warning"))
            temp_service->last_time_warning = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_time_unknown"))
            temp_service->last_time_unknown = strtoul(val, NULL, 10);
          else if (!strcmp(var, "last_time_critical"))
            temp_service->last_time_critical = strtoul(val, NULL, 10);
          else if (!strcmp(var, "plugin_output")) {
            delete[] temp_service->plugin_output;
            temp_service->plugin_output = string::dup(val);
          }
          else if (!strcmp(var, "long_plugin_output")) {
            delete[] temp_service->long_plugin_output;
            temp_service->long_plugin_output = string::dup(val);
          }
          else if (!strcmp(var, "performance_data")) {
            delete[] temp_service->perf_data;
            temp_service->perf_data = string::dup(val);
          }
          else if (!strcmp(var, "last_check"))
            temp_service->last_check = strtoul(val, NULL, 10);
          else if (!strcmp(var, "next_check")) {
            if (scheduling_info_is_ok == TRUE)
              temp_service->next_check = strtoul(val, NULL, 10);
          }
          else if (!strcmp(var, "check_options")) {
            if (scheduling_info_is_ok == TRUE)
              temp_service->check_options = atoi(val);
          }
          else if (!strcmp(var, "is_flapping"))
            was_flapping = atoi(val);
          else if (!strcmp(var, "percent_state_change"))
            temp_service->percent_state_change = strtod(val, NULL);
          else
            if (!strcmp(var, "state_history")) {
              temp_ptr = val;
              for (x = 0; x < MAX_STATE_HISTORY_ENTRIES; x++) {
                if ((ch = my_strsep(&temp_ptr, ",")) != NULL)
                  temp_service->state_history[x] = atoi(ch);
                else
                  break;
              }
              temp_service->state_history_index = 0;
            }
            else
              found_directive = FALSE;
        }

        /* null-op speeds up logic */
        if (found_directive == TRUE);

        else if (!strcmp(var, "active_checks_enabled")) {
          if (temp_service->modified_attributes & MODATTR_ACTIVE_CHECKS_ENABLED)
            temp_service->checks_enabled = (atoi(val) > 0) ? TRUE : FALSE;
        }
        else if (!strcmp(var, "event_handler_enabled")) {
          if (temp_service->modified_attributes & MODATTR_EVENT_HANDLER_ENABLED)
            temp_service->event_handler_enabled = (atoi(val) > 0) ? TRUE : FALSE;
        }
        else if (!strcmp(var, "flap_detection_enabled")) {
          if (temp_service->modified_attributes & MODATTR_FLAP_DETECTION_ENABLED)
            temp_service->flap_detection_enabled = (atoi(val) > 0) ? TRUE : FALSE;
        }
        else if (!strcmp(var, "obsess_over_service")) {
          if (temp_service->modified_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED)
            temp_service->obsess_over_service = (atoi(val) > 0) ? TRUE : FALSE;
        }
        else if (!strcmp(var, "check_command")) {
          if (temp_service->modified_attributes & MODATTR_CHECK_COMMAND) {
            /* make sure the check command still exists... */
            tempval = string::dup(val);
            temp_ptr = my_strtok(tempval, "!");
            temp_command = find_command(temp_ptr);
            temp_ptr = string::dup(val);
            delete[] tempval;

            if (temp_command != NULL && temp_ptr != NULL) {
              delete[] temp_service->service_check_command;
              temp_service->service_check_command = temp_ptr;
            }
            else
              temp_service->modified_attributes -= MODATTR_CHECK_COMMAND;
          }
        }
        else if (!strcmp(var, "check_period")) {
          if (temp_service->modified_attributes & MODATTR_CHECK_TIMEPERIOD) {
            /* make sure the timeperiod still exists... */
            temp_timeperiod = find_timeperiod(val);
            temp_ptr = string::dup(val);

            if (temp_timeperiod != NULL && temp_ptr != NULL) {
              delete[] temp_service->check_period;
              temp_service->check_period = temp_ptr;
            }
            else
              temp_service->modified_attributes -= MODATTR_CHECK_TIMEPERIOD;
          }
        }
        else if (!strcmp(var, "event_handler")) {
          if (temp_service->modified_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
            /* make sure the check command still exists... */
            tempval = string::dup(val);
            temp_ptr = my_strtok(tempval, "!");
            temp_command = find_command(temp_ptr);
            temp_ptr = string::dup(val);
            delete[] tempval;

            if (temp_command != NULL && temp_ptr != NULL) {
              delete[] temp_service->event_handler;
              temp_service->event_handler = temp_ptr;
            }
            else
              temp_service->modified_attributes -= MODATTR_EVENT_HANDLER_COMMAND;
          }
        }
        else if (!strcmp(var, "normal_check_interval")) {
          if (temp_service->modified_attributes & MODATTR_NORMAL_CHECK_INTERVAL
              && strtod(val, NULL) >= 0)
            temp_service->check_interval = strtod(val, NULL);
        }
        else if (!strcmp(var, "retry_check_interval")) {
          if (temp_service->modified_attributes & MODATTR_RETRY_CHECK_INTERVAL
              && strtod(val, NULL) >= 0)
            temp_service->retry_interval = strtod(val, NULL);
        }
        else if (!strcmp(var, "max_attempts")) {
          if (temp_service->modified_attributes & MODATTR_MAX_CHECK_ATTEMPTS
              && atoi(val) >= 1) {
            temp_service->max_attempts = atoi(val);

            /* adjust current attempt number if in a hard state */
            if (temp_service->state_type == HARD_STATE
                && temp_service->current_state != STATE_OK
                && temp_service->current_attempt > 1)
              temp_service->current_attempt = temp_service->max_attempts;
          }
        }
        /* custom variables */
        else if (var[0] == '_') {
          if (temp_service->modified_attributes & MODATTR_CUSTOM_VARIABLE) {

            /* get the variable name */
            customvarname = var + 1;

            for (temp_customvariablesmember = temp_service->custom_variables;
                 temp_customvariablesmember != NULL;
                 temp_customvariablesmember = temp_customvariablesmember->next) {
              if (!strcmp(customvarname, temp_customvariablesmember->variable_name)) {
                if ((x = atoi(val)) > 0 && strlen(val) > 3) {
                  delete[] temp_customvariablesmember->variable_value;
                  temp_customvariablesmember->variable_value = string::dup(val + 2);
                  temp_customvariablesmember->has_been_modified = (x > 0) ? TRUE : FALSE;
                }
                break;
              }
            }
          }
        }
        break;

      default:
        break;
      }
    }
  }

  /* free memory and close the file */
  delete[] inputbuf;
  mmap_fclose(thefile);

  if (test_scheduling == TRUE)
    gettimeofday(&tv[1], NULL);

  if (test_scheduling == TRUE) {
    runtime[0] = (double)((double)(tv[1].tv_sec - tv[0].tv_sec) +
                          (double)((tv[1].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);

    runtime[1] = (double)((double)(tv[1].tv_sec - tv[0].tv_sec) +
                          (double)((tv[1].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);

    printf("RETENTION DATA TIMES\n");
    printf("----------------------------------\n");
    printf("Read and Process:     %.6f sec\n", runtime[0]);
    printf("                      ============\n");
    printf("TOTAL:                %.6f sec\n", runtime[1]);
    printf("\n\n");
  }

  return (OK);
}
