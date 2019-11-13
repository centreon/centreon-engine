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

#include "com/centreon/engine/sehandlers.hh"
#include <sstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/downtimes/downtime.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::logging;

/******************************************************************/
/************* OBSESSIVE COMPULSIVE HANDLER FUNCTIONS *************/
/******************************************************************/

/* handles host check results in an obsessive compulsive manner... */
int obsessive_compulsive_host_check_processor(
    com::centreon::engine::host* hst) {
  std::string raw_command;
  std::string processed_command;
  int early_timeout = false;
  double exectime = 0.0;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  nagios_macros mac;

  logger(dbg_functions, basic) << "obsessive_compulsive_host_check_processor()";

  if (hst == nullptr)
    return ERROR;

  /* bail out if we shouldn't be obsessing */
  if (!config->obsess_over_hosts())
    return OK;
  if (!hst->get_obsess_over())
    return OK;

  /* if there is no valid command, exit */
  if (config->ochp_command().empty())
    return ERROR;

  /* update macros */
  grab_host_macros_r(&mac, hst);

  /* get the raw command line */
  get_raw_command_line_r(&mac, ochp_command_ptr, config->ochp_command().c_str(),
                         raw_command, macro_options);
  if (raw_command.empty()) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_checks, most)
      << "Raw obsessive compulsive host processor command line: "
      << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(&mac, raw_command, processed_command, macro_options);
  if (processed_command.empty()) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_checks, most) << "Processed obsessive compulsive host processor "
                              "command line: "
                           << processed_command;

  /* run the command */
  try {
    std::string tmp;
    my_system_r(&mac, processed_command, config->ochp_timeout(), &early_timeout,
                &exectime, tmp, 0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
        << "Error: can't execute compulsive host processor command line '"
        << processed_command << "' : " << e.what();
  }
  clear_volatile_macros_r(&mac);

  /* check to see if the command timed out */
  if (early_timeout == true)
    logger(log_runtime_warning, basic)
        << "Warning: OCHP command '" << processed_command << "' for host '"
        << hst->get_name() << "' timed out after " << config->ochp_timeout()
        << " seconds";

  return OK;
}

/******************************************************************/
/**************** SERVICE EVENT HANDLER FUNCTIONS *****************/
/******************************************************************/

/* runs the global service event handler */
int run_global_service_event_handler(nagios_macros* mac,
                                     com::centreon::engine::service* svc) {
  std::string raw_command;
  std::string processed_command;
  std::string processed_logentry;
  std::string command_output;
  int early_timeout = false;
  double exectime = 0.0;
  int result = 0;
  struct timeval start_time;
  struct timeval end_time;
  int neb_result = OK;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;

  logger(dbg_functions, basic) << "run_global_service_event_handler()";

  if (svc == nullptr)
    return ERROR;

  /* bail out if we shouldn't be running event handlers */
  if (config->enable_event_handlers() == false)
    return OK;

  /* a global service event handler command has not been defined */
  if (config->global_service_event_handler().empty())
    return ERROR;

  logger(dbg_eventhandlers, more)
      << "Running global event handler for service '" << svc->get_description()
      << "' on host '" << svc->get_hostname() << "'...";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* get the raw command line */
  get_raw_command_line_r(mac, global_service_event_handler_ptr,
                         config->global_service_event_handler().c_str(),
                         raw_command, macro_options);
  if (raw_command.empty()) {
    return ERROR;
  }

  logger(dbg_eventhandlers, most)
      << "Raw global service event handler command line: " << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(mac, raw_command, processed_command, macro_options);
  if (processed_command.empty())
    return ERROR;

  logger(dbg_eventhandlers, most) << "Processed global service event handler "
                                     "command line: "
                                  << processed_command;

  if (config->log_event_handlers() == true) {
    std::ostringstream oss;
    oss << "GLOBAL SERVICE EVENT HANDLER: " << svc->get_hostname() << ';'
        << svc->get_description()
        << ";$SERVICESTATE$;$SERVICESTATETYPE$;$SERVICEATTEMPT$;"
        << config->global_service_event_handler();
    process_macros_r(mac, oss.str(), processed_logentry, macro_options);
    logger(log_event_handler, basic) << processed_logentry;
  }

  /* send event data to broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_event_handler(
      NEBTYPE_EVENTHANDLER_START, NEBFLAG_NONE, NEBATTR_NONE,
      GLOBAL_SERVICE_EVENTHANDLER, (void*)svc, svc->get_current_state(),
      svc->get_state_type(), start_time, end_time, exectime,
      config->event_handler_timeout(), early_timeout, result,
      config->global_service_event_handler().c_str(),
      const_cast<char*>(processed_command.c_str()), nullptr, nullptr);

  /* neb module wants to override (or cancel) the event handler - perhaps it
   * will run the eventhandler itself */
  if ((neb_result == NEBERROR_CALLBACKCANCEL) ||
      (neb_result == NEBERROR_CALLBACKOVERRIDE)) {
    return (neb_result == NEBERROR_CALLBACKCANCEL) ? ERROR : OK;
  }

  /* run the command */
  try {
    result =
        my_system_r(mac, processed_command, config->event_handler_timeout(),
                    &early_timeout, &exectime, command_output, 0);
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
  gettimeofday(&end_time, nullptr);

  /* send event data to broker */
  broker_event_handler(NEBTYPE_EVENTHANDLER_END, NEBFLAG_NONE, NEBATTR_NONE,
                       GLOBAL_SERVICE_EVENTHANDLER, (void*)svc,
                       svc->get_current_state(), svc->get_state_type(),
                       start_time, end_time, exectime,
                       config->event_handler_timeout(), early_timeout, result,
                       config->global_service_event_handler().c_str(),
                       const_cast<char*>(processed_command.c_str()),
                       const_cast<char*>(command_output.c_str()), nullptr);

  return OK;
}

/* runs a service event handler command */
int run_service_event_handler(nagios_macros* mac,
                              com::centreon::engine::service* svc) {
  std::string raw_command;
  std::string processed_command;
  std::string processed_logentry;
  std::string command_output;
  int early_timeout = false;
  double exectime = 0.0;
  int result = 0;
  struct timeval start_time;
  struct timeval end_time;
  int neb_result = OK;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;

  logger(dbg_functions, basic) << "run_service_event_handler()";

  if (svc == nullptr)
    return ERROR;

  /* bail if there's no command */
  if (svc->get_event_handler().empty())
    return ERROR;

  logger(dbg_eventhandlers, more)
      << "Running event handler for service '" << svc->get_description()
      << "' on host '" << svc->get_hostname() << "'...";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* get the raw command line */
  get_raw_command_line_r(mac, svc->get_event_handler_ptr(),
                         svc->get_event_handler().c_str(), raw_command,
                         macro_options);
  if (raw_command.empty())
    return ERROR;

  logger(dbg_eventhandlers, most)
      << "Raw service event handler command line: " << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(mac, raw_command, processed_command, macro_options);
  if (processed_command.empty())
    return ERROR;

  logger(dbg_eventhandlers, most)
      << "Processed service event handler command line: " << processed_command;

  if (config->log_event_handlers() == true) {
    std::ostringstream oss;
    oss << "SERVICE EVENT HANDLER: " << svc->get_hostname() << ';'
        << svc->get_description()
        << ";$SERVICESTATE$;$SERVICESTATETYPE$;$SERVICEATTEMPT$;"
        << svc->get_event_handler();
    process_macros_r(mac, oss.str(), processed_logentry, macro_options);
    logger(log_event_handler, basic) << processed_logentry;
  }

  /* send event data to broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_event_handler(
      NEBTYPE_EVENTHANDLER_START, NEBFLAG_NONE, NEBATTR_NONE,
      SERVICE_EVENTHANDLER, (void*)svc, svc->get_current_state(),
      svc->get_state_type(), start_time, end_time, exectime,
      config->event_handler_timeout(), early_timeout, result,
      svc->get_event_handler().c_str(),
      const_cast<char*>(processed_command.c_str()), nullptr, nullptr);

  /* neb module wants to override (or cancel) the event handler - perhaps it
   * will run the eventhandler itself */
  if ((neb_result == NEBERROR_CALLBACKCANCEL) ||
      (neb_result == NEBERROR_CALLBACKOVERRIDE)) {
    return (neb_result == NEBERROR_CALLBACKCANCEL) ? ERROR : OK;
  }

  /* run the command */
  try {
    result =
        my_system_r(mac, processed_command, config->event_handler_timeout(),
                    &early_timeout, &exectime, command_output, 0);
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
  gettimeofday(&end_time, nullptr);

  /* send event data to broker */
  broker_event_handler(NEBTYPE_EVENTHANDLER_END, NEBFLAG_NONE, NEBATTR_NONE,
                       SERVICE_EVENTHANDLER, (void*)svc,
                       svc->get_current_state(), svc->get_state_type(),
                       start_time, end_time, exectime,
                       config->event_handler_timeout(), early_timeout, result,
                       svc->get_event_handler().c_str(),
                       const_cast<char*>(processed_command.c_str()),
                       const_cast<char*>(command_output.c_str()), nullptr);

  return OK;
}

/******************************************************************/
/****************** HOST EVENT HANDLER FUNCTIONS ******************/
/******************************************************************/

/* handles a change in the status of a host */
int handle_host_event(com::centreon::engine::host* hst) {
  nagios_macros mac;

  logger(dbg_functions, basic) << "handle_host_event()";

  if (hst == nullptr)
    return ERROR;

  /* send event data to broker */
  broker_statechange_data(
      NEBTYPE_STATECHANGE_END, NEBFLAG_NONE, NEBATTR_NONE, HOST_STATECHANGE,
      (void*)hst, hst->get_current_state(), hst->get_state_type(),
      hst->get_current_attempt(), hst->get_max_attempts(), nullptr);

  /* bail out if we shouldn't be running event handlers */
  if (!config->enable_event_handlers())
    return OK;
  if (!hst->get_event_handler_enabled())
    return OK;

  /* update host macros */
  grab_host_macros_r(&mac, hst);

  /* run the global host event handler */
  run_global_host_event_handler(&mac, hst);

  /* run the event handler command if there is one */
  if (!hst->get_event_handler().empty())
    run_host_event_handler(&mac, hst);

  /* send data to event broker */
  broker_external_command(NEBTYPE_EXTERNALCOMMAND_CHECK, NEBFLAG_NONE,
                          NEBATTR_NONE, CMD_NONE, time(nullptr), nullptr,
                          nullptr, nullptr);

  return OK;
}

/* runs the global host event handler */
int run_global_host_event_handler(nagios_macros* mac,
                                  com::centreon::engine::host* hst) {
  std::string raw_command;
  std::string processed_command;
  std::string processed_logentry;
  std::string command_output;
  int early_timeout = false;
  double exectime = 0.0;
  int result = 0;
  struct timeval start_time;
  struct timeval end_time;
  int neb_result = OK;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;

  logger(dbg_functions, basic) << "run_global_host_event_handler()";

  if (hst == nullptr)
    return ERROR;

  /* bail out if we shouldn't be running event handlers */
  if (config->enable_event_handlers() == false)
    return OK;

  /* no global host event handler command is defined */
  if (config->global_host_event_handler() == "")
    return ERROR;

  logger(dbg_eventhandlers, more)
      << "Running global event handler for host '" << hst->get_name() << "'...";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* get the raw command line */
  get_raw_command_line_r(mac, global_host_event_handler_ptr,
                         config->global_host_event_handler().c_str(),
                         raw_command, macro_options);
  if (raw_command.empty())
    return ERROR;

  logger(dbg_eventhandlers, most)
      << "Raw global host event handler command line: " << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(mac, raw_command, processed_command, macro_options);
  if (processed_command.empty())
    return ERROR;

  logger(dbg_eventhandlers, most) << "Processed global host event handler "
                                     "command line: "
                                  << processed_command;

  if (config->log_event_handlers() == true) {
    std::ostringstream oss;
    oss << "GLOBAL HOST EVENT HANDLER: " << hst->get_name()
        << "$HOSTSTATE$;$HOSTSTATETYPE$;$HOSTATTEMPT$;"
        << config->global_host_event_handler();
    process_macros_r(mac, oss.str(), processed_logentry, macro_options);
    logger(log_event_handler, basic) << processed_logentry;
  }

  /* send event data to broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_event_handler(
      NEBTYPE_EVENTHANDLER_START, NEBFLAG_NONE, NEBATTR_NONE,
      GLOBAL_HOST_EVENTHANDLER, (void*)hst, hst->get_current_state(),
      hst->get_state_type(), start_time, end_time, exectime,
      config->event_handler_timeout(), early_timeout, result,
      config->global_host_event_handler().c_str(),
      const_cast<char*>(processed_command.c_str()), nullptr, nullptr);

  /* neb module wants to override (or cancel) the event handler - perhaps it
   * will run the eventhandler itself */
  if ((neb_result == NEBERROR_CALLBACKCANCEL) ||
      (neb_result == NEBERROR_CALLBACKOVERRIDE)) {
    return (neb_result == NEBERROR_CALLBACKCANCEL) ? ERROR : OK;
  }

  /* run the command */
  try {
    result =
        my_system_r(mac, processed_command, config->event_handler_timeout(),
                    &early_timeout, &exectime, command_output, 0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
        << "Error: can't execute global host event handler command line '"
        << processed_command << "' : " << e.what();
  }

  /* check for a timeout in the execution of the event handler command */
  if (early_timeout == true)
    logger(log_event_handler | log_runtime_warning, basic)
        << "Warning: Global host event handler command '" << processed_command
        << "' timed out after " << config->event_handler_timeout()
        << " seconds";

  /* get end time */
  gettimeofday(&end_time, nullptr);

  /* send event data to broker */
  broker_event_handler(NEBTYPE_EVENTHANDLER_END, NEBFLAG_NONE, NEBATTR_NONE,
                       GLOBAL_HOST_EVENTHANDLER, (void*)hst,
                       hst->get_current_state(), hst->get_state_type(),
                       start_time, end_time, exectime,
                       config->event_handler_timeout(), early_timeout, result,
                       config->global_host_event_handler().c_str(),
                       const_cast<char*>(processed_command.c_str()),
                       const_cast<char*>(command_output.c_str()), nullptr);

  return OK;
}

/* runs a host event handler command */
int run_host_event_handler(nagios_macros* mac,
                           com::centreon::engine::host* hst) {
  std::string raw_command;
  std::string processed_command;
  std::string processed_logentry;
  std::string command_output;
  int early_timeout = false;
  double exectime = 0.0;
  int result = 0;
  struct timeval start_time;
  struct timeval end_time;
  int neb_result = OK;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;

  logger(dbg_functions, basic) << "run_host_event_handler()";

  if (hst == nullptr)
    return ERROR;

  /* bail if there's no command */
  if (hst->get_event_handler().empty())
    return ERROR;

  logger(dbg_eventhandlers, more)
      << "Running event handler for host '" << hst->get_name() << "'...";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* get the raw command line */
  get_raw_command_line_r(mac, hst->get_event_handler_ptr(),
                         hst->get_event_handler().c_str(), raw_command,
                         macro_options);
  if (raw_command.empty())
    return ERROR;

  logger(dbg_eventhandlers, most)
      << "Raw host event handler command line: " << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(mac, raw_command, processed_command, macro_options);
  if (processed_command.empty())
    return ERROR;

  logger(dbg_eventhandlers, most)
      << "Processed host event handler command line: " << processed_command;

  if (config->log_event_handlers() == true) {
    std::ostringstream oss;
    oss << "HOST EVENT HANDLER: " << hst->get_name()
        << ";$HOSTSTATE$;$HOSTSTATETYPE$;$HOSTATTEMPT$;"
        << hst->get_event_handler();
    process_macros_r(mac, oss.str(), processed_logentry, macro_options);
    logger(log_event_handler, basic) << processed_logentry;
  }

  /* send event data to broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_event_handler(
      NEBTYPE_EVENTHANDLER_START, NEBFLAG_NONE, NEBATTR_NONE, HOST_EVENTHANDLER,
      (void*)hst, hst->get_current_state(), hst->get_state_type(), start_time,
      end_time, exectime, config->event_handler_timeout(), early_timeout,
      result, hst->get_event_handler().c_str(),
      const_cast<char*>(processed_command.c_str()), nullptr, nullptr);

  /* neb module wants to override (or cancel) the event handler - perhaps it
   * will run the eventhandler itself */
  if ((neb_result == NEBERROR_CALLBACKCANCEL) ||
      (neb_result == NEBERROR_CALLBACKOVERRIDE)) {
    return (neb_result == NEBERROR_CALLBACKCANCEL) ? ERROR : OK;
  }

  /* run the command */
  try {
    result =
        my_system_r(mac, processed_command, config->event_handler_timeout(),
                    &early_timeout, &exectime, command_output, 0);
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
  gettimeofday(&end_time, nullptr);

  /* send event data to broker */
  broker_event_handler(NEBTYPE_EVENTHANDLER_END, NEBFLAG_NONE, NEBATTR_NONE,
                       HOST_EVENTHANDLER, (void*)hst, hst->get_current_state(),
                       hst->get_state_type(), start_time, end_time, exectime,
                       config->event_handler_timeout(), early_timeout, result,
                       hst->get_event_handler().c_str(),
                       const_cast<char*>(processed_command.c_str()),
                       const_cast<char*>(command_output.c_str()), nullptr);

  return OK;
}

/******************************************************************/
/****************** HOST STATE HANDLER FUNCTIONS ******************/
/******************************************************************/
