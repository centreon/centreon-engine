/*
** Copyright 2011 Merethis
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

#include <string>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include "globals.hh"
#include "objects.hh"
#include "macros.hh"
#include "common.hh"
#include "statusdata.hh"
#include "logging/file.hh"
#include "logging/logger.hh"
#include "logging/object.hh"
#include "compatibility/logging.h"

// using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

static const unsigned int BUFFER_SIZE = 4096;

/**
 *  The main logging function.
 *  This function has been DEPRECATED.
 *
 *  @param[in] type    Logging types.
 *  @param[in] display Unused.
 *  @param[in] fmt     A format who described the output.
 *  @param[in] ...     Describe the variable argument list here.
 */
void logit(int type, int display, char const* fmt, ...) {
  (void)display;

  char buffer[BUFFER_SIZE];
  va_list ap;

  va_start(ap, fmt);
  if (vsnprintf(buffer, sizeof(buffer), fmt, ap) > 0) {
    logger(type, basic) << buffer;
  }
  va_end(ap);
}

/**
 *  The main debug logging function.
 *  This function has been DEPRECATED.
 *
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 *  @param[in] fmt       A format who described the output.
 *  @param[in] ...       Describe the variable argument list here.
 *
 *  @return Return true.
 */
int log_debug_info(int type, unsigned int verbosity, char const* fmt, ...) {
  char buffer[BUFFER_SIZE];
  va_list ap;

  va_start(ap, fmt);
  if (vsnprintf(buffer, sizeof(buffer), fmt, ap) > 0) {
    timeval now;
    if (gettimeofday(&now, NULL) == -1) {
      now.tv_sec = 0;
      now.tv_usec = 0;
    }

    if (verbosity > most) {
      verbosity = most;
    }

    logger(static_cast<unsigned long long>(type) << 32, verbosity)
      << "[" << now.tv_sec << "." << now.tv_usec << "] "
      << "[" << type << "." << verbosity << "] "
      << "[pid=" << getpid() << "] " << buffer;
  }
  va_end(ap);

  return (OK);
}

/**
 *  Write message into all type of logging objects.
 *  This function has been DEPRECATED.
 *
 *  @param[in] buffer    The message to log.
 *  @param[in] type      Logging types.
 *
 *  @return Return true.
 */
int write_to_all_logs(char const* buffer, unsigned long type) {
  if (buffer != NULL) {
    logger(type, basic) << buffer;
  }
  return (OK);
}

/**
 *  Write message into all type of logging objects.
 *  This function has been DEPRECATED.
 *
 *  @param[in] buffer    The message to log.
 *  @param[in] type      Logging types.
 *  @param[in] timestamp Unused.
 *
 *  @return Return true.
 */
int write_to_log(char const* buffer, unsigned long type, time_t* timestamp) {
  (void)timestamp;

  if (buffer != NULL) {
    logger(type, basic) << buffer;
  }
  return (OK);
}

/**
 *  Write message into all type of logging objects.
 *  This function has been DEPRECATED.
 *
 *  @param[in] buffer    The message to log.
 *  @param[in] type      Logging types.
 *
 *  @return Return true.
 */
int write_to_syslog(char const* buffer, unsigned long type) {
  if (buffer != NULL) {
    logger(type, basic) << buffer;
  }
  return (OK);
}

/**
 *  Log service event information.
 *  This function has been DEPRECATED.
 *
 *  @param[in] svc The service to log.
 *
 *  @return Return true on success.
 */
int log_service_event(service* svc) {
  if (svc->state_type == SOFT_STATE && !config.get_log_service_retries()) {
    return (OK);
  }

  if (svc->host_ptr == NULL
      || svc->host_name == NULL
      || svc->description == NULL) {
    return (ERROR);
  }

  unsigned long log_options = NSLOG_SERVICE_OK;
  unsigned long nslog_options[] = {
    NSLOG_SERVICE_OK,
    NSLOG_SERVICE_WARNING,
    NSLOG_SERVICE_CRITICAL,
    NSLOG_SERVICE_UNKNOWN
  };

  if (svc->current_state > 0
      && (unsigned int)svc->current_state < sizeof(nslog_options) / sizeof(*nslog_options)) {
    log_options = nslog_options[svc->current_state];
  }

  nagios_macros mac;
  memset(&mac, 0, sizeof(mac));
  grab_host_macros(&mac, svc->host_ptr);
  grab_service_macros(&mac, svc);

  std::string buffer = "SERVICE ALERT: " + std::string(svc->host_name) + ";"
    + svc->description + ";$SERVICESTATE$;$SERVICESTATETYPE$;$SERVICEATTEMPT$;"
    + (svc->plugin_output ? svc->plugin_output : "") + "\n";

  char* processed_buffer = NULL;
  process_macros_r(&mac, buffer.c_str(), &processed_buffer, 0);
  clear_host_macros(&mac);
  clear_service_macros(&mac);

  logger(log_options, basic) << processed_buffer;

  delete[] processed_buffer;

  return (OK);
}

/**
 *  Log host event information.
 *  This function has been DEPRECATED.
 *
 *  @param[in] hst The host to log.
 *
 *  @return Return true on success.
 */
int log_host_event(host* hst) {
  if (hst == NULL || hst->name == NULL) {
    return (ERROR);
  }

  nagios_macros mac;
  memset(&mac, 0, sizeof(mac));
  grab_host_macros(&mac, hst);

  unsigned long log_options = NSLOG_HOST_UP;
  unsigned long nslog_options[] = {
    NSLOG_HOST_UP,
    NSLOG_HOST_DOWN,
    NSLOG_HOST_UNREACHABLE
  };

  if (hst->current_state > 0
      && (unsigned int)hst->current_state < sizeof(nslog_options) / sizeof(*nslog_options)) {
    log_options = nslog_options[hst->current_state];
  }

  std::string buffer = "HOST ALERT: " + std::string(hst->name)
    + ";$HOSTSTATE$;$HOSTSTATETYPE$;$HOSTATTEMPT$;"
    + (hst->plugin_output ? hst->plugin_output : "") + "\n";

  char* processed_buffer = NULL;
  process_macros_r(&mac, buffer.c_str(), &processed_buffer, 0);

  logger(log_options, basic) << processed_buffer;

  clear_host_macros(&mac);
  delete[] processed_buffer;

  return (OK);
}

/**
 *  Log host states information.
 *  This function has been DEPRECATED.
 *
 *  @param[in] type      State logging types.
 *  @param[in] timestamp Unused.
 *
 *  @return Return true on success.
 */
int log_host_states(unsigned int type, time_t* timestamp) {
  (void)timestamp;

  if (type == INITIAL_STATES && config.get_log_initial_state() == false) {
    return (OK);
  }

  nagios_macros mac;
  memset(&mac, 0, sizeof(mac));
  for (host* host = host_list;
       host != NULL;
       host = host->next) {
    if (host->name == NULL) {
      continue;
    }

    grab_host_macros(&mac, host);

    std::string buffer = (type == INITIAL_STATES ? "INITIAL" : "CURRENT")
      + std::string(" HOST STATE: ") + std::string(host->name)
      + ";$HOSTSTATE$;$HOSTSTATETYPE$;$HOSTATTEMPT$;"
      + (host->plugin_output ? host->plugin_output : "") + "\n";

    char* processed_buffer = NULL;
    process_macros_r(&mac, buffer.c_str(), &processed_buffer, 0);

    logger(log_info_message, basic) << processed_buffer;

    clear_host_macros(&mac);
    delete[] processed_buffer;
  }

  return (OK);
}

/**
 *  Log service states information.
 *  This function has been DEPRECATED.
 *
 *  @param[in] type      State logging types.
 *  @param[in] timestamp Unused.
 *
 *  @return Return true on success.
 */
int log_service_states(unsigned int type, time_t* timestamp) {
  (void)timestamp;

  if (type == INITIAL_STATES && config.get_log_initial_state() == false) {
    return (OK);
  }

  nagios_macros mac;
  memset(&mac, 0, sizeof(mac));
  for (service* service = service_list;
       service != NULL;
       service = service->next) {

    if (service->host_ptr == NULL
	|| service->host_name == NULL
	|| service->description == NULL) {
      continue;
    }

    grab_host_macros(&mac, service->host_ptr);
    grab_service_macros(&mac, service);

    std::string buffer = (type == INITIAL_STATES ? "INITIAL" : "CURRENT")
      + std::string(" SERVICE STATE: ") + std::string(service->host_name)
      + ';' + service->description
      + ";$SERVICESTATE$;$SERVICESTATETYPE$;$SERVICEATTEMPT$;"
      + (service->plugin_output != NULL ? service->plugin_output : "") + "\n";

    char* processed_buffer = NULL;
    process_macros_r(&mac, buffer.c_str(), &processed_buffer, 0);

    logger(log_info_message, basic) << processed_buffer;

    clear_host_macros(&mac);
    clear_service_macros(&mac);
    delete[] processed_buffer;
  }

  return (OK);
}

/**
 *  Archive logging files.
 *  This function has been DEPRECATED.
 *
 *  @param[in] rotation_time Unused.
 *
 *  @return Return true on success.
 */
int rotate_log_file(time_t rotation_time) {
  (void)rotation_time;

  // update the last log rotation time and status log.
  last_log_rotation = time(NULL);
  update_program_status(FALSE);

  file::rotate_all();
  return (OK);
}

/**
 *  Write the log version into log objects.
 *  This function has been DEPRECATED.
 *
 *  @param[in] timestamp Unused.
 *
 *  @return Return true on success.
 */
int write_log_file_info(time_t* timestamp) {
  (void)timestamp;

  logger(log_process_info, basic)
    <<  "LOG VERSION: " << LOG_VERSION_2 << "\n";
  return (OK);
}

/**
 *  Do nothing.
 *  This function has been DEPRECATED.
 *
 *  @return Return true on success.
 */
int open_debug_log(void) {
  return (OK);
}

/**
 *  Do nothing.
 *  This function has been DEPRECATED.
 *
 *  @return Return true on success.
 */
int close_debug_log(void) {
  return (OK);
}

