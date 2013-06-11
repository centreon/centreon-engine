/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2009      Nagios Core Development Team and Community Contributors
** Copyright 2011-2013 Merethis
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
#include "com/centreon/engine/comments.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/sretention.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/utils.hh"
#include "com/centreon/engine/xrddefault.hh"

using namespace com::centreon::engine;

static char* xrddefault_retention_file = NULL;
static int   xrddefault_retention_file_fd = -1;

/******************************************************************/
/********************* CONFIG INITIALIZATION  *********************/
/******************************************************************/

int xrddefault_grab_config_info(char* main_config_file) {
  char* input = NULL;
  mmapfile* thefile = NULL;
  nagios_macros* mac;

  mac = get_global_macros();

  /* open the main config file for reading */
  if (!(thefile = mmap_fopen(main_config_file))) {
    logger(logging::dbg_retentiondata, logging::most)
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
    if (!(input = mmap_fgets_multiline(thefile)))
      break;

    strip(input);

    /* skip blank lines and comments */
    if (input[0] == '#' || input[0] == '\x0')
      continue;

    xrddefault_grab_config_directives(input);
  }

  /* free memory and close the file */
  delete[] input;
  mmap_fclose(thefile);

  /* initialize locations if necessary  */
  if (!xrddefault_retention_file)
    xrddefault_retention_file = my_strdup(DEFAULT_RETENTION_FILE);

  /* save the retention file macro */
  delete[] mac->x[MACRO_RETENTIONDATAFILE];
  mac->x[MACRO_RETENTIONDATAFILE] = my_strdup(xrddefault_retention_file);
  strip(mac->x[MACRO_RETENTIONDATAFILE]);

  return (OK);
}

/* process a single config directive */
int xrddefault_grab_config_directives(char* input) {
  char const* temp_ptr = NULL;
  char* varname = NULL;
  char* varvalue = NULL;

  /* get the variable name */
  if (!(temp_ptr = my_strtok(input, "=")))
    return (ERROR);
  varname = my_strdup(temp_ptr);

  /* get the variable value */
  if (!(temp_ptr = my_strtok(NULL, "\n"))) {
    delete[] varname;
    return (ERROR);
  }
  varvalue = my_strdup(temp_ptr);

  /* retention file definition */
  if (!strcmp(varname, "xrddefault_retention_file")
      || !strcmp(varname, "state_retention_file"))
    xrddefault_retention_file = my_strdup(varvalue);

  /* free memory */
  delete[] varname;
  delete[] varvalue;

  return (OK);
}

/******************************************************************/
/********************* INIT/CLEANUP FUNCTIONS *********************/
/******************************************************************/

/* initialize retention data */
int xrddefault_initialize_retention_data(char* config_file) {
  /* grab configuration data */
  if (xrddefault_grab_config_info(config_file) == ERROR)
    return (ERROR);

  if (xrddefault_retention_file_fd == -1) {
    if ((xrddefault_retention_file_fd = open(
                                          xrddefault_retention_file,
                                          O_WRONLY | O_CREAT,
                                          S_IRUSR | S_IWUSR)) == -1) {
      logger(logging::log_runtime_error, logging::basic)
        << "Error: Unable to open retention file '"
        << xrddefault_retention_file << "': " << strerror(errno);
      return (ERROR);
    }
    set_cloexec(xrddefault_retention_file_fd);
  }

  return (OK);
}

/* cleanup retention data before terminating */
int xrddefault_cleanup_retention_data(char* config_file) {
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
  logger(logging::dbg_functions, logging::basic)
    << "xrddefault_save_state_information()";

  /* make sure we have everything */
  if (!xrddefault_retention_file
      || xrddefault_retention_file_fd == -1) {
    logger(logging::log_runtime_error, logging::basic)
      << "Error: We don't have the required file names to store "
      "retention data!";
    return (ERROR);
  }

  /* what attributes should be masked out? */
  /* NOTE: host/service/contact-specific values may be added in the future, but for now we only have global masks */
  unsigned long process_host_attribute_mask = config->retained_process_host_attribute_mask();
  unsigned long process_service_attribute_mask = config->retained_process_host_attribute_mask();
  unsigned long host_attribute_mask = config->retained_host_attribute_mask();
  unsigned long service_attribute_mask = config->retained_host_attribute_mask();
  unsigned long contact_host_attribute_mask = config->retained_contact_host_attribute_mask();
  unsigned long contact_service_attribute_mask = config->retained_contact_service_attribute_mask();
  unsigned long contact_attribute_mask = 0L;

  std::ostringstream stream;
  std::streamsize ss(stream.precision());

  /* write version info to status file */
  stream
    << "##############################################\n"
       "#    CENTREON ENGINE STATE RETENTION FILE    #\n"
       "#                                            #\n"
       "# THIS FILE IS AUTOMATICALLY GENERATED BY    #\n"
       "# CENTREON ENGINE. DO NOT MODIFY THIS FILE ! #\n"
       "##############################################\n";

  time_t current_time;
  time(&current_time);

  /* write file info */
  stream
    << "info {\n"
    "created=" << static_cast<unsigned long>(current_time) << "\n"
    "}\n";

  /* save program state information */
  stream
    << "program {\n"
    "modified_host_attributes=" << (modified_host_process_attributes & ~process_host_attribute_mask) << "\n"
    "modified_service_attributes=" << (modified_service_process_attributes & ~process_service_attribute_mask) << "\n"
    "enable_notifications=" << config->enable_notifications() << "\n"
    "active_service_checks_enabled=" << config->execute_service_checks() << "\n"
    "passive_service_checks_enabled=" << config->accept_passive_service_checks() << "\n"
    "active_host_checks_enabled=" << config->execute_host_checks() << "\n"
    "passive_host_checks_enabled=" << config->accept_passive_host_checks() << "\n"
    "enable_event_handlers=" << config->enable_event_handlers() << "\n"
    "obsess_over_services=" << config->obsess_over_services() << "\n"
    "obsess_over_hosts=" << config->obsess_over_hosts() << "\n"
    "check_service_freshness=" << config->check_service_freshness() << "\n"
    "check_host_freshness=" << config->check_host_freshness() << "\n"
    "enable_flap_detection=" << config->enable_flap_detection() << "\n"
    "enable_failure_prediction=" << config->enable_failure_prediction() << "\n"
    "process_performance_data=" << config->process_performance_data() << "\n"
    "global_host_event_handler=" << config->global_host_event_handler().c_str() << "\n"
    "global_service_event_handler=" << config->global_service_event_handler().c_str() << "\n"
    "next_comment_id=" << next_comment_id << "\n"
    "next_downtime_id=" << next_downtime_id << "\n"
    "next_event_id=" << next_event_id << "\n"
    "next_problem_id=" << next_problem_id << "\n"
    "next_notification_id=" << next_notification_id << "\n"
    "}\n";

  /* save host state information */
  for (host* obj(host_list); obj; obj = obj->next) {
    stream
      << "host {\n"
      "host_name=" << obj->name << "\n"
      "modified_attributes=" << (obj->modified_attributes & ~host_attribute_mask) << "\n"
      "check_command=" << (obj->host_check_command ? obj->host_check_command : "") << "\n"
      "check_period=" << (obj->check_period ? obj->check_period : "") << "\n"
      "notification_period=" << (obj->notification_period ? obj->notification_period : "") << "\n"
      "event_handler=" << (obj->event_handler ? obj->event_handler : "") << "\n"
      "has_been_checked=" << obj->has_been_checked << "\n"
      "check_execution_time=" << std::setprecision(3) << std::fixed << obj->execution_time << std::setprecision(ss) << "\n"
      "check_latency=" << std::setprecision(3) << std::fixed << obj->latency << std::setprecision(ss) << "\n"
      "check_type=" << obj->check_type << "\n"
      "current_state=" << obj->current_state << "\n"
      "last_state=" << obj->last_state << "\n"
      "last_hard_state=" << obj->last_hard_state << "\n"
      "last_event_id=" << obj->last_event_id << "\n"
      "current_event_id=" << obj->current_event_id << "\n"
      "current_problem_id=" << obj->current_problem_id << "\n"
      "last_problem_id=" << obj->last_problem_id << "\n"
      "plugin_output=" << (obj->plugin_output ? obj->plugin_output : "") << "\n"
      "long_plugin_output=" << (obj->long_plugin_output ? obj->long_plugin_output : "") << "\n"
      "performance_data=" << (obj->perf_data ? obj->perf_data : "") << "\n"
      "last_check=" << static_cast<unsigned long>(obj->last_check) << "\n"
      "next_check=" << static_cast<unsigned long>(obj->next_check) << "\n"
      "check_options=" << obj->check_options << "\n"
      "current_attempt=" << obj->current_attempt << "\n"
      "max_attempts=" << obj->max_attempts << "\n"
      "normal_check_interval=" << obj->check_interval << "\n"
      "retry_check_interval=" << obj->check_interval << "\n"
      "state_type=" << obj->state_type << "\n"
      "last_state_change=" << static_cast<unsigned long>(obj->last_state_change) << "\n"
      "last_hard_state_change=" << static_cast<unsigned long>(obj->last_hard_state_change) << "\n"
      "last_time_up=" << static_cast<unsigned long>(obj->last_time_up) << "\n"
      "last_time_down=" << static_cast<unsigned long>(obj->last_time_down) << "\n"
      "last_time_unreachable=" << static_cast<unsigned long>(obj->last_time_unreachable) << "\n"
      "notified_on_down=" << obj->notified_on_down << "\n"
      "notified_on_unreachable=" << obj->notified_on_unreachable << "\n"
      "last_notification=" << static_cast<unsigned long>(obj->last_host_notification) << "\n"
      "current_notification_number=" << obj->current_notification_number << "\n"
      "current_notification_id=" << obj->current_notification_id << "\n"
      "notifications_enabled=" << obj->notifications_enabled << "\n"
      "problem_has_been_acknowledged=" << obj->problem_has_been_acknowledged << "\n"
      "acknowledgement_type=" << obj->acknowledgement_type << "\n"
      "active_checks_enabled=" << obj->checks_enabled << "\n"
      "passive_checks_enabled=" << obj->accept_passive_host_checks << "\n"
      "event_handler_enabled=" << obj->event_handler_enabled << "\n"
      "flap_detection_enabled=" << obj->flap_detection_enabled << "\n"
      "failure_prediction_enabled=" << obj->failure_prediction_enabled << "\n"
      "process_performance_data=" << obj->process_performance_data << "\n"
      "obsess_over_host=" << obj->obsess_over_host << "\n"
      "is_flapping=" << obj->is_flapping << "\n"
      "percent_state_change=" << std::setprecision(2) << std::fixed << obj->percent_state_change << std::setprecision(ss) << "\n"
      "check_flapping_recovery_notification=" << obj->check_flapping_recovery_notification << "\n";
    stream << "state_history=";
    for (unsigned int x = 0; x < MAX_STATE_HISTORY_ENTRIES; x++)
      stream << (x > 0 ? "," : "") << obj->state_history[(x + obj->state_history_index) % MAX_STATE_HISTORY_ENTRIES];
    stream << "\n";

    /* custom variables */
    for (customvariablesmember* member = obj->custom_variables;
         member;
         member = member->next) {
      if (member->variable_name)
        stream << "_" << member->variable_name << "="
               << member->has_been_modified << ";"
               << (member->variable_value
                   ? member->variable_value : "") << "\n";
    }
    stream << "}\n";
  }

  /* save service state information */
  for (service* obj(service_list); obj; obj = obj->next) {
    stream
      << "service {\n"
      "host_name=" << obj->host_name << "\n"
      "service_description=" << obj->description << "\n"
      "modified_attributes=" << (obj->modified_attributes & ~service_attribute_mask) << "\n"
      "check_command=" << (obj->service_check_command ? obj->service_check_command : "") << "\n"
      "check_period=" << (obj->check_period ? obj->check_period : "") << "\n"
      "notification_period=" << (obj->notification_period ? obj->notification_period : "") << "\n"
      "event_handler=" << (obj->event_handler ? obj->event_handler : "") << "\n"
      "has_been_checked=" << obj->has_been_checked << "\n"
      "check_execution_time=" << std::setprecision(3) << std::fixed << obj->execution_time << std::setprecision(ss) << "\n"
      "check_latency=" << std::setprecision(3) << std::fixed << obj->latency << std::setprecision(ss) << "\n"
      "check_type=" << obj->check_type << "\n"
      "current_state=" << obj->current_state << "\n"
      "last_state=" << obj->last_state << "\n"
      "last_hard_state=" << obj->last_hard_state << "\n"
      "last_event_id=" << obj->last_event_id << "\n"
      "current_event_id=" << obj->current_event_id << "\n"
      "current_problem_id=" << obj->current_problem_id << "\n"
      "last_problem_id=" << obj->last_problem_id << "\n"
      "current_attempt=" << obj->current_attempt << "\n"
      "max_attempts=" << obj->max_attempts << "\n"
      "normal_check_interval=" << obj->check_interval << "\n"
      "retry_check_interval=" << obj->retry_interval << "\n"
      "state_type=" << obj->state_type << "\n"
      "last_state_change=" << static_cast<unsigned long>(obj->last_state_change) << "\n"
      "last_hard_state_change=" << static_cast<unsigned long>(obj->last_hard_state_change) << "\n"
      "last_time_ok=" << static_cast<unsigned long>(obj->last_time_ok) << "\n"
      "last_time_warning=" << static_cast<unsigned long>(obj->last_time_warning) << "\n"
      "last_time_unknown=" << static_cast<unsigned long>(obj->last_time_unknown) << "\n"
      "last_time_critical=" << static_cast<unsigned long>(obj->last_time_critical) << "\n"
      "plugin_output=" << (obj->plugin_output ? obj->plugin_output : "") << "\n"
      "long_plugin_output=" << (obj->long_plugin_output ? obj->long_plugin_output : "") << "\n"
      "performance_data=" << (obj->perf_data ? obj->perf_data : "") << "\n"
      "last_check=" << static_cast<unsigned long>(obj->last_check) << "\n"
      "next_check=" << static_cast<unsigned long>(obj->next_check) << "\n"
      "check_options=" << obj->check_options << "\n"
      "notified_on_unknown=" << obj->notified_on_unknown << "\n"
      "notified_on_warning=" << obj->notified_on_warning << "\n"
      "notified_on_critical=" << obj->notified_on_critical << "\n"
      "current_notification_number=" << obj->current_notification_number << "\n"
      "current_notification_id=" << obj->current_notification_id << "\n"
      "last_notification=" << static_cast<unsigned long>(obj->last_notification) << "\n"
      "notifications_enabled=" << obj->notifications_enabled << "\n"
      "active_checks_enabled=" << obj->checks_enabled << "\n"
      "passive_checks_enabled=" << obj->accept_passive_service_checks << "\n"
      "event_handler_enabled=" << obj->event_handler_enabled << "\n"
      "problem_has_been_acknowledged=" << obj->problem_has_been_acknowledged << "\n"
      "acknowledgement_type=" << obj->acknowledgement_type << "\n"
      "flap_detection_enabled=" << obj->flap_detection_enabled << "\n"
      "failure_prediction_enabled=" << obj->failure_prediction_enabled << "\n"
      "process_performance_data=" << obj->process_performance_data << "\n"
      "obsess_over_service=" << obj->obsess_over_service << "\n"
      "is_flapping=" << obj->is_flapping << "\n"
      "percent_state_change=" << std::setprecision(2) << std::fixed << obj->percent_state_change << std::setprecision(ss) << "\n"
      "check_flapping_recovery_notification=" << obj->check_flapping_recovery_notification << "\n";

    stream << "state_history=";
    for (unsigned int x = 0; x < MAX_STATE_HISTORY_ENTRIES; x++)
      stream << (x > 0 ? "," : "") << obj->state_history[(x + obj->state_history_index) % MAX_STATE_HISTORY_ENTRIES];
    stream << "\n";

    /* custom variables */
    for (customvariablesmember* member = obj->custom_variables;
         member;
         member = member->next) {
      if (member->variable_name)
        stream << "_" << member->variable_name << "="
               << member->has_been_modified << ";"
               << (member->variable_value
                   ? member->variable_value : "") << "\n";
    }
    stream << "}\n";
  }

  /* save contact state information */
  for (contact* obj(contact_list); obj; obj = obj->next) {
    stream
      << "contact {\n"
      "contact_name=" << obj->name << "\n"
      "modified_attributes=" << (obj->modified_attributes & ~contact_attribute_mask) << "\n"
      "modified_host_attributes=" << (obj->modified_host_attributes & ~contact_host_attribute_mask) << "\n"
      "modified_service_attributes=" << (obj->modified_service_attributes & ~contact_service_attribute_mask) << "\n"
      "host_notification_period=" << (obj->host_notification_period ? obj->host_notification_period : "") << "\n"
      "service_notification_period=" << (obj->service_notification_period ? obj->service_notification_period : "") << "\n"
      "last_host_notification=" << static_cast<unsigned long>(obj->last_host_notification) << "\n"
      "last_service_notification=" << static_cast<unsigned long>(obj->last_service_notification) << "\n"
      "host_notifications_enabled=" << obj->host_notifications_enabled << "\n"
      "service_notifications_enabled=" << obj->service_notifications_enabled << "\n";

    /* custom variables */
    for (customvariablesmember* member = obj->custom_variables;
         member;
         member = member->next) {
      if (member->variable_name)
        stream << "_" << member->variable_name << "="
               << member->has_been_modified << ";"
               << (member->variable_value ? member->variable_value : "") << "\n";
    }

    stream << "}\n";
  }

  /* save all comments */
  for (comment* obj = comment_list;
       obj;
       obj = obj->next) {

    if (obj->comment_type == HOST_COMMENT)
      stream << "hostcomment {\n";
    else
      stream << "servicecomment {\n";
    stream << "host_name=" << obj->host_name << "\n";
    if (obj->comment_type == SERVICE_COMMENT)
      stream << "service_description=" << obj->service_description << "\n";
    stream
      << "entry_type=" << obj->entry_type << "\n"
      "comment_id=" << obj->comment_id << "\n"
      "source=" << obj->source << "\n"
      "persistent=" << obj->persistent << "\n"
      "entry_time=" << static_cast<unsigned long>(obj->entry_time) << "\n"
      "expires=" << obj->expires << "\n"
      "expire_time=" << static_cast<unsigned long>(obj->expire_time) << "\n"
      "author=" << obj->author << "\n"
      "comment_data=" << obj->comment_data << "\n"
      "}\n";
  }

  /* save all downtime */
  for (scheduled_downtime* obj(scheduled_downtime_list);
       obj;
       obj = obj->next) {

    if (obj->type == HOST_DOWNTIME)
      stream << "hostdowntime {\n";
    else
      stream << "servicedowntime {\n";
    stream << "host_name=" << obj->host_name << "\n";
    if (obj->type == SERVICE_DOWNTIME)
      stream << "service_description=" << obj->service_description << "\n";
    stream
      << "downtime_id=" << obj->downtime_id << "\n"
      "entry_time=" << static_cast<unsigned long>(obj->entry_time) << "\n"
      "start_time=" << static_cast<unsigned long>(obj->start_time) << "\n"
      "end_time=" << static_cast<unsigned long>(obj->end_time) << "\n"
      "triggered_by=" << obj->triggered_by << "\n"
      "fixed=" << obj->fixed << "\n"
      "duration=" << obj->duration << "\n"
      "author=" << obj->author << "\n"
      "comment=" << obj->comment << "\n"
      "}\n";
  }

  // Write data in buffer.
  stream.flush();

  // Prepare retention file for overwrite.
  if ((ftruncate(xrddefault_retention_file_fd, 0) == -1)
      || (fsync(xrddefault_retention_file_fd) == -1)
      || (lseek(xrddefault_retention_file_fd, 0, SEEK_SET)
          == (off_t)-1)) {
    char const* msg(strerror(errno));
    logger(logging::log_runtime_error, logging::basic)
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
      logger(logging::log_runtime_error, logging::basic)
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
  char* input(NULL);
  char* inputbuf(NULL);
  char* temp_ptr(NULL);
  mmapfile* thefile;
  char* host_name(NULL);
  char* service_description(NULL);
  char* contact_name(NULL);
  char* author(NULL);
  char* comment_data(NULL);
  unsigned int data_type(XRDDEFAULT_NO_DATA);
  unsigned int x(0);
  host* hst(NULL);
  service* svc(NULL);
  contact* cntct(NULL);
  command* cmd(NULL);
  timeperiod* tperiod(NULL);
  customvariablesmember* member(NULL);
  char* customvarname(NULL);
  char* var(NULL);
  char* val(NULL);
  char* tempval(NULL);
  char* ch(NULL);
  unsigned long comment_id(0);
  int persistent(false);
  int expires(false);
  time_t expire_time(0L);
  unsigned int entry_type(USER_COMMENT);
  int source(COMMENTSOURCE_INTERNAL);
  time_t entry_time(0L);
  time_t creation_time;
  time_t current_time;
  int scheduling_info_is_ok(false);
  unsigned long downtime_id(0);
  time_t start_time(0L);
  time_t end_time(0L);
  int fixed(false);
  unsigned long triggered_by(0);
  unsigned long duration(0L);
  unsigned long host_attribute_mask(0L);
  unsigned long service_attribute_mask(0L);
  unsigned long contact_attribute_mask(0L);
  unsigned long contact_host_attribute_mask(0L);
  unsigned long contact_service_attribute_mask(0L);
  unsigned long process_host_attribute_mask(0L);
  unsigned long process_service_attribute_mask(0L);
  int remove_comment(false);
  int ack(false);
  int was_flapping(false);
  int allow_flapstart_notification(true);
  struct timeval tv[2];
  int found_directive(false);

  logger(logging::dbg_functions, logging::basic)
    << "xrddefault_read_state_information()";

  /* make sure we have what we need */
  if (!xrddefault_retention_file) {
    logger(logging::log_runtime_error, logging::basic)
      << "Error: We don't have a filename for retention data!";
    return (ERROR);
  }

  if (test_scheduling)
    gettimeofday(&tv[0], NULL);

  /* open the retention file for reading */
  if (!(thefile = mmap_fopen(xrddefault_retention_file)))
    return (ERROR);

  /* what attributes should be masked out? */
  /* NOTE: host/service/contact-specific values may be added in the future, but for now we only have global masks */
  process_host_attribute_mask = config->retained_process_host_attribute_mask();
  process_service_attribute_mask = config->retained_process_host_attribute_mask();
  host_attribute_mask = config->retained_host_attribute_mask();
  service_attribute_mask = config->retained_host_attribute_mask();
  // contact_host_attribute_mask = config->retained_contact_host_attribute_mask();
  // contact_service_attribute_mask = config->retained_contact_service_attribute_mask();

  /* Big speedup when reading retention.dat in bulk */
  defer_downtime_sorting = 1;
  defer_comment_sorting = 1;

  /* read all lines in the retention file */
  while (1) {
    /* free memory */
    delete[] inputbuf;

    /* read the next line */
    if (!(inputbuf = mmap_fgets(thefile)))
      break;

    input = inputbuf;

    /* far better than strip()ing */
    if (input[0] == '\t')
      input++;

    strip(input);

    if (!strcmp(input, "service {"))
      data_type = XRDDEFAULT_SERVICESTATUS_DATA;
    // else if (!strcmp(input, "host {"))
    //   data_type = XRDDEFAULT_HOSTSTATUS_DATA;
    else if (!strcmp(input, "contact {"))
      data_type = XRDDEFAULT_CONTACTSTATUS_DATA;
    else if (!strcmp(input, "hostcomment {"))
      data_type = XRDDEFAULT_HOSTCOMMENT_DATA;
    else if (!strcmp(input, "servicecomment {"))
      data_type = XRDDEFAULT_SERVICECOMMENT_DATA;
    else if (!strcmp(input, "hostdowntime {"))
      data_type = XRDDEFAULT_HOSTDOWNTIME_DATA;
    else if (!strcmp(input, "servicedowntime {"))
      data_type = XRDDEFAULT_SERVICEDOWNTIME_DATA;
    else if (!strcmp(input, "info {"))
      data_type = XRDDEFAULT_INFO_DATA;
    else if (!strcmp(input, "program {"))
      data_type = XRDDEFAULT_PROGRAMSTATUS_DATA;
    else if (!strcmp(input, "}")) {
      switch (data_type) {
      case XRDDEFAULT_INFO_DATA:
        break;

      case XRDDEFAULT_PROGRAMSTATUS_DATA:
        /* adjust modified attributes if necessary */
        if (!config->use_retained_program_state()) {
          modified_host_process_attributes = MODATTR_NONE;
          modified_service_process_attributes = MODATTR_NONE;
        }
        break;

      // case XRDDEFAULT_HOSTSTATUS_DATA:
      //   if (hst) {
      //     /* adjust modified attributes if necessary */
      //     if (!hst->retain_nonstatus_information)
      //       hst->modified_attributes = MODATTR_NONE;

      //     /* adjust modified attributes if no custom variables have been changed */
      //     if (hst->modified_attributes & MODATTR_CUSTOM_VARIABLE) {
      //       for (member = hst->custom_variables;
      //            member;
      //            member = member->next) {
      //         if (member->has_been_modified)
      //           break;
      //       }
      //       if (!member)
      //         hst->modified_attributes -= MODATTR_CUSTOM_VARIABLE;
      //     }

      //     /* calculate next possible notification time */
      //     if (hst->current_state != HOST_UP
      //         && hst->last_host_notification)
      //       hst->next_host_notification = get_next_host_notification_time(hst, hst->last_host_notification);

      //     /* ADDED 01/23/2009 adjust current check attempts if host in hard problem state (max attempts may have changed in config since restart) */
      //     if (hst->current_state != HOST_UP
      //         && hst->state_type == HARD_STATE)
      //       hst->current_attempt = hst->max_attempts;

      //     /* ADDED 02/20/08 assume same flapping state if large install tweaks enabled */
      //     if (config->use_large_installation_tweaks())
      //       hst->is_flapping = was_flapping;
      //     /* else use normal startup flap detection logic */
      //     else {
      //       /* host was flapping before program started */
      //       /* 11/10/07 don't allow flapping notifications to go out */
      //       allow_flapstart_notification = (was_flapping ? false : true);


      //       /* check for flapping */
      //       check_for_host_flapping(
      //         hst,
      //         false,
      //         false,
      //         allow_flapstart_notification);

      //       /* host was flapping before and isn't now, so clear recovery check variable if host isn't flapping now */
      //       if (was_flapping && !hst->is_flapping)
      //         hst->check_flapping_recovery_notification = false;
      //     }

      //     /* handle new vars added in 2.x */
      //     if (!hst->last_hard_state_change)
      //       hst->last_hard_state_change = hst->last_state_change;

      //     /* update host status */
      //     update_host_status(hst, false);
      //   }

      //   /* reset vars */
      //   was_flapping = false;
      //   allow_flapstart_notification = true;

      //   delete[] host_name;
      //   host_name = NULL;
      //   hst = NULL;
      //   break;

      case XRDDEFAULT_SERVICESTATUS_DATA:
        if (svc) {
          /* adjust modified attributes if necessary */
          if (!svc->retain_nonstatus_information)
            svc->modified_attributes = MODATTR_NONE;

          /* adjust modified attributes if no custom variables have been changed */
          if (svc->modified_attributes & MODATTR_CUSTOM_VARIABLE) {
            for (member = svc->custom_variables;
                 member;
                 member = member->next) {
              if (member->has_been_modified)
                break;
            }
            if (!member)
              svc->modified_attributes -= MODATTR_CUSTOM_VARIABLE;
          }

          /* calculate next possible notification time */
          if (svc->current_state != STATE_OK
              && svc->last_notification)
            svc->next_notification
              = get_next_service_notification_time(
                  svc,
                  svc->last_notification);

          /* fix old vars */
          if (!svc->has_been_checked
              && svc->state_type == SOFT_STATE)
            svc->state_type = HARD_STATE;

          /* ADDED 01/23/2009 adjust current check attempt if service is in hard problem state (max attempts may have changed in config since restart) */
          if (svc->current_state != STATE_OK
              && svc->state_type == HARD_STATE)
            svc->current_attempt = svc->max_attempts;


          /* ADDED 02/20/08 assume same flapping state if large install tweaks enabled */
          if (config->use_large_installation_tweaks())
            svc->is_flapping = was_flapping;
          /* else use normal startup flap detection logic */
          else {
            /* service was flapping before program started */
            /* 11/10/07 don't allow flapping notifications to go out */
            allow_flapstart_notification = (was_flapping ? false : true);

            /* check for flapping */
            check_for_service_flapping(
              svc,
              false,
              allow_flapstart_notification);

            /* service was flapping before and isn't now, so clear recovery check variable if service isn't flapping now */
            if (was_flapping && !svc->is_flapping)
              svc->check_flapping_recovery_notification = false;
          }

          /* handle new vars added in 2.x */
          if (svc->last_hard_state_change)
            svc->last_hard_state_change = svc->last_state_change;

          /* update service status */
          update_service_status(svc, false);
        }

        /* reset vars */
        was_flapping = false;
        allow_flapstart_notification = true;

        delete[] host_name;
        delete[] service_description;
        host_name = NULL;
        service_description = NULL;
        svc = NULL;
        break;

      case XRDDEFAULT_CONTACTSTATUS_DATA:
        if (cntct) {
          /* adjust modified attributes if necessary */
          if (!cntct->retain_nonstatus_information)
            cntct->modified_attributes = MODATTR_NONE;

          /* adjust modified attributes if no custom variables have been changed */
          if (cntct->modified_attributes & MODATTR_CUSTOM_VARIABLE) {
            for (member = cntct->custom_variables;
                 member;
                 member = member->next) {
              if (member->has_been_modified)
                break;
            }
            if (!member)
              cntct->modified_attributes -= MODATTR_CUSTOM_VARIABLE;
          }

          /* update contact status */
          update_contact_status(cntct, false);
        }

        delete[] contact_name;
        contact_name = NULL;
        cntct = NULL;
        break;

      case XRDDEFAULT_HOSTCOMMENT_DATA:
      case XRDDEFAULT_SERVICECOMMENT_DATA:
        /* add the comment */
        add_comment((data_type == XRDDEFAULT_HOSTCOMMENT_DATA) ? HOST_COMMENT : SERVICE_COMMENT,
		    entry_type,
		    host_name,
		    service_description,
		    entry_time,
		    author,
                    comment_data,
		    comment_id,
		    persistent,
		    expires,
		    expire_time,
		    source);

        /* delete the comment if necessary */
        /* it seems a bit backwards to add and then immediately delete the comment, but its necessary to track comment deletions in the event broker */
        remove_comment = false;
        /* host no longer exists */
        if (!(hst = find_host(host_name)))
          remove_comment = true;
        /* service no longer exists */
        else if (data_type == XRDDEFAULT_SERVICECOMMENT_DATA
                 && !(svc = find_service(host_name, service_description)))
          remove_comment = true;
        /* acknowledgement comments get deleted if they're not persistent and the original problem is no longer acknowledged */
        else if (entry_type == ACKNOWLEDGEMENT_COMMENT) {
          ack = false;
          if (data_type == XRDDEFAULT_HOSTCOMMENT_DATA)
            ack = hst->problem_has_been_acknowledged;
          else
            ack = svc->problem_has_been_acknowledged;
          if (!ack && !persistent)
            remove_comment = true;
        }
        /* non-persistent comments don't last past restarts UNLESS they're acks (see above) */
        else if (!persistent)
          remove_comment = true;

        if (remove_comment)
          delete_comment((data_type == XRDDEFAULT_HOSTCOMMENT_DATA) ? HOST_COMMENT : SERVICE_COMMENT, comment_id);

        /* free temp memory */
        delete[] host_name;
        delete[] service_description;
        delete[] author;
        delete[] comment_data;

        host_name = NULL;
        service_description = NULL;
        author = NULL;
        comment_data = NULL;

        /* reset defaults */
        entry_type = USER_COMMENT;
        comment_id = 0;
        source = COMMENTSOURCE_INTERNAL;
        persistent = false;
        entry_time = 0L;
        expires = false;
        expire_time = 0L;
        break;

      case XRDDEFAULT_HOSTDOWNTIME_DATA:
      case XRDDEFAULT_SERVICEDOWNTIME_DATA:
        /* add the downtime */
        if (data_type == XRDDEFAULT_HOSTDOWNTIME_DATA)
          add_host_downtime(
            host_name,
            entry_time,
            author,
            comment_data,
            start_time,
            end_time,
            fixed,
            triggered_by,
            duration,
            downtime_id);
        else
          add_service_downtime(
            host_name,
            service_description,
            entry_time,
            author,
            comment_data,
            start_time,
            end_time,
            fixed,
            triggered_by,
            duration,
            downtime_id);

        /* must register the downtime with Centreon Engine so it can schedule it, add comments, etc. */
        register_downtime(
          (data_type == XRDDEFAULT_HOSTDOWNTIME_DATA
           ? HOST_DOWNTIME
           : SERVICE_DOWNTIME),
          downtime_id);

        /* free temp memory */
        delete[] host_name;
        delete[] service_description;
        delete[] author;
        delete[] comment_data;

        host_name = NULL;
        service_description = NULL;
        author = NULL;
        comment_data = NULL;

        /* reset defaults */
        downtime_id = 0;
        entry_time = 0L;
        start_time = 0L;
        end_time = 0L;
        fixed = false;
        triggered_by = 0;
        duration = 0L;
        break;

      default:
        break;
      }

      data_type = XRDDEFAULT_NO_DATA;
    }
    else if (data_type != XRDDEFAULT_NO_DATA) {
      /* slightly faster than strtok () */
      var = input;
      if (!(val = strchr(input, '=')))
        continue;
      val[0] = '\x0';
      val++;

      found_directive = true;

      switch (data_type) {
      case XRDDEFAULT_INFO_DATA:
        if (!strcmp(var, "created")) {
          creation_time = strtoul(val, NULL, 10);
          time(&current_time);
          if ((current_time - creation_time)
              < static_cast<time_t>(config->retention_scheduling_horizon()))
            scheduling_info_is_ok = true;
          else
            scheduling_info_is_ok = false;
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

          /* mask out attributes we don't want to retain */
          modified_host_process_attributes &= ~process_host_attribute_mask;
        }
        else if (!strcmp(var, "modified_service_attributes")) {
          modified_service_process_attributes = strtoul(val, NULL, 10);

          /* mask out attributes we don't want to retain */
          modified_service_process_attributes &= ~process_service_attribute_mask;
        }
        if (config->use_retained_program_state()) {
          if (!strcmp(var, "enable_notifications")) {
            if (modified_host_process_attributes & MODATTR_NOTIFICATIONS_ENABLED)
              config->enable_notifications(atoi(val) > 0);
          }
          else if (!strcmp(var, "active_service_checks_enabled")) {
            if (modified_service_process_attributes & MODATTR_ACTIVE_CHECKS_ENABLED)
              config->execute_service_checks(atoi(val) > 0);
          }
          else if (!strcmp(var, "passive_service_checks_enabled")) {
            if (modified_service_process_attributes & MODATTR_PASSIVE_CHECKS_ENABLED)
              config->accept_passive_service_checks(atoi(val) > 0);
          }
          else if (!strcmp(var, "active_host_checks_enabled")) {
            if (modified_host_process_attributes & MODATTR_ACTIVE_CHECKS_ENABLED)
              config->execute_host_checks(atoi(val) > 0);
          }
          else if (!strcmp(var, "passive_host_checks_enabled")) {
            if (modified_host_process_attributes & MODATTR_PASSIVE_CHECKS_ENABLED)
              config->accept_passive_host_checks(atoi(val) > 0);
          }
          else if (!strcmp(var, "enable_event_handlers")) {
            if (modified_host_process_attributes & MODATTR_EVENT_HANDLER_ENABLED)
              config->enable_event_handlers(atoi(val) > 0);
          }
          else if (!strcmp(var, "obsess_over_services")) {
            if (modified_service_process_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED)
              config->obsess_over_services(atoi(val) > 0);
          }
          else if (!strcmp(var, "obsess_over_hosts")) {
            if (modified_host_process_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED)
              config->obsess_over_hosts(atoi(val) > 0);
          }
          else if (!strcmp(var, "check_service_freshness")) {
            if (modified_service_process_attributes & MODATTR_FRESHNESS_CHECKS_ENABLED)
              config->check_service_freshness(atoi(val) > 0);
          }
          else if (!strcmp(var, "check_host_freshness")) {
            if (modified_host_process_attributes & MODATTR_FRESHNESS_CHECKS_ENABLED)
              config->check_host_freshness(atoi(val) > 0);
          }
          else if (!strcmp(var, "enable_flap_detection")) {
            if (modified_host_process_attributes & MODATTR_FLAP_DETECTION_ENABLED)
              config->enable_flap_detection(atoi(val) > 0);
          }
          else if (!strcmp(var, "enable_failure_prediction")) {
            if (modified_host_process_attributes & MODATTR_FAILURE_PREDICTION_ENABLED)
              config->enable_failure_prediction(atoi(val) > 0);
          }
          else if (!strcmp(var, "process_performance_data")) {
            if (modified_host_process_attributes & MODATTR_PERFORMANCE_DATA_ENABLED)
              config->process_performance_data(atoi(val) > 0);
          }
          else if (!strcmp(var, "global_host_event_handler")) {
            if (modified_host_process_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
              /* make sure the check command still exists... */
              tempval = my_strdup(val);
              temp_ptr = my_strtok(tempval, "!");
              cmd = find_command(temp_ptr);
              temp_ptr = my_strdup(val);
              delete[] tempval;

              if (cmd && temp_ptr) {
                config->global_host_event_handler(temp_ptr);
              }
            }
          }
          else if (!strcmp(var, "global_service_event_handler")) {
            if (modified_service_process_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
              /* make sure the check command still exists... */
              tempval = my_strdup(val);
              temp_ptr = my_strtok(tempval, "!");
              cmd = find_command(temp_ptr);
              temp_ptr = my_strdup(val);
              delete[] tempval;

              if (cmd && temp_ptr) {
                config->global_service_event_handler(temp_ptr);
              }
            }
          }
          else if (!strcmp(var, "next_comment_id"))
            next_comment_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "next_downtime_id"))
            next_downtime_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "next_event_id"))
            next_event_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "next_problem_id"))
            next_problem_id = strtoul(val, NULL, 10);
          else if (!strcmp(var, "next_notification_id"))
            next_notification_id = strtoul(val, NULL, 10);
        }
        break;

      // case XRDDEFAULT_HOSTSTATUS_DATA:
      //   if (!hst) {
      //     if (!strcmp(var, "host_name")) {
      //       host_name = my_strdup(val);
      //       hst = find_host(host_name);
      //     }
      //   }
      //   else {
      //     if (!strcmp(var, "modified_attributes")) {
      //       hst->modified_attributes = strtoul(val, NULL, 10);

      //       /* mask out attributes we don't want to retain */
      //       hst->modified_attributes &= ~host_attribute_mask;

      //       /* break out */
      //       break;
      //     }
      //     if (hst->retain_status_information) {
      //       if (!strcmp(var, "has_been_checked"))
      //         hst->has_been_checked = (atoi(val) > 0);
      //       else if (!strcmp(var, "check_execution_time"))
      //         hst->execution_time = strtod(val, NULL);
      //       else if (!strcmp(var, "check_latency"))
      //         hst->latency = strtod(val, NULL);
      //       else if (!strcmp(var, "check_type"))
      //         hst->check_type = atoi(val);
      //       else if (!strcmp(var, "current_state"))
      //         hst->current_state = atoi(val);
      //       else if (!strcmp(var, "last_state"))
      //         hst->last_state = atoi(val);
      //       else if (!strcmp(var, "last_hard_state"))
      //         hst->last_hard_state = atoi(val);
      //       else if (!strcmp(var, "plugin_output")) {
      //         delete[] hst->plugin_output;
      //         hst->plugin_output = my_strdup(val);
      //       }
      //       else if (!strcmp(var, "long_plugin_output")) {
      //         delete[] hst->long_plugin_output;
      //         hst->long_plugin_output = my_strdup(val);
      //       }
      //       else if (!strcmp(var, "performance_data")) {
      //         delete[] hst->perf_data;
      //         hst->perf_data = my_strdup(val);
      //       }
      //       else if (!strcmp(var, "last_check"))
      //         hst->last_check = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "next_check")) {
      //         if (config->use_retained_scheduling_info()
      //             && scheduling_info_is_ok)
      //           hst->next_check = strtoul(val, NULL, 10);
      //       }
      //       else if (!strcmp(var, "check_options")) {
      //         if (config->use_retained_scheduling_info()
      //             && scheduling_info_is_ok)
      //           hst->check_options = atoi(val);
      //       }
      //       else if (!strcmp(var, "current_attempt"))
      //         hst->current_attempt = (atoi(val) > 0);
      //       else if (!strcmp(var, "current_event_id"))
      //         hst->current_event_id = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "last_event_id"))
      //         hst->last_event_id = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "current_problem_id"))
      //         hst->current_problem_id = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "last_problem_id"))
      //         hst->last_problem_id = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "state_type"))
      //         hst->state_type = atoi(val);
      //       else if (!strcmp(var, "last_state_change"))
      //         hst->last_state_change = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "last_hard_state_change"))
      //         hst->last_hard_state_change = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "last_time_up"))
      //         hst->last_time_up = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "last_time_down"))
      //         hst->last_time_down = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "last_time_unreachable"))
      //         hst->last_time_unreachable = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "notified_on_down"))
      //         hst->notified_on_down = (atoi(val) > 0);
      //       else if (!strcmp(var, "notified_on_unreachable"))
      //         hst->notified_on_unreachable = (atoi(val) > 0);
      //       else if (!strcmp(var, "last_notification"))
      //         hst->last_host_notification = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "current_notification_number"))
      //         hst->current_notification_number = atoi(val);
      //       else if (!strcmp(var, "current_notification_id"))
      //         hst->current_notification_id = strtoul(val, NULL, 10);
      //       else if (!strcmp(var, "is_flapping"))
      //         was_flapping = atoi(val);
      //       else if (!strcmp(var, "percent_state_change"))
      //         hst->percent_state_change = strtod(val, NULL);
      //       else
      //         if (!strcmp(var, "check_flapping_recovery_notification"))
      //           hst->check_flapping_recovery_notification = atoi(val);
      //         else if (!strcmp(var, "state_history")) {
      //           temp_ptr = val;
      //           for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x) {
      //             if ((ch = my_strsep(&temp_ptr, ",")) != NULL)
      //               hst->state_history[x] = atoi(ch);
      //             else
      //               break;
      //           }
      //           hst->state_history_index = 0;
      //         }
      //         else
      //           found_directive = false;
      //     }
      //     if (hst->retain_nonstatus_information) {
      //       /* null-op speeds up logic */
      //       if (found_directive);

      //       else if (!strcmp(var, "problem_has_been_acknowledged"))
      //         hst->problem_has_been_acknowledged = (atoi(val) > 0);
      //       else if (!strcmp(var, "acknowledgement_type"))
      //         hst->acknowledgement_type = atoi(val);
      //       else if (!strcmp(var, "notifications_enabled")) {
      //         if (hst->modified_attributes & MODATTR_NOTIFICATIONS_ENABLED)
      //           hst->notifications_enabled = (atoi(val) > 0);
      //       }
      //       else if (!strcmp(var, "active_checks_enabled")) {
      //         if (hst->modified_attributes & MODATTR_ACTIVE_CHECKS_ENABLED)
      //           hst->checks_enabled = (atoi(val) > 0);
      //       }
      //       else if (!strcmp(var, "passive_checks_enabled")) {
      //         if (hst->modified_attributes & MODATTR_PASSIVE_CHECKS_ENABLED)
      //           hst->accept_passive_host_checks = (atoi(val) > 0);
      //       }
      //       else if (!strcmp(var, "event_handler_enabled")) {
      //         if (hst->modified_attributes & MODATTR_EVENT_HANDLER_ENABLED)
      //           hst->event_handler_enabled = (atoi(val) > 0);
      //       }
      //       else if (!strcmp(var, "flap_detection_enabled")) {
      //         if (hst->modified_attributes & MODATTR_FLAP_DETECTION_ENABLED)
      //           hst->flap_detection_enabled = (atoi(val) > 0);
      //       }
      //       else if (!strcmp(var, "failure_prediction_enabled")) {
      //         if (hst->modified_attributes & MODATTR_FAILURE_PREDICTION_ENABLED)
      //           hst->failure_prediction_enabled = (atoi(val) > 0);
      //       }
      //       else if (!strcmp(var, "process_performance_data")) {
      //         if (hst->modified_attributes & MODATTR_PERFORMANCE_DATA_ENABLED)
      //           hst->process_performance_data = (atoi(val) > 0);
      //       }
      //       else if (!strcmp(var, "obsess_over_host")) {
      //         if (hst->modified_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED)
      //           hst->obsess_over_host = (atoi(val) > 0);
      //       }
      //       else if (!strcmp(var, "check_command")) {
      //         if (hst->modified_attributes & MODATTR_CHECK_COMMAND) {
      //           /* make sure the check command still exists... */
      //           tempval = my_strdup(val);
      //           temp_ptr = my_strtok(tempval, "!");
      //           cmd = find_command(temp_ptr);
      //           temp_ptr = my_strdup(val);
      //           delete[] tempval;

      //           if (cmd && temp_ptr) {
      //             delete[] hst->host_check_command;
      //             hst->host_check_command = temp_ptr;
      //           }
      //           else
      //             hst->modified_attributes -= MODATTR_CHECK_COMMAND;
      //         }
      //       }
      //       else if (!strcmp(var, "check_period")) {
      //         if (hst->modified_attributes & MODATTR_CHECK_TIMEPERIOD) {
      //           /* make sure the timeperiod still exists... */
      //           tperiod = find_timeperiod(val);
      //           temp_ptr = my_strdup(val);

      //           if (tperiod && temp_ptr) {
      //             delete[] hst->check_period;
      //             hst->check_period = temp_ptr;
      //           }
      //           else
      //             hst->modified_attributes -= MODATTR_CHECK_TIMEPERIOD;
      //         }
      //       }
      //       else if (!strcmp(var, "notification_period")) {
      //         if (hst->modified_attributes & MODATTR_NOTIFICATION_TIMEPERIOD) {
      //           /* make sure the timeperiod still exists... */
      //           tperiod = find_timeperiod(val);
      //           temp_ptr = my_strdup(val);

      //           if (tperiod && temp_ptr) {
      //             delete[] hst->notification_period;
      //             hst->notification_period = temp_ptr;
      //           }
      //           else
      //             hst->modified_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
      //         }
      //       }
      //       else if (!strcmp(var, "event_handler")) {
      //         if (hst->modified_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
      //           /* make sure the check command still exists... */
      //           tempval = my_strdup(val);
      //           temp_ptr = my_strtok(tempval, "!");
      //           cmd = find_command(temp_ptr);
      //           temp_ptr = my_strdup(val);
      //           delete[] tempval;

      //           if (cmd && temp_ptr) {
      //             delete[] hst->event_handler;
      //             hst->event_handler = temp_ptr;
      //           }
      //           else
      //             hst->modified_attributes -= MODATTR_EVENT_HANDLER_COMMAND;
      //         }
      //       }
      //       else if (!strcmp(var, "normal_check_interval")) {
      //         if (hst->modified_attributes & MODATTR_NORMAL_CHECK_INTERVAL
      //             && strtod(val, NULL) >= 0)
      //           hst->check_interval = strtod(val, NULL);
      //       }
      //       else if (!strcmp(var, "retry_check_interval")) {
      //         if (hst->modified_attributes & MODATTR_RETRY_CHECK_INTERVAL
      //             && strtod(val, NULL) >= 0)
      //           hst->retry_interval = strtod(val, NULL);
      //       }
      //       else if (!strcmp(var, "max_attempts")) {
      //         if (hst->modified_attributes & MODATTR_MAX_CHECK_ATTEMPTS
      //             && atoi(val) >= 1) {
      //           hst->max_attempts = atoi(val);

      //           /* adjust current attempt number if in a hard state */
      //           if (hst->state_type == HARD_STATE
      //               && hst->current_state != HOST_UP
      //               && hst->current_attempt > 1)
      //             hst->current_attempt = hst->max_attempts;
      //         }
      //       }
      //       /* custom variables */
      //       else if (var[0] == '_') {
      //         if (hst->modified_attributes & MODATTR_CUSTOM_VARIABLE) {

      //           /* get the variable name */
      //           customvarname = var + 1;

      //           for (member = hst->custom_variables;
      //                member;
      //                member = member->next) {
      //             if (!strcmp(customvarname, member->variable_name)) {
      //               if ((x = atoi(val)) > 0 && strlen(val) > 3) {
      //                 delete[] member->variable_value;
      //                 member->variable_value = my_strdup(val + 2);
      //                 member->has_been_modified = (x > 0);
      //               }
      //               break;
      //             }
      //           }
      //         }
      //       }
      //     }
      //   }
      //   break;

      case XRDDEFAULT_SERVICESTATUS_DATA:
        if (!svc) {
          if (!strcmp(var, "host_name")) {
            host_name = my_strdup(val);

            /*svc=find_service(host_name,service_description); */

            /* break out */
            break;
          }
          else if (!strcmp(var, "service_description")) {
            service_description = my_strdup(val);
            svc = find_service(host_name, service_description);

            /* break out */
            break;
          }
        }
        else {
          if (!strcmp(var, "modified_attributes")) {
            svc->modified_attributes = strtoul(val, NULL, 10);

            /* mask out attributes we don't want to retain */
            svc->modified_attributes &= ~service_attribute_mask;
          }
          if (svc->retain_status_information) {
            if (!strcmp(var, "has_been_checked"))
              svc->has_been_checked = (atoi(val) > 0);
            else if (!strcmp(var, "check_execution_time"))
              svc->execution_time = strtod(val, NULL);
            else if (!strcmp(var, "check_latency"))
              svc->latency = strtod(val, NULL);
            else if (!strcmp(var, "check_type"))
              svc->check_type = atoi(val);
            else if (!strcmp(var, "current_state"))
              svc->current_state = atoi(val);
            else if (!strcmp(var, "last_state"))
              svc->last_state = atoi(val);
            else if (!strcmp(var, "last_hard_state"))
              svc->last_hard_state = atoi(val);
            else if (!strcmp(var, "current_attempt"))
              svc->current_attempt = atoi(val);
            else if (!strcmp(var, "current_event_id"))
              svc->current_event_id = strtoul(val, NULL, 10);
            else if (!strcmp(var, "last_event_id"))
              svc->last_event_id = strtoul(val, NULL, 10);
            else if (!strcmp(var, "current_problem_id"))
              svc->current_problem_id = strtoul(val, NULL, 10);
            else if (!strcmp(var, "last_problem_id"))
              svc->last_problem_id = strtoul(val, NULL, 10);
            else if (!strcmp(var, "state_type"))
              svc->state_type = atoi(val);
            else if (!strcmp(var, "last_state_change"))
              svc->last_state_change = strtoul(val, NULL, 10);
            else if (!strcmp(var, "last_hard_state_change"))
              svc->last_hard_state_change = strtoul(val, NULL, 10);
            else if (!strcmp(var, "last_time_ok"))
              svc->last_time_ok = strtoul(val, NULL, 10);
            else if (!strcmp(var, "last_time_warning"))
              svc->last_time_warning = strtoul(val, NULL, 10);
            else if (!strcmp(var, "last_time_unknown"))
              svc->last_time_unknown = strtoul(val, NULL, 10);
            else if (!strcmp(var, "last_time_critical"))
              svc->last_time_critical = strtoul(val, NULL, 10);
            else if (!strcmp(var, "plugin_output")) {
              delete[] svc->plugin_output;
              svc->plugin_output = my_strdup(val);
            }
            else if (!strcmp(var, "long_plugin_output")) {
              delete[] svc->long_plugin_output;
              svc->long_plugin_output = my_strdup(val);
            }
            else if (!strcmp(var, "performance_data")) {
              delete[] svc->perf_data;
              svc->perf_data = my_strdup(val);
            }
            else if (!strcmp(var, "last_check"))
              svc->last_check = strtoul(val, NULL, 10);
            else if (!strcmp(var, "next_check")) {
              if (config->use_retained_scheduling_info()
                  && scheduling_info_is_ok)
                svc->next_check = strtoul(val, NULL, 10);
            }
            else if (!strcmp(var, "check_options")) {
              if (config->use_retained_scheduling_info()
                  && scheduling_info_is_ok)
                svc->check_options = atoi(val);
            }
            else if (!strcmp(var, "notified_on_unknown"))
              svc->notified_on_unknown = (atoi(val) > 0);
            else if (!strcmp(var, "notified_on_warning"))
              svc->notified_on_warning = (atoi(val) > 0);
            else if (!strcmp(var, "notified_on_critical"))
              svc->notified_on_critical = (atoi(val) > 0);
            else if (!strcmp(var, "current_notification_number"))
              svc->current_notification_number = atoi(val);
            else if (!strcmp(var, "current_notification_id"))
              svc->current_notification_id = strtoul(val, NULL, 10);
            else if (!strcmp(var, "last_notification"))
              svc->last_notification = strtoul(val, NULL, 10);
            else if (!strcmp(var, "is_flapping"))
              was_flapping = atoi(val);
            else if (!strcmp(var, "percent_state_change"))
              svc->percent_state_change = strtod(val, NULL);
            else
              if (!strcmp(var, "check_flapping_recovery_notification"))
                svc->check_flapping_recovery_notification = atoi(val);
              else if (!strcmp(var, "state_history")) {
                temp_ptr = val;
                for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x) {
                  if ((ch = my_strsep(&temp_ptr, ",")) != NULL)
                    svc->state_history[x] = atoi(ch);
                  else
                    break;
                }
                svc->state_history_index = 0;
              }
              else
                found_directive = false;
          }
          if (svc->retain_nonstatus_information) {
            /* null-op speeds up logic */
            if (found_directive);

            else if (!strcmp(var, "problem_has_been_acknowledged"))
              svc->problem_has_been_acknowledged = (atoi(val) > 0);
            else if (!strcmp(var, "acknowledgement_type"))
              svc->acknowledgement_type = atoi(val);
            else if (!strcmp(var, "notifications_enabled")) {
              if (svc->modified_attributes & MODATTR_NOTIFICATIONS_ENABLED)
                svc->notifications_enabled = (atoi(val) > 0);
            }
            else if (!strcmp(var, "active_checks_enabled")) {
              if (svc->modified_attributes & MODATTR_ACTIVE_CHECKS_ENABLED)
                svc->checks_enabled = (atoi(val) > 0);
            }
            else if (!strcmp(var, "passive_checks_enabled")) {
              if (svc->modified_attributes & MODATTR_PASSIVE_CHECKS_ENABLED)
                svc->accept_passive_service_checks = (atoi(val) > 0);
            }
            else if (!strcmp(var, "event_handler_enabled")) {
              if (svc->modified_attributes & MODATTR_EVENT_HANDLER_ENABLED)
                svc->event_handler_enabled = (atoi(val) > 0);
            }
            else if (!strcmp(var, "flap_detection_enabled")) {
              if (svc->modified_attributes & MODATTR_FLAP_DETECTION_ENABLED)
                svc->flap_detection_enabled = (atoi(val) > 0);
            }
            else if (!strcmp(var, "failure_prediction_enabled")) {
              if (svc->modified_attributes & MODATTR_FAILURE_PREDICTION_ENABLED)
                svc->failure_prediction_enabled = (atoi(val) > 0);
            }
            else if (!strcmp(var, "process_performance_data")) {
              if (svc->modified_attributes & MODATTR_PERFORMANCE_DATA_ENABLED)
                svc->process_performance_data = (atoi(val) > 0);
            }
            else if (!strcmp(var, "obsess_over_service")) {
              if (svc->modified_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED)
                svc->obsess_over_service = (atoi(val) > 0);
            }
            else if (!strcmp(var, "check_command")) {
              if (svc->modified_attributes & MODATTR_CHECK_COMMAND) {
                /* make sure the check command still exists... */
                tempval = my_strdup(val);
                temp_ptr = my_strtok(tempval, "!");
                cmd = find_command(temp_ptr);
                temp_ptr = my_strdup(val);
                delete[] tempval;

                if (cmd && temp_ptr) {
                  delete[] svc->service_check_command;
                  svc->service_check_command = temp_ptr;
                }
                else
                  svc->modified_attributes -= MODATTR_CHECK_COMMAND;
              }
            }
            else if (!strcmp(var, "check_period")) {
              if (svc->modified_attributes & MODATTR_CHECK_TIMEPERIOD) {
                /* make sure the timeperiod still exists... */
                tperiod = find_timeperiod(val);
                temp_ptr = my_strdup(val);

                if (tperiod && temp_ptr) {
                  delete[] svc->check_period;
                  svc->check_period = temp_ptr;
                }
                else
                  svc->modified_attributes -= MODATTR_CHECK_TIMEPERIOD;
              }
            }
            else if (!strcmp(var, "notification_period")) {
              if (svc->modified_attributes & MODATTR_NOTIFICATION_TIMEPERIOD) {
                /* make sure the timeperiod still exists... */
                tperiod = find_timeperiod(val);
                temp_ptr = my_strdup(val);

                if (tperiod && temp_ptr) {
                  delete[] svc->notification_period;
                  svc->notification_period = temp_ptr;
                }
                else
                  svc->modified_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
              }
            }
            else if (!strcmp(var, "event_handler")) {
              if (svc->modified_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
                /* make sure the check command still exists... */
                tempval = my_strdup(val);
                temp_ptr = my_strtok(tempval, "!");
                cmd = find_command(temp_ptr);
                temp_ptr = my_strdup(val);
                delete[] tempval;

                if (cmd && temp_ptr) {
                  delete[] svc->event_handler;
                  svc->event_handler = temp_ptr;
                }
                else
                  svc->modified_attributes -= MODATTR_EVENT_HANDLER_COMMAND;
              }
            }
            else if (!strcmp(var, "normal_check_interval")) {
              if (svc->modified_attributes & MODATTR_NORMAL_CHECK_INTERVAL
                  && strtod(val, NULL) >= 0)
                svc->check_interval = strtod(val, NULL);
            }
            else if (!strcmp(var, "retry_check_interval")) {
              if (svc->modified_attributes & MODATTR_RETRY_CHECK_INTERVAL
                  && strtod(val, NULL) >= 0)
                svc->retry_interval = strtod(val, NULL);
            }
            else if (!strcmp(var, "max_attempts")) {
              if (svc->modified_attributes & MODATTR_MAX_CHECK_ATTEMPTS
                  && atoi(val) >= 1) {
                svc->max_attempts = atoi(val);

                /* adjust current attempt number if in a hard state */
                if (svc->state_type == HARD_STATE
                    && svc->current_state != STATE_OK
                    && svc->current_attempt > 1)
                  svc->current_attempt = svc->max_attempts;
              }
            }
            /* custom variables */
            else if (var[0] == '_') {
              if (svc->modified_attributes & MODATTR_CUSTOM_VARIABLE) {

                /* get the variable name */
                customvarname = var + 1;

                for (member = svc->custom_variables;
                     member;
                     member = member->next) {
                  if (!strcmp(customvarname, member->variable_name)) {
                    if ((x = atoi(val)) > 0 && strlen(val) > 3) {
                      delete[] member->variable_value;
                      member->variable_value = my_strdup(val + 2);
                      member->has_been_modified = (x > 0);
                    }
                    break;
                  }
                }
              }
            }
          }
        }
        break;

      case XRDDEFAULT_CONTACTSTATUS_DATA:
        if (!cntct) {
          if (!strcmp(var, "contact_name")) {
            contact_name = my_strdup(val);
            cntct = find_contact(contact_name);
          }
        }
        else {
          if (!strcmp(var, "modified_attributes")) {
            cntct->modified_attributes = strtoul(val, NULL, 10);

            /* mask out attributes we don't want to retain */
            cntct->modified_attributes &= ~contact_attribute_mask;
          }
          else if (!strcmp(var, "modified_host_attributes")) {
            cntct->modified_host_attributes = strtoul(val, NULL, 10);

            /* mask out attributes we don't want to retain */
            cntct->modified_host_attributes &= ~contact_host_attribute_mask;
          }
          else if (!strcmp(var, "modified_service_attributes")) {
            cntct->modified_service_attributes = strtoul(val, NULL, 10);

            /* mask out attributes we don't want to retain */
            cntct->modified_service_attributes &= ~contact_service_attribute_mask;
          }
          else if (cntct->retain_status_information) {
            if (!strcmp(var, "last_host_notification"))
              cntct->last_host_notification = strtoul(val, NULL, 10);
            else if (!strcmp(var, "last_service_notification"))
              cntct->last_service_notification = strtoul(val, NULL, 10);
            else
              found_directive = false;
          }
          if (cntct->retain_nonstatus_information) {
            /* null-op speeds up logic */
            if (found_directive);

            else if (!strcmp(var, "host_notification_period")) {
              if (cntct->modified_host_attributes & MODATTR_NOTIFICATION_TIMEPERIOD) {
                /* make sure the timeperiod still exists... */
                tperiod = find_timeperiod(val);
                temp_ptr = my_strdup(val);

                if (tperiod && temp_ptr) {
                  delete[] cntct->host_notification_period;
                  cntct->host_notification_period = temp_ptr;
                }
                else
                  cntct->modified_host_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
              }
            }
            else if (!strcmp(var, "service_notification_period")) {
              if (cntct->modified_service_attributes & MODATTR_NOTIFICATION_TIMEPERIOD) {
                /* make sure the timeperiod still exists... */
                tperiod = find_timeperiod(val);
                temp_ptr = my_strdup(val);

                if (tperiod && temp_ptr) {
                  delete[] cntct->service_notification_period;
                  cntct->service_notification_period = temp_ptr;
                }
                else
                  cntct->modified_service_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
              }
            }
            else if (!strcmp(var, "host_notifications_enabled")) {
              if (cntct->modified_host_attributes & MODATTR_NOTIFICATIONS_ENABLED)
                cntct->host_notifications_enabled = (atoi(val) > 0);
            }
            else if (!strcmp(var, "service_notifications_enabled")) {
              if (cntct->modified_service_attributes & MODATTR_NOTIFICATIONS_ENABLED)
                cntct->service_notifications_enabled = (atoi(val) > 0);
            }
            /* custom variables */
            else if (var[0] == '_') {
              if (cntct->modified_attributes & MODATTR_CUSTOM_VARIABLE) {
                /* get the variable name */
                customvarname = var + 1;

                for (member = cntct->custom_variables;
                     member;
                     member = member->next) {
                  if (!strcmp(customvarname, member->variable_name)) {
                    if ((x = atoi(val)) > 0 && strlen(val) > 3) {
                      delete[] member->variable_value;
                      member->variable_value = my_strdup(val + 2);
                      member->has_been_modified = (x > 0);
                    }
                    break;
                  }
                }
              }
            }
          }
        }
        break;

      case XRDDEFAULT_HOSTCOMMENT_DATA:
      case XRDDEFAULT_SERVICECOMMENT_DATA:
        if (!strcmp(var, "host_name"))
          host_name = my_strdup(val);
        else if (!strcmp(var, "service_description"))
          service_description = my_strdup(val);
        else if (!strcmp(var, "entry_type"))
          entry_type = atoi(val);
        else if (!strcmp(var, "comment_id"))
          comment_id = strtoul(val, NULL, 10);
        else if (!strcmp(var, "source"))
          source = atoi(val);
        else if (!strcmp(var, "persistent"))
          persistent = (atoi(val) > 0);
        else if (!strcmp(var, "entry_time"))
          entry_time = strtoul(val, NULL, 10);
        else if (!strcmp(var, "expires"))
          expires = (atoi(val) > 0);
        else if (!strcmp(var, "expire_time"))
          expire_time = strtoul(val, NULL, 10);
        else if (!strcmp(var, "author"))
          author = my_strdup(val);
        else if (!strcmp(var, "comment_data"))
          comment_data = my_strdup(val);
            break;

      case XRDDEFAULT_HOSTDOWNTIME_DATA:
      case XRDDEFAULT_SERVICEDOWNTIME_DATA:
        if (!strcmp(var, "host_name"))
          host_name = my_strdup(val);
        else if (!strcmp(var, "service_description"))
          service_description = my_strdup(val);
        else if (!strcmp(var, "downtime_id"))
          downtime_id = strtoul(val, NULL, 10);
        else if (!strcmp(var, "entry_time"))
          entry_time = strtoul(val, NULL, 10);
        else if (!strcmp(var, "start_time"))
          start_time = strtoul(val, NULL, 10);
        else if (!strcmp(var, "end_time"))
          end_time = strtoul(val, NULL, 10);
        else if (!strcmp(var, "fixed"))
          fixed = (atoi(val) > 0);
        else if (!strcmp(var, "triggered_by"))
          triggered_by = strtoul(val, NULL, 10);
        else if (!strcmp(var, "duration"))
          duration = strtoul(val, NULL, 10);
        else if (!strcmp(var, "author"))
          author = my_strdup(val);
        else if (!strcmp(var, "comment"))
          comment_data = my_strdup(val);
            break;

      default:
        break;
      }
    }
  }

  /* free memory and close the file */
  delete[] inputbuf;
  mmap_fclose(thefile);

  if (sort_downtime() != OK)
    return (ERROR);
  if (sort_comments() != OK)
    return (ERROR);

  if (test_scheduling)
    gettimeofday(&tv[1], NULL);

  if (test_scheduling) {
    double runtime[2];
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
