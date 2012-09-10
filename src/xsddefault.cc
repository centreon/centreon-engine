/*
** Copyright 2000-2009 Ethan Galstad
** Copyright 2009      Nagios Core Development Team and Community Contributors
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

#include <errno.h>
#include <fcntl.h>
#include <QByteArray>
#include <QTextStream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "com/centreon/engine/comments.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/skiplist.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/utils.hh"
#include "com/centreon/engine/xsddefault.hh"

using namespace com::centreon::engine::logging;

static char* xsddefault_status_log = NULL;
static int   xsddefault_status_log_fd = -1;

/******************************************************************/
/***************** COMMON CONFIG INITIALIZATION  ******************/
/******************************************************************/

/* grab configuration information */
int xsddefault_grab_config_info(char* config_file) {
  char* input = NULL;
  mmapfile* thefile;
  nagios_macros* mac;

  /*** CORE PASSES IN MAIN CONFIG FILE, CGIS PASS IN CGI CONFIG FILE! ***/

  /* open the config file for reading */
  if ((thefile = mmap_fopen(config_file)) == NULL)
    return (ERROR);

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

    /* core reads variables directly from the main config file */
    xsddefault_grab_config_directives(input);
  }

  /* free memory and close the file */
  delete[] input;
  mmap_fclose(thefile);

  /* initialize locations if necessary */
  if (xsddefault_status_log == NULL)
    xsddefault_status_log = my_strdup(DEFAULT_STATUS_FILE);

  /* make sure we have what we need */
  if (xsddefault_status_log == NULL)
    return (ERROR);

  mac = get_global_macros();
  /* save the status file macro */
  delete[] mac->x[MACRO_STATUSDATAFILE];
  mac->x[MACRO_STATUSDATAFILE] = my_strdup(xsddefault_status_log);
  strip(mac->x[MACRO_STATUSDATAFILE]);

  return (OK);
}

/* processes a single directive */
int xsddefault_grab_config_directives(char* input) {
  char const* temp_ptr = NULL;
  char* varname = NULL;
  char* varvalue = NULL;

  /* get the variable name */
  if ((temp_ptr = my_strtok(input, "=")) == NULL)
    return (ERROR);
  varname = my_strdup(temp_ptr);

  /* get the variable value */
  if ((temp_ptr = my_strtok(NULL, "\n")) == NULL) {
    delete[] varname;
    return (ERROR);
  }
  varvalue = my_strdup(temp_ptr);

  /* status log definition */
  if (!strcmp(varname, "status_file") || !strcmp(varname, "xsddefault_status_log"))
    xsddefault_status_log = my_strdup(temp_ptr);

  /* free memory */
  delete[] varname;
  delete[] varvalue;

  return (OK);
}

/******************************************************************/
/********************* INIT/CLEANUP FUNCTIONS *********************/
/******************************************************************/

/* initialize status data */
int xsddefault_initialize_status_data(char* config_file) {
  int result;

  /* grab configuration data */
  result = xsddefault_grab_config_info(config_file);
  if (result == ERROR)
    return (ERROR);

  /* delete the old status log (it might not exist) */
  if (xsddefault_status_log)
    unlink(xsddefault_status_log);

  if (xsddefault_status_log_fd == -1) {
    if ((xsddefault_status_log_fd = open(xsddefault_status_log,
                                         O_WRONLY | O_CREAT,
                                         S_IRUSR | S_IWUSR | S_IRGRP)) == -1) {
      logger(log_runtime_error, basic)
        << "Error: Unable to open status data file '"
        << xsddefault_status_log << "': " << strerror(errno);
      return (ERROR);
    }
  }

  return (OK);
}

/* cleanup status data before terminating */
int xsddefault_cleanup_status_data(char* config_file, int delete_status_data) {
  (void)config_file;

  /* delete the status log */
  if (delete_status_data == TRUE && xsddefault_status_log) {
    if (unlink(xsddefault_status_log))
      return (ERROR);
  }

  /* free memory */
  delete[] xsddefault_status_log;
  xsddefault_status_log = NULL;

  if (xsddefault_status_log_fd != -1) {
    close(xsddefault_status_log_fd);
    xsddefault_status_log_fd = -1;
  }

  return (OK);
}

/******************************************************************/
/****************** STATUS DATA OUTPUT FUNCTIONS ******************/
/******************************************************************/

/* write all status data to file */
int xsddefault_save_status_data(void) {
  int used_external_command_buffer_slots = 0;
  int high_external_command_buffer_slots = 0;

  logger(dbg_functions, basic) << "save_status_data()";

  /* get number of items in the command buffer */
  if (config.get_check_external_commands() == true) {
    pthread_mutex_lock(&external_command_buffer.buffer_lock);
    used_external_command_buffer_slots = external_command_buffer.items;
    high_external_command_buffer_slots = external_command_buffer.high;
    pthread_mutex_unlock(&external_command_buffer.buffer_lock);
  }

  /* generate check statistics */
  generate_check_stats();

  QByteArray data;
  QTextStream stream(&data);

  /* write version info to status file */
  stream << "#############################################\n"
         << "#        CENTREON ENGINE STATUS FILE        #\n"
         << "#                                           #\n"
         << "# THIS FILE IS AUTOMATICALLY GENERATED BY   #\n"
         << "# CENTREON ENGINE. DO NOT MODIFY THIS FILE! #\n"
         << "#############################################\n\n";

  time_t current_time;
  time(&current_time);

  /* write file info */
  stream << "info {\n"
         << "\tcreated=" << static_cast<unsigned long>(current_time) << "\n"
         << "\t}\n\n";

  /* save program status data */
  stream << "programstatus {\n"
         << "\tmodified_host_attributes=" << modified_host_process_attributes << "\n"
         << "\tmodified_service_attributes=" << modified_service_process_attributes << "\n"
         << "\tnagios_pid=" << static_cast<unsigned int>(getpid()) << "\n"
         << "\tprogram_start=" << static_cast<unsigned long>(program_start) << "\n"
         << "\tlast_command_check=" << static_cast<unsigned long>(last_command_check) << "\n"
         << "\tlast_log_rotation=" << static_cast<unsigned long>(last_log_rotation) << "\n"
         << "\tenable_notifications=" << config.get_enable_notifications() << "\n"
         << "\tactive_service_checks_enabled=" << config.get_execute_service_checks() << "\n"
         << "\tpassive_service_checks_enabled=" << config.get_accept_passive_service_checks() << "\n"
         << "\tactive_host_checks_enabled=" << config.get_execute_host_checks() << "\n"
         << "\tpassive_host_checks_enabled=" << config.get_accept_passive_host_checks() << "\n"
         << "\tenable_event_handlers=" << config.get_enable_event_handlers() << "\n"
         << "\tobsess_over_services=" << config.get_obsess_over_services() << "\n"
         << "\tobsess_over_hosts=" << config.get_obsess_over_hosts() << "\n"
         << "\tcheck_service_freshness=" << config.get_check_service_freshness() << "\n"
         << "\tcheck_host_freshness=" << config.get_check_host_freshness() << "\n"
         << "\tenable_flap_detection=" << config.get_enable_flap_detection() << "\n"
         << "\tenable_failure_prediction=" << config.get_enable_failure_prediction() << "\n"
         << "\tprocess_performance_data=" << config.get_process_performance_data() << "\n"
         << "\tglobal_host_event_handler=" << qPrintable(config.get_global_host_event_handler()) << "\n"
         << "\tglobal_service_event_handler=" << qPrintable(config.get_global_service_event_handler()) << "\n"
         << "\tnext_comment_id=" << next_comment_id << "\n"
         << "\tnext_downtime_id=" << next_downtime_id << "\n"
         << "\tnext_event_id=" << next_event_id << "\n"
         << "\tnext_problem_id=" << next_problem_id << "\n"
         << "\tnext_notification_id=" << next_notification_id << "\n"
         << "\ttotal_external_command_buffer_slots=" << config.get_external_command_buffer_slots() << "\n"
         << "\tused_external_command_buffer_slots=" << used_external_command_buffer_slots << "\n"
         << "\thigh_external_command_buffer_slots=" << high_external_command_buffer_slots << "\n"
         << "\tactive_scheduled_host_check_stats="
         << check_statistics[ACTIVE_SCHEDULED_HOST_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[ACTIVE_SCHEDULED_HOST_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[ACTIVE_SCHEDULED_HOST_CHECK_STATS].minute_stats[2] << "\n"
         << "\tactive_ondemand_host_check_stats="
         << check_statistics[ACTIVE_ONDEMAND_HOST_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[ACTIVE_ONDEMAND_HOST_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[ACTIVE_ONDEMAND_HOST_CHECK_STATS].minute_stats[2] << "\n"
         << "\tpassive_host_check_stats="
         << check_statistics[PASSIVE_HOST_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[PASSIVE_HOST_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[PASSIVE_HOST_CHECK_STATS].minute_stats[2] << "\n"
         << "\tactive_scheduled_service_check_stats="
         << check_statistics[ACTIVE_SCHEDULED_SERVICE_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[ACTIVE_SCHEDULED_SERVICE_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[ACTIVE_SCHEDULED_SERVICE_CHECK_STATS].minute_stats[2] << "\n"
         << "\tactive_ondemand_service_check_stats="
         << check_statistics[ACTIVE_ONDEMAND_SERVICE_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[ACTIVE_ONDEMAND_SERVICE_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[ACTIVE_ONDEMAND_SERVICE_CHECK_STATS].minute_stats[2] << "\n"
         << "\tpassive_service_check_stats="
         << check_statistics[PASSIVE_SERVICE_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[PASSIVE_SERVICE_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[PASSIVE_SERVICE_CHECK_STATS].minute_stats[2] << "\n"
         << "\tcached_host_check_stats="
         << check_statistics[ACTIVE_CACHED_HOST_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[ACTIVE_CACHED_HOST_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[ACTIVE_CACHED_HOST_CHECK_STATS].minute_stats[2] << "\n"
         << "\tcached_service_check_stats="
         << check_statistics[ACTIVE_CACHED_SERVICE_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[ACTIVE_CACHED_SERVICE_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[ACTIVE_CACHED_SERVICE_CHECK_STATS].minute_stats[2] << "\n"
         << "\texternal_command_stats="
         << check_statistics[EXTERNAL_COMMAND_STATS].minute_stats[0] << ","
         << check_statistics[EXTERNAL_COMMAND_STATS].minute_stats[1] << ","
         << check_statistics[EXTERNAL_COMMAND_STATS].minute_stats[2] << "\n"
         << "\tparallel_host_check_stats="
         << check_statistics[PARALLEL_HOST_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[PARALLEL_HOST_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[PARALLEL_HOST_CHECK_STATS].minute_stats[2] << "\n"
         << "\tserial_host_check_stats="
         << check_statistics[SERIAL_HOST_CHECK_STATS].minute_stats[0] << ","
         << check_statistics[SERIAL_HOST_CHECK_STATS].minute_stats[1] << ","
         << check_statistics[SERIAL_HOST_CHECK_STATS].minute_stats[2] << "\n"
         << "\t}\n\n";

  /* save host status data */
  for (host* temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    stream << "hoststatus {\n"
           << "\thost_name=" << temp_host->name << "\n"
           << "\tmodified_attributes=" << temp_host->modified_attributes << "\n"
           << "\tcheck_command=" << (temp_host->host_check_command ? temp_host->host_check_command : "") << "\n"
           << "\tcheck_period=" << (temp_host->check_period ? temp_host->check_period : "") << "\n"
           << "\tnotification_period=" << (temp_host->notification_period ? temp_host->notification_period : "") << "\n"
           << "\tcheck_interval=" << temp_host->check_interval << "\n"
           << "\tretry_interval=" << temp_host->retry_interval << "\n"
           << "\tevent_handler=" << (temp_host->event_handler ? temp_host->event_handler : "") << "\n"
           << "\thas_been_checked=" << temp_host->has_been_checked << "\n"
           << "\tshould_be_scheduled=" << temp_host->should_be_scheduled << "\n"
           << "\tcheck_execution_time=" << qSetRealNumberPrecision(3) << ::fixed << temp_host->execution_time << reset << "\n"
           << "\tcheck_latency=" << qSetRealNumberPrecision(3) << ::fixed << temp_host->latency << reset << "\n"
           << "\tcheck_type=" << temp_host->check_type << "\n"
           << "\tcurrent_state=" << temp_host->current_state << "\n"
           << "\tlast_hard_state=" << temp_host->last_hard_state << "\n"
           << "\tlast_event_id=" << temp_host->last_event_id << "\n"
           << "\tcurrent_event_id=" << temp_host->current_event_id << "\n"
           << "\tcurrent_problem_id=" << temp_host->current_problem_id << "\n"
           << "\tlast_problem_id=" << temp_host->last_problem_id << "\n"
           << "\tplugin_output=" << (temp_host->plugin_output ? temp_host->plugin_output : "") << "\n"
           << "\tlong_plugin_output=" << (temp_host->long_plugin_output ? temp_host->long_plugin_output : "") << "\n"
           << "\tperformance_data=" << (temp_host->perf_data ? temp_host->perf_data : "") << "\n"
           << "\tlast_check=" << static_cast<unsigned long>(temp_host->last_check) << "\n"
           << "\tnext_check=" << static_cast<unsigned long>(temp_host->next_check) << "\n"
           << "\tcheck_options=" << temp_host->check_options << "\n"
           << "\tcurrent_attempt=" << temp_host->current_attempt << "\n"
           << "\tmax_attempts=" << temp_host->max_attempts << "\n"
           << "\tstate_type=" << temp_host->state_type << "\n"
           << "\tlast_state_change=" << static_cast<unsigned long>(temp_host->last_state_change) << "\n"
           << "\tlast_hard_state_change=" << static_cast<unsigned long>(temp_host->last_hard_state_change) << "\n"
           << "\tlast_time_up=" << static_cast<unsigned long>(temp_host->last_time_up) << "\n"
           << "\tlast_time_down=" << static_cast<unsigned long>(temp_host->last_time_down) << "\n"
           << "\tlast_time_unreachable=" << static_cast<unsigned long>(temp_host->last_time_unreachable) << "\n"
           << "\tlast_notification=" << static_cast<unsigned long>(temp_host->last_host_notification) << "\n"
           << "\tnext_notification=" << static_cast<unsigned long>(temp_host->next_host_notification) << "\n"
           << "\tno_more_notifications=" << temp_host->no_more_notifications << "\n"
           << "\tcurrent_notification_number=" << temp_host->current_notification_number << "\n"
           << "\tcurrent_notification_id=" << temp_host->current_notification_id << "\n"
           << "\tnotifications_enabled=" << temp_host->notifications_enabled << "\n"
           << "\tproblem_has_been_acknowledged=" << temp_host->problem_has_been_acknowledged << "\n"
           << "\tacknowledgement_type=" << temp_host->acknowledgement_type << "\n"
           << "\tactive_checks_enabled=" << temp_host->checks_enabled << "\n"
           << "\tpassive_checks_enabled=" << temp_host->accept_passive_host_checks << "\n"
           << "\tevent_handler_enabled=" << temp_host->event_handler_enabled << "\n"
           << "\tflap_detection_enabled=" << temp_host->flap_detection_enabled << "\n"
           << "\tfailure_prediction_enabled=" << temp_host->failure_prediction_enabled << "\n"
           << "\tprocess_performance_data=" << temp_host->process_performance_data << "\n"
           << "\tobsess_over_host=" << temp_host->obsess_over_host << "\n"
           << "\tlast_update=" << static_cast<unsigned long>(current_time) << "\n"
           << "\tis_flapping=" << temp_host->is_flapping << "\n"
           << "\tpercent_state_change=" << qSetRealNumberPrecision(2) << ::fixed << temp_host->percent_state_change << reset << "\n"
           << "\tscheduled_downtime_depth=" << temp_host->scheduled_downtime_depth << "\n";
    /*
      fprintf(fp,"\tstate_history=");
      for(x=0;x<MAX_STATE_HISTORY_ENTRIES;x++)
      fprintf(fp,"%s%d",(x>0)?",":"",temp_host->state_history[(x+temp_host->state_history_index)%MAX_STATE_HISTORY_ENTRIES]);
      fprintf(fp,"\n");
    */
    /* custom variables */
    for (customvariablesmember* temp_customvariablesmember = temp_host->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      if (temp_customvariablesmember->variable_name)
        stream << "\t_" << temp_customvariablesmember->variable_name << "="
               << temp_customvariablesmember->has_been_modified << ";"
               << (temp_customvariablesmember->variable_value
                   ? temp_customvariablesmember->variable_value : "") << "\n";
    }
    stream << "\t}\n\n";
  }

  /* save service status data */
  for (service* temp_service = service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {
    stream << "servicestatus {\n"
           << "\thost_name=" << temp_service->host_name << "\n"
           << "\tservice_description=" << temp_service->description << "\n"
           << "\tmodified_attributes=" << temp_service->modified_attributes << "\n"
           << "\tcheck_command=" << (temp_service->service_check_command ? temp_service->service_check_command : "") << "\n"
           << "\tcheck_period=" << (temp_service->check_period ? temp_service->check_period : "") << "\n"
           << "\tnotification_period=" << (temp_service->notification_period ? temp_service->notification_period : "") << "\n"
           << "\tcheck_interval=" << temp_service->check_interval << "\n"
           << "\tretry_interval=" << temp_service->retry_interval << "\n"
           << "\tevent_handler=" << (temp_service->event_handler ? temp_service->event_handler : "") << "\n"
           << "\thas_been_checked=" << temp_service->has_been_checked << "\n"
           << "\tshould_be_scheduled=" << temp_service->should_be_scheduled << "\n"
           << "\tcheck_execution_time=" << qSetRealNumberPrecision(3) << ::fixed << temp_service->execution_time << reset << "\n"
           << "\tcheck_latency=" << qSetRealNumberPrecision(3) << ::fixed << temp_service->latency << reset << "\n"
           << "\tcheck_type=" << temp_service->check_type << "\n"
           << "\tcurrent_state=" << temp_service->current_state << "\n"
           << "\tlast_hard_state=" << temp_service->last_hard_state << "\n"
           << "\tlast_event_id=" << temp_service->last_event_id << "\n"
           << "\tcurrent_event_id=" << temp_service->current_event_id << "\n"
           << "\tcurrent_problem_id=" << temp_service->current_problem_id << "\n"
           << "\tlast_problem_id=" << temp_service->last_problem_id << "\n"
           << "\tcurrent_attempt=" << temp_service->current_attempt << "\n"
           << "\tmax_attempts=" << temp_service->max_attempts << "\n"
           << "\tstate_type=" << temp_service->state_type << "\n"
           << "\tlast_state_change=" << static_cast<unsigned long>(temp_service->last_state_change) << "\n"
           << "\tlast_hard_state_change=" << static_cast<unsigned long>(temp_service->last_hard_state_change) << "\n"
           << "\tlast_time_ok=" << static_cast<unsigned long>(temp_service->last_time_ok) << "\n"
           << "\tlast_time_warning=" << static_cast<unsigned long>(temp_service->last_time_warning) << "\n"
           << "\tlast_time_unknown=" << static_cast<unsigned long>(temp_service->last_time_unknown) << "\n"
           << "\tlast_time_critical=" << static_cast<unsigned long>(temp_service->last_time_critical) << "\n"
           << "\tplugin_output=" << (temp_service->plugin_output ? temp_service->plugin_output : "") << "\n"
           << "\tlong_plugin_output=" << (temp_service->long_plugin_output ? temp_service->long_plugin_output : "") << "\n"
           << "\tperformance_data=" << (temp_service->perf_data ? temp_service->perf_data : "") << "\n"
           << "\tlast_check=" << static_cast<unsigned long>(temp_service->last_check) << "\n"
           << "\tnext_check=" << static_cast<unsigned long>(temp_service->next_check) << "\n"
           << "\tcheck_options=" << temp_service->check_options << "\n"
           << "\tcurrent_notification_number=" << temp_service->current_notification_number << "\n"
           << "\tcurrent_notification_id=" << temp_service->current_notification_id << "\n"
           << "\tlast_notification=" << static_cast<unsigned long>(temp_service->last_notification) << "\n"
           << "\tnext_notification=" << static_cast<unsigned long>(temp_service->next_notification) << "\n"
           << "\tno_more_notifications=" << temp_service->no_more_notifications << "\n"
           << "\tnotifications_enabled=" << temp_service->notifications_enabled << "\n"
           << "\tactive_checks_enabled=" << temp_service->checks_enabled << "\n"
           << "\tpassive_checks_enabled=" << temp_service->accept_passive_service_checks << "\n"
           << "\tevent_handler_enabled=" << temp_service->event_handler_enabled << "\n"
           << "\tproblem_has_been_acknowledged=" << temp_service->problem_has_been_acknowledged << "\n"
           << "\tacknowledgement_type=" << temp_service->acknowledgement_type << "\n"
           << "\tflap_detection_enabled=" << temp_service->flap_detection_enabled << "\n"
           << "\tfailure_prediction_enabled=" << temp_service->failure_prediction_enabled << "\n"
           << "\tprocess_performance_data=" << temp_service->process_performance_data << "\n"
           << "\tobsess_over_service=" << temp_service->obsess_over_service << "\n"
           << "\tlast_update=" << static_cast<unsigned long>(current_time) << "\n"
           << "\tis_flapping=" << temp_service->is_flapping << "\n"
           << "\tpercent_state_change=" << qSetRealNumberPrecision(2) << ::fixed << temp_service->percent_state_change << reset << "\n"
           << "\tscheduled_downtime_depth=" << temp_service->scheduled_downtime_depth << "\n";
    /*
      fprintf(fp,"\tstate_history=");
      for(x=0;x<MAX_STATE_HISTORY_ENTRIES;x++)
      fprintf(fp,"%s%d",(x>0)?",":"",temp_service->state_history[(x+temp_service->state_history_index)%MAX_STATE_HISTORY_ENTRIES]);
      fprintf(fp,"\n");
    */
    /* custom variables */
    for (customvariablesmember* temp_customvariablesmember = temp_service->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      if (temp_customvariablesmember->variable_name)
        stream << "\t_" << temp_customvariablesmember->variable_name << "="
               << temp_customvariablesmember->has_been_modified << ";"
               << (temp_customvariablesmember->variable_value
                   ? temp_customvariablesmember->variable_value : "") << "\n";
    }
    stream << "\t}\n\n";
  }

  /* save contact status data */
  for (contact* temp_contact = contact_list;
       temp_contact != NULL;
       temp_contact = temp_contact->next) {
    stream << "contactstatus {\n"
           << "\tcontact_name=" << temp_contact->name << "\n"
           << "\tmodified_attributes=" << temp_contact->modified_attributes << "\n"
           << "\tmodified_host_attributes=" << temp_contact->modified_host_attributes << "\n"
           << "\tmodified_service_attributes=" << temp_contact->modified_service_attributes << "\n"
           << "\thost_notification_period=" << (temp_contact->host_notification_period ? temp_contact->host_notification_period : "") << "\n"
           << "\tservice_notification_period=" << (temp_contact->service_notification_period ? temp_contact->service_notification_period : "") << "\n"
           << "\tlast_host_notification=" << static_cast<unsigned long>(temp_contact->last_host_notification) << "\n"
           << "\tlast_service_notification=" << static_cast<unsigned long>(temp_contact->last_service_notification) << "\n"
           << "\thost_notifications_enabled=" << temp_contact->host_notifications_enabled << "\n"
           << "\tservice_notifications_enabled=" << temp_contact->service_notifications_enabled << "\n";
    /* custom variables */
    for (customvariablesmember* temp_customvariablesmember = temp_contact->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      if (temp_customvariablesmember->variable_name)
        stream << "\t_" << temp_customvariablesmember->variable_name << "="
               << temp_customvariablesmember->has_been_modified << ";"
               << (temp_customvariablesmember->variable_value
                   ? temp_customvariablesmember->variable_value : "") << "\n";
    }
    stream << "\t}\n\n";
  }

  /* save all comments */
  for (comment* temp_comment = comment_list;
       temp_comment != NULL;
       temp_comment = temp_comment->next) {
    if (temp_comment->comment_type == HOST_COMMENT)
      stream << "hostcomment {\n";
    else
      stream << "servicecomment {\n";
    stream << "\thost_name=" << temp_comment->host_name << "\n";
    if (temp_comment->comment_type == SERVICE_COMMENT)
      stream << "\tservice_description=" << temp_comment->service_description << "\n";
    stream << "\tentry_type=" << temp_comment->entry_type << "\n"
           << "\tcomment_id=" << temp_comment->comment_id << "\n"
           << "\tsource=" << temp_comment->source << "\n"
           << "\tpersistent=" << temp_comment->persistent << "\n"
           << "\tentry_time=" << static_cast<unsigned long>(temp_comment->entry_time) << "\n"
           << "\texpires=" << temp_comment->expires << "\n"
           << "\texpire_time=" << static_cast<unsigned long>(temp_comment->expire_time) << "\n"
           << "\tauthor=" << temp_comment->author << "\n"
           << "\tcomment_data=" << temp_comment->comment_data << "\n"
           << "\t}\n\n";
  }

  /* save all downtime */
  for (scheduled_downtime* temp_downtime = scheduled_downtime_list;
       temp_downtime != NULL;
       temp_downtime = temp_downtime->next) {
    if (temp_downtime->type == HOST_DOWNTIME)
      stream << "hostdowntime {\n";
    else
      stream << "servicedowntime {\n";
    stream << "\thost_name=" << temp_downtime->host_name << "\n";
    if (temp_downtime->type == SERVICE_DOWNTIME)
      stream << "\tservice_description=" << temp_downtime->service_description << "\n";
    stream << "\tdowntime_id=" << temp_downtime->downtime_id << "\n"
           << "\tentry_time=" << static_cast<unsigned long>(temp_downtime->entry_time) << "\n"
           << "\tstart_time=" << static_cast<unsigned long>(temp_downtime->start_time) << "\n"
           << "\tend_time=" << static_cast<unsigned long>(temp_downtime->end_time) << "\n"
           << "\ttriggered_by=" << temp_downtime->triggered_by << "\n"
           << "\tfixed=" << temp_downtime->fixed << "\n"
           << "\tduration=" << temp_downtime->duration << "\n"
           << "\tauthor=" << temp_downtime->author << "\n"
           << "\tcomment=" << temp_downtime->comment << "\n"
           << "\t}\n\n";
  }

  // Write data in buffer.
  stream.flush();

  // Prepare status file for overwrite.
  if ((ftruncate(xsddefault_status_log_fd, 0) == -1)
      || (fsync(xsddefault_status_log_fd) == -1)
      || (lseek(xsddefault_status_log_fd, 0, SEEK_SET) == (off_t)-1)) {
    char const* msg(strerror(errno));
    logger(log_runtime_error, basic)
      << "Error: Unable to update status data file '"
      << xsddefault_status_log << "': " << msg;
    return (ERROR);
  }

  // Write status file.
  char const* data_ptr(data.constData());
  unsigned int size(data.size());
  while (size > 0) {
    ssize_t wb(write(xsddefault_status_log_fd, data_ptr, size));
    if (wb <= 0) {
      char const* msg(strerror(errno));
      logger(log_runtime_error, basic)
        << "Error: Unable to update status data file '"
        << xsddefault_status_log << "': " << msg;
      return (ERROR);
    }
    data_ptr += wb;
    size -= wb;
  }

  return (OK);
}
