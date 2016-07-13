/*
** Copyright 1999-2010 Ethan Galstad
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

#include <sstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/objects/comment.hh"
#include "com/centreon/engine/objects/downtime.hh"
#include "com/centreon/engine/perfdata.hh"
#include "com/centreon/engine/sehandlers.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine::logging;

/******************************************************************/
/************* OBSESSIVE COMPULSIVE HANDLER FUNCTIONS *************/
/******************************************************************/

/* handles service check results in an obsessive compulsive manner... */
int obsessive_compulsive_service_check_processor(service* svc) {
  char* raw_command = NULL;
  char* processed_command = NULL;
  host* temp_host = NULL;
  int early_timeout = false;
  double exectime = 0.0;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  nagios_macros mac;

  logger(dbg_functions, basic)
    << "obsessive_compulsive_service_check_processor()";

  if (svc == NULL)
    return (ERROR);

  /* bail out if we shouldn't be obsessing */
  if (config->obsess_over_services() == false)
    return (OK);
  if (svc->obsess_over_service == false)
    return (OK);

  /* if there is no valid command, exit */
  if (config->ocsp_command().empty())
    return (ERROR);

  /* find the associated host */
  if ((temp_host = (host*) svc->host_ptr) == NULL)
    return (ERROR);

  /* update service macros */
  memset(&mac, 0, sizeof(mac));
  grab_host_macros_r(&mac, temp_host);
  grab_service_macros_r(&mac, svc);

  /* get the raw command line */
  get_raw_command_line_r(
    &mac,
    ocsp_command_ptr,
    config->ocsp_command().c_str(),
    &raw_command, macro_options);
  if (raw_command == NULL) {
    clear_volatile_macros_r(&mac);
    return (ERROR);
  }

  logger(dbg_checks, most)
    << "Raw obsessive compulsive service processor "
    "command line: " << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(
    &mac,
    raw_command,
    &processed_command,
    macro_options);
  if (processed_command == NULL) {
    clear_volatile_macros_r(&mac);
    return (ERROR);
  }

  logger(dbg_checks, most)
    << "Processed obsessive compulsive service "
    "processor command line: " << processed_command;

  /* run the command */
  try {
      my_system_r(
      &mac,
      processed_command,
      config->ocsp_timeout(),
      &early_timeout,
      &exectime,
      NULL,
      0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute compulsive service processor command line '"
      << processed_command << "' : " << e.what();
  }

  clear_volatile_macros_r(&mac);

  /* check to see if the command timed out */
  if (early_timeout == true)
    logger(log_runtime_warning, basic)
      << "Warning: OCSP command '" << processed_command
      << "' for service '" << svc->description << "' on host '"
      << svc->host_name << "' timed out after "
      << config->ocsp_timeout() << " seconds";

  /* free memory */
  delete[] raw_command;
  delete[] processed_command;

  return (OK);
}

/* handles host check results in an obsessive compulsive manner... */
int obsessive_compulsive_host_check_processor(host* hst) {
  char* raw_command = NULL;
  char* processed_command = NULL;
  int early_timeout = false;
  double exectime = 0.0;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  nagios_macros mac;

  logger(dbg_functions, basic)
    << "obsessive_compulsive_host_check_processor()";

  if (hst == NULL)
    return (ERROR);

  /* bail out if we shouldn't be obsessing */
  if (config->obsess_over_hosts() == false)
    return (OK);
  if (hst->obsess_over_host == false)
    return (OK);

  /* if there is no valid command, exit */
  if (config->ochp_command().empty())
    return (ERROR);

  /* update macros */
  memset(&mac, 0, sizeof(mac));
  grab_host_macros_r(&mac, hst);

  /* get the raw command line */
  get_raw_command_line_r(
    &mac,
    ochp_command_ptr,
    config->ochp_command().c_str(),
    &raw_command, macro_options);
  if (raw_command == NULL) {
    clear_volatile_macros_r(&mac);
    return (ERROR);
  }

  logger(dbg_checks, most)
    << "Raw obsessive compulsive host processor command line: "
    << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(
    &mac,
    raw_command,
    &processed_command,
    macro_options);
  if (processed_command == NULL) {
    clear_volatile_macros_r(&mac);
    return (ERROR);
  }

  logger(dbg_checks, most)
    << "Processed obsessive compulsive host processor "
    "command line: " << processed_command;

  /* run the command */
  try {
    my_system_r(
      &mac,
      processed_command,
      config->ochp_timeout(),
      &early_timeout,
      &exectime,
      NULL,
      0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute compulsive host processor command line '"
      << processed_command << "' : " << e.what();
  }
  clear_volatile_macros_r(&mac);

  /* check to see if the command timed out */
  if (early_timeout == true)
    logger(log_runtime_warning, basic)
      << "Warning: OCHP command '" << processed_command
      << "' for host '" << hst->name << "' timed out after "
      << config->ochp_timeout() << " seconds";

  /* free memory */
  delete[] raw_command;
  delete[] processed_command;

  return (OK);
}

/******************************************************************/
/**************** SERVICE EVENT HANDLER FUNCTIONS *****************/
/******************************************************************/

/* handles changes in the state of a service */
int handle_service_event(service* svc) {
  host* temp_host = NULL;
  nagios_macros mac;

  logger(dbg_functions, basic)
    << "handle_service_event()";

  if (svc == NULL)
    return (ERROR);

  /* send event data to broker */
  broker_statechange_data(
    NEBTYPE_STATECHANGE_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_STATECHANGE,
    (void*)svc,
    svc->current_state,
    svc->state_type,
    svc->current_attempt,
    svc->max_attempts,
    NULL);

  /* bail out if we shouldn't be running event handlers */
  if (config->enable_event_handlers() == false)
    return (OK);
  if (svc->event_handler_enabled == false)
    return (OK);

  /* find the host */
  if ((temp_host = (host*)svc->host_ptr) == NULL)
    return (ERROR);

  /* update service macros */
  memset(&mac, 0, sizeof(mac));
  grab_host_macros_r(&mac, temp_host);
  grab_service_macros_r(&mac, svc);

  /* run the global service event handler */
  run_global_service_event_handler(&mac, svc);

  /* run the event handler command if there is one */
  if (svc->event_handler != NULL)
    run_service_event_handler(&mac, svc);
  clear_volatile_macros_r(&mac);

  /* send data to event broker */
  broker_external_command(
    NEBTYPE_EXTERNALCOMMAND_CHECK,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    time(NULL),
    NULL,
    NULL,
    NULL);

  return (OK);
}

/* runs the global service event handler */
int run_global_service_event_handler(nagios_macros* mac, service* svc) {
  char* raw_command = NULL;
  char* processed_command = NULL;
  char* processed_logentry = NULL;
  char* command_output = NULL;
  int early_timeout = false;
  double exectime = 0.0;
  int result = 0;
  struct timeval start_time;
  struct timeval end_time;
  int neb_result = OK;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;

  logger(dbg_functions, basic)
    << "run_global_service_event_handler()";

  if (svc == NULL)
    return (ERROR);

  /* bail out if we shouldn't be running event handlers */
  if (config->enable_event_handlers() == false)
    return (OK);

  /* a global service event handler command has not been defined */
  if (config->global_service_event_handler().empty())
    return (ERROR);

  logger(dbg_eventhandlers, more)
    << "Running global event handler for service '"
    << svc->description << "' on host '" << svc->host_name << "'...";

  /* get start time */
  gettimeofday(&start_time, NULL);

  /* get the raw command line */
  get_raw_command_line_r(
    mac,
    global_service_event_handler_ptr,
    config->global_service_event_handler().c_str(),
    &raw_command,
    macro_options);
  if (raw_command == NULL) {
    return (ERROR);
  }

  logger(dbg_eventhandlers, most)
    << "Raw global service event handler command line: " << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(mac, raw_command, &processed_command, macro_options);
  if (processed_command == NULL)
    return (ERROR);

  logger(dbg_eventhandlers, most)
    << "Processed global service event handler "
    "command line: " << processed_command;

  if (config->log_event_handlers() == true) {
    std::ostringstream oss;
    oss << "GLOBAL SERVICE EVENT HANDLER: " << svc->host_name << ';'
	<< svc->description
        << ";$SERVICESTATE$;$SERVICESTATETYPE$;$SERVICEATTEMPT$;"
        << config->global_service_event_handler();
    process_macros_r(
      mac,
      oss.str().c_str(),
      &processed_logentry,
      macro_options);
    logger(log_event_handler, basic)
      << processed_logentry;
  }

  /* send event data to broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_event_handler(
    NEBTYPE_EVENTHANDLER_START,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    GLOBAL_SERVICE_EVENTHANDLER,
    (void*)svc,
    svc->current_state,
    svc->state_type,
    start_time,
    end_time,
    exectime,
    config->event_handler_timeout(),
    early_timeout,
    result,
    config->global_service_event_handler().c_str(),
    processed_command,
    NULL,
    NULL);

  /* neb module wants to override (or cancel) the event handler - perhaps it will run the eventhandler itself */
  if ((neb_result == NEBERROR_CALLBACKCANCEL)
      || (neb_result == NEBERROR_CALLBACKOVERRIDE)) {
    delete[] processed_command;
    delete[] raw_command;
    delete[] processed_logentry;
    return ((neb_result == NEBERROR_CALLBACKCANCEL) ? ERROR : OK);
  }

  /* run the command */
  try {
    result = my_system_r(
               mac,
               processed_command,
               config->event_handler_timeout(),
               &early_timeout,
               &exectime,
               &command_output,
               0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute global service event handler "
         "command line '"
      << processed_command << "' : " << e.what();
  }

  /* check to see if the event handler timed out */
  if (early_timeout == true)
    logger(log_event_handler | log_runtime_warning, basic)
      << "Warning: Global service event handler command '"
      << processed_command << "' timed out after "
      << config->event_handler_timeout() << " seconds";

  /* get end time */
  gettimeofday(&end_time, NULL);

  /* send event data to broker */
  broker_event_handler(
    NEBTYPE_EVENTHANDLER_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    GLOBAL_SERVICE_EVENTHANDLER,
    (void*)svc,
    svc->current_state,
    svc->state_type,
    start_time,
    end_time,
    exectime,
    config->event_handler_timeout(),
    early_timeout,
    result,
    config->global_service_event_handler().c_str(),
    processed_command,
    command_output,
    NULL);

  /* free memory */
  delete[] command_output;
  delete[] raw_command;
  delete[] processed_command;
  delete[] processed_logentry;

  return (OK);
}

/* runs a service event handler command */
int run_service_event_handler(nagios_macros* mac, service* svc) {
  char* raw_command = NULL;
  char* processed_command = NULL;
  char* processed_logentry = NULL;
  char* command_output = NULL;
  int early_timeout = false;
  double exectime = 0.0;
  int result = 0;
  struct timeval start_time;
  struct timeval end_time;
  int neb_result = OK;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;


  logger(dbg_functions, basic)
    << "run_service_event_handler()";

  if (svc == NULL)
    return (ERROR);

  /* bail if there's no command */
  if (svc->event_handler == NULL)
    return (ERROR);

  logger(dbg_eventhandlers, more)
    << "Running event handler for service '" << svc->description
    << "' on host '" << svc->host_name <<"'...";

  /* get start time */
  gettimeofday(&start_time, NULL);

  /* get the raw command line */
  get_raw_command_line_r(
    mac,
    svc->event_handler_ptr,
    svc->event_handler,
    &raw_command,
    macro_options);
  if (raw_command == NULL)
    return (ERROR);

  logger(dbg_eventhandlers, most)
    << "Raw service event handler command line: " << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(
    mac,
    raw_command,
    &processed_command,
    macro_options);
  if (processed_command == NULL)
    return (ERROR);

  logger(dbg_eventhandlers, most)
    << "Processed service event handler command line: "
    << processed_command;

  if (config->log_event_handlers() == true) {
    std::ostringstream oss;
    oss << "SERVICE EVENT HANDLER: " << svc->host_name << ';'
	<< svc->description
	<< ";$SERVICESTATE$;$SERVICESTATETYPE$;$SERVICEATTEMPT$;"
	<< svc->event_handler;
    process_macros_r(
      mac,
      oss.str().c_str(),
      &processed_logentry,
      macro_options);
    logger(log_event_handler, basic)
      << processed_logentry;
  }

  /* send event data to broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_event_handler(
                 NEBTYPE_EVENTHANDLER_START,
                 NEBFLAG_NONE,
                 NEBATTR_NONE,
                 SERVICE_EVENTHANDLER,
                 (void*)svc,
                 svc->current_state,
                 svc->state_type,
                 start_time,
                 end_time,
                 exectime,
                 config->event_handler_timeout(),
                 early_timeout,
                 result,
                 svc->event_handler,
                 processed_command,
                 NULL,
                 NULL);

  /* neb module wants to override (or cancel) the event handler - perhaps it will run the eventhandler itself */
  if ((neb_result == NEBERROR_CALLBACKCANCEL)
      || (neb_result == NEBERROR_CALLBACKOVERRIDE)) {
    delete[] processed_command;
    delete[] raw_command;
    delete[] processed_logentry;
    return ((neb_result == NEBERROR_CALLBACKCANCEL) ? ERROR : OK);
  }

  /* run the command */
  try {
    result = my_system_r(
               mac,
               processed_command,
               config->event_handler_timeout(),
               &early_timeout,
               &exectime,
               &command_output,
               0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute service event handler command line '"
      << processed_command << "' : " << e.what();
  }

  /* check to see if the event handler timed out */
  if (early_timeout == true)
    logger(log_event_handler | log_runtime_warning, basic)
      << "Warning: Service event handler command '" << processed_command
      << "' timed out after " << config->event_handler_timeout()
      << " seconds";

  /* get end time */
  gettimeofday(&end_time, NULL);

  /* send event data to broker */
  broker_event_handler(
    NEBTYPE_EVENTHANDLER_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_EVENTHANDLER,
    (void*)svc,
    svc->current_state,
    svc->state_type,
    start_time,
    end_time,
    exectime,
    config->event_handler_timeout(),
    early_timeout,
    result,
    svc->event_handler,
    processed_command,
    command_output,
    NULL);

  /* free memory */
  delete[] command_output;
  delete[] raw_command;
  delete[] processed_command;
  delete[] processed_logentry;

  return (OK);
}

/******************************************************************/
/****************** HOST EVENT HANDLER FUNCTIONS ******************/
/******************************************************************/

/* handles a change in the status of a host */
int handle_host_event(host* hst) {
  nagios_macros mac;

  logger(dbg_functions, basic)
    << "handle_host_event()";

  if (hst == NULL)
    return (ERROR);

  /* send event data to broker */
  broker_statechange_data(
    NEBTYPE_STATECHANGE_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_STATECHANGE,
    (void*)hst,
    hst->current_state,
    hst->state_type,
    hst->current_attempt,
    hst->max_attempts,
    NULL);

  /* bail out if we shouldn't be running event handlers */
  if (config->enable_event_handlers() == false)
    return (OK);
  if (hst->event_handler_enabled == false)
    return (OK);

  /* update host macros */
  memset(&mac, 0, sizeof(mac));
  grab_host_macros_r(&mac, hst);

  /* run the global host event handler */
  run_global_host_event_handler(&mac, hst);

  /* run the event handler command if there is one */
  if (hst->event_handler != NULL)
    run_host_event_handler(&mac, hst);

  /* send data to event broker */
  broker_external_command(
    NEBTYPE_EXTERNALCOMMAND_CHECK,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    time(NULL),
    NULL,
    NULL,
    NULL);

  return (OK);
}

/* runs the global host event handler */
int run_global_host_event_handler(nagios_macros* mac, host* hst) {
  char* raw_command = NULL;
  char* processed_command = NULL;
  char* processed_logentry = NULL;
  char* command_output = NULL;
  int early_timeout = false;
  double exectime = 0.0;
  int result = 0;
  struct timeval start_time;
  struct timeval end_time;
  int neb_result = OK;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;

  logger(dbg_functions, basic)
    << "run_global_host_event_handler()";

  if (hst == NULL)
    return (ERROR);

  /* bail out if we shouldn't be running event handlers */
  if (config->enable_event_handlers() == false)
    return (OK);

  /* no global host event handler command is defined */
  if (config->global_host_event_handler() == "")
    return (ERROR);

  logger(dbg_eventhandlers, more)
    << "Running global event handler for host '" << hst->name << "'...";

  /* get start time */
  gettimeofday(&start_time, NULL);

  /* get the raw command line */
  get_raw_command_line_r(
    mac,
    global_host_event_handler_ptr,
    config->global_host_event_handler().c_str(),
    &raw_command,
    macro_options);
  if (raw_command == NULL)
    return (ERROR);

  logger(dbg_eventhandlers, most)
    << "Raw global host event handler command line: " << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(
    mac,
    raw_command,
    &processed_command,
    macro_options);
  if (processed_command == NULL)
    return (ERROR);

  logger(dbg_eventhandlers, most)
    << "Processed global host event handler "
    "command line: " << processed_command;

  if (config->log_event_handlers() == true) {
    std::ostringstream oss;
    oss << "GLOBAL HOST EVENT HANDLER: " << hst->name
	<< "$HOSTSTATE$;$HOSTSTATETYPE$;$HOSTATTEMPT$;"
	<< config->global_host_event_handler();
    process_macros_r(
      mac,
      oss.str().c_str(),
      &processed_logentry,
      macro_options);
    logger(log_event_handler, basic)
      << processed_logentry;
  }

  /* send event data to broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_event_handler(
                 NEBTYPE_EVENTHANDLER_START,
                 NEBFLAG_NONE,
                 NEBATTR_NONE,
                 GLOBAL_HOST_EVENTHANDLER,
                 (void*)hst,
                 hst->current_state,
                 hst->state_type,
                 start_time,
                 end_time,
                 exectime,
                 config->event_handler_timeout(),
                 early_timeout, result,
                 config->global_host_event_handler().c_str(),
                 processed_command,
                 NULL,
                 NULL);

  /* neb module wants to override (or cancel) the event handler - perhaps it will run the eventhandler itself */
  if ((neb_result == NEBERROR_CALLBACKCANCEL)
      || (neb_result == NEBERROR_CALLBACKOVERRIDE)) {
    delete[] processed_command;
    delete[] raw_command;
    delete[] processed_logentry;
    return ((neb_result == NEBERROR_CALLBACKCANCEL) ? ERROR : OK);
  }

  /* run the command */
  try {
    result = my_system_r(
               mac,
               processed_command,
               config->event_handler_timeout(),
               &early_timeout,
               &exectime,
               &command_output,
               0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute global host event handler command line '"
      << processed_command << "' : " << e.what();
  }

  /* check for a timeout in the execution of the event handler command */
  if (early_timeout == true)
    logger(log_event_handler | log_runtime_warning, basic)
      << "Warning: Global host event handler command '"
      << processed_command << "' timed out after "
      << config->event_handler_timeout() << " seconds";

  /* get end time */
  gettimeofday(&end_time, NULL);

  /* send event data to broker */
  broker_event_handler(
    NEBTYPE_EVENTHANDLER_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    GLOBAL_HOST_EVENTHANDLER,
    (void*)hst,
    hst->current_state,
    hst->state_type,
    start_time,
    end_time,
    exectime,
    config->event_handler_timeout(),
    early_timeout,
    result,
    config->global_host_event_handler().c_str(),
    processed_command,
    command_output,
    NULL);

  /* free memory */
  delete[] command_output;
  delete[] raw_command;
  delete[] processed_command;
  delete[] processed_logentry;

  return (OK);
}

/* runs a host event handler command */
int run_host_event_handler(nagios_macros* mac, host* hst) {
  char* raw_command = NULL;
  char* processed_command = NULL;
  char* processed_logentry = NULL;
  char* command_output = NULL;
  int early_timeout = false;
  double exectime = 0.0;
  int result = 0;
  struct timeval start_time;
  struct timeval end_time;
  int neb_result = OK;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;

  logger(dbg_functions, basic)
    << "run_host_event_handler()";

  if (hst == NULL)
    return (ERROR);

  /* bail if there's no command */
  if (hst->event_handler == NULL)
    return (ERROR);

  logger(dbg_eventhandlers, more)
    << "Running event handler for host '" << hst->name << "'...";

  /* get start time */
  gettimeofday(&start_time, NULL);

  /* get the raw command line */
  get_raw_command_line_r(
    mac,
    hst->event_handler_ptr,
    hst->event_handler,
    &raw_command,
    macro_options);
  if (raw_command == NULL)
    return (ERROR);

  logger(dbg_eventhandlers, most)
    << "Raw host event handler command line: " << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(
    mac,
    raw_command,
    &processed_command,
    macro_options);
  if (processed_command == NULL)
    return (ERROR);

  logger(dbg_eventhandlers, most)
    << "Processed host event handler command line: "
    << processed_command;

  if (config->log_event_handlers() == true) {
    std::ostringstream oss;
    oss << "HOST EVENT HANDLER: " << hst->name
	<< ";$HOSTSTATE$;$HOSTSTATETYPE$;$HOSTATTEMPT$;"
	<< hst->event_handler;
    process_macros_r(
      mac,
      oss.str().c_str(),
      &processed_logentry,
      macro_options);
    logger(log_event_handler, basic)
      << processed_logentry;
  }

  /* send event data to broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_event_handler(
                 NEBTYPE_EVENTHANDLER_START,
                 NEBFLAG_NONE,
                 NEBATTR_NONE,
                 HOST_EVENTHANDLER,
                 (void*)hst,
                 hst->current_state,
                 hst->state_type,
                 start_time,
                 end_time,
                 exectime,
                 config->event_handler_timeout(),
                 early_timeout,
                 result,
                 hst->event_handler,
                 processed_command,
                 NULL,
                 NULL);

  /* neb module wants to override (or cancel) the event handler - perhaps it will run the eventhandler itself */
  if ((neb_result == NEBERROR_CALLBACKCANCEL)
      || (neb_result == NEBERROR_CALLBACKOVERRIDE)) {
    delete[] processed_command;
    delete[] raw_command;
    delete[] processed_logentry;
    return ((neb_result == NEBERROR_CALLBACKCANCEL) ? ERROR : OK);
  }

  /* run the command */
  try {
    result = my_system_r(
               mac,
               processed_command,
               config->event_handler_timeout(),
               &early_timeout,
               &exectime,
               &command_output,
               0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: can't execute host event handler command line '"
      << processed_command << "' : " << e.what();
  }

  /* check to see if the event handler timed out */
  if (early_timeout == true)
    logger(log_event_handler | log_runtime_warning, basic)
      << "Warning: Host event handler command '" << processed_command
      << "' timed out after " << config->event_handler_timeout()
      << " seconds";

  /* get end time */
  gettimeofday(&end_time, NULL);

  /* send event data to broker */
  broker_event_handler(
    NEBTYPE_EVENTHANDLER_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_EVENTHANDLER,
    (void*)hst,
    hst->current_state,
    hst->state_type,
    start_time,
    end_time,
    exectime,
    config->event_handler_timeout(),
    early_timeout,
    result,
    hst->event_handler,
    processed_command,
    command_output,
    NULL);

  /* free memory */
  delete[] command_output;
  delete[] raw_command;
  delete[] processed_command;
  delete[] processed_logentry;

  return (OK);
}

/******************************************************************/
/****************** HOST STATE HANDLER FUNCTIONS ******************/
/******************************************************************/

/* top level host state handler - occurs after every host check (soft/hard and active/passive) */
int handle_host_state(host* hst) {
  int state_change = false;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "handle_host_state()";

  /* get current time */
  time(&current_time);

  /* obsess over this host check */
  obsessive_compulsive_host_check_processor(hst);

  /* update performance data */
  update_host_performance_data(hst);

  /* record latest time for current state */
  switch (hst->current_state) {
  case HOST_UP:
    hst->last_time_up = current_time;
    break;

  case HOST_DOWN:
    hst->last_time_down = current_time;
    break;

  case HOST_UNREACHABLE:
    hst->last_time_unreachable = current_time;
    break;

  default:
    break;
  }

  /* has the host state changed? */
  if (hst->last_state != hst->current_state
      || hst->last_hard_state != hst->current_state
      || (hst->current_state == HOST_UP
          && hst->state_type == SOFT_STATE))
    state_change = true;

  /* if the host state has changed... */
  if (state_change == true) {

    /* update last state change times */
    if (hst->state_type == SOFT_STATE
        || hst->last_state != hst->current_state)
      hst->last_state_change = current_time;
    if (hst->state_type == HARD_STATE)
      hst->last_hard_state_change = current_time;

    /* update the event id */
    hst->last_event_id = hst->current_event_id;
    hst->current_event_id = next_event_id;
    next_event_id++;

    /* update the problem id when transitioning to a problem state */
    if (hst->last_state == HOST_UP) {
      /* don't reset last problem id, or it will be zero the next time a problem is encountered */
      /*hst->last_problem_id=hst->current_problem_id; */
      hst->current_problem_id = next_problem_id;
      next_problem_id++;
    }

    /* clear the problem id when transitioning from a problem state to an UP state */
    if (hst->current_state == HOST_UP) {
      hst->last_problem_id = hst->current_problem_id;
      hst->current_problem_id = 0L;
    }

    /* reset the acknowledgement flag if necessary */
    if (hst->acknowledgement_type == ACKNOWLEDGEMENT_NORMAL) {

      hst->problem_has_been_acknowledged = false;
      hst->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

      /* remove any non-persistant comments associated with the ack */
      delete_host_acknowledgement_comments(hst);
    }
    else if (hst->acknowledgement_type == ACKNOWLEDGEMENT_STICKY
             && hst->current_state == HOST_UP) {

      hst->problem_has_been_acknowledged = false;
      hst->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

      /* remove any non-persistant comments associated with the ack */
      delete_host_acknowledgement_comments(hst);
    }

    /* reset the next and last notification times */
    hst->last_host_notification = (time_t)0;
    hst->next_host_notification = (time_t)0;

    /* reset notification suppression option */
    hst->no_more_notifications = false;

    /* write the host state change to the main log file */
    if (hst->state_type == HARD_STATE
        || (hst->state_type == SOFT_STATE
            && config->log_host_retries() == true))
      log_host_event(hst);

    /* check for start of flexible (non-fixed) scheduled downtime */
    /* CHANGED 08-05-2010 EG flex downtime can now start on soft states */
    /*if(hst->state_type==HARD_STATE) */
    check_pending_flex_host_downtime(hst);

    /* notify contacts about the recovery or problem if its a "hard" state */
    if (hst->state_type == HARD_STATE)
      host_notification(
        hst,
        NOTIFICATION_NORMAL,
        NULL,
        NULL,
        NOTIFICATION_OPTION_NONE);

    /* handle the host state change */
    handle_host_event(hst);

    /* reset the initial notification time external parameter only if we changed from an UP to a non-UP state */
    /* (and also to an UP state from a non-UP state, but this particular is managed below) */
    if(hst->current_state == HOST_DOWN)
      host_other_props[hst->name].initial_notif_time = 0;

    /* the host just recovered, so reset the current host attempt */
    if (hst->current_state == HOST_UP)
      hst->current_attempt = 1;

    /* the host recovered, so reset the current notification number and state flags (after the recovery notification has gone out) */
    if (hst->current_state == HOST_UP) {
      hst->current_notification_number = 0;
      hst->notified_on_down = false;
      hst->notified_on_unreachable = false;
      host_other_props[hst->name].initial_notif_time = 0;
    }
  }

  /* else the host state has not changed */
  else {

    /* notify contacts if host is still down or unreachable */
    if (hst->current_state != HOST_UP
        && hst->state_type == HARD_STATE)
      host_notification(
        hst,
        NOTIFICATION_NORMAL,
        NULL,
        NULL,
        NOTIFICATION_OPTION_NONE);

    /* if we're in a soft state and we should log host retries, do so now... */
    if (hst->state_type == SOFT_STATE
        && config->log_host_retries() == true)
      log_host_event(hst);
  }

  return (OK);
}
