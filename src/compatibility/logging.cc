/*
** Copyright 2011-2013,2017 Centreon
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

#include <cstdarg>
#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/logging/file.hh"

// using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

static const unsigned int BUFFER_SIZE = 4096;

static char const* tab_state_type[] = {
  "SOFT",
  "HARD"
};

static char const* tab_initial_state[] = {
  "UNKNOWN",
  "INITIAL",
  "CURRENT"
};

static struct {
  unsigned long id;
  char const* str;
} tab_host_states[] = {
  { NSLOG_HOST_UP,          "UP"          },
  { NSLOG_HOST_DOWN,        "DOWN"        },
  { NSLOG_HOST_UNREACHABLE, "UNREACHABLE" }
};

static struct {
  unsigned long id;
  char const* str;
} tab_service_states[] = {
  { NSLOG_SERVICE_OK,       "OK"       },
  { NSLOG_SERVICE_WARNING,  "WARNING"  },
  { NSLOG_SERVICE_CRITICAL, "CRITICAL" }
};

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
  return;
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
int log_debug_info(
      int type,
      unsigned int verbosity,
      char const* fmt,
      ...) {
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
int write_to_log(
      char const* buffer,
      unsigned long type,
      time_t* timestamp) {
  (void)timestamp;

  if (buffer)
    logger(type, basic) << buffer;
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
  if (buffer)
    logger(type, basic) << buffer;
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
int log_service_event(service const* svc) {
  if (svc->state_type == SOFT_STATE
      && !config->log_service_retries())
    return (OK);

  if (!svc->host_ptr || !svc->host_name || !svc->description)
    return (ERROR);

  unsigned long log_options(NSLOG_SERVICE_UNKNOWN);
  char const* state("UNKNOWN");
  if (svc->current_state >= 0
      && (unsigned int)svc->current_state
      < sizeof(tab_service_states) / sizeof(*tab_service_states)) {
    log_options = tab_service_states[svc->current_state].id;
    state = tab_service_states[svc->current_state].str;
  }
  char const* state_type(tab_state_type[svc->state_type]);
  char const* output(svc->plugin_output ? svc->plugin_output : "");

  logger(log_options, basic)
    << "SERVICE ALERT: " << svc->host_name << ";" << svc->description
    << ";" << state << ";" << state_type << ";" << svc->current_attempt
    << ";" << output;
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
int log_host_event(com::centreon::engine::host const* hst) {
  if (!hst)
    return (ERROR);

  unsigned long log_options(NSLOG_HOST_UP);
  char const* state("UP");
  if (hst->get_current_state() > 0
      && (unsigned int)hst->get_current_state()
      < sizeof(tab_host_states) / sizeof(*tab_host_states)) {
    log_options = tab_host_states[hst->get_current_state()].id;
    state = tab_host_states[hst->get_current_state()].str;
  }
  char const* state_type(tab_state_type[hst->get_state_type()]);

  logger(log_options, basic)
    << "HOST ALERT: " << hst->get_name() << ";" << state << ";"
    << state_type << ";" << hst->get_current_attempt() << ";"
    << hst->get_plugin_output();

  return (OK);
}

/**
 *  Log host state information.
 *
 *  @param[in] type  State logging type.
 *  @param[in] hst   Host object.
 */
void log_host_state(unsigned int type, com::centreon::engine::host* hst) {
  char const* type_str(tab_initial_state[type]);
  char const* state("UP");
  if ((hst->get_current_state() > 0)
      && ((unsigned int)hst->get_current_state()
          < sizeof(tab_host_states) / sizeof(*tab_host_states)))
    state = tab_host_states[hst->get_current_state()].str;
  char const* state_type(tab_state_type[hst->get_state_type()]);
  logger(log_info_message, basic)
    << type_str << " HOST STATE: " << hst->get_name() << ";" << state
    << ";" << state_type << ";" << hst->get_current_attempt() << ";"
    << hst->get_plugin_output();
  return ;
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
  for (com::centreon::engine::host* hst(host_list); hst; hst = hst->next)
    log_host_state(type, hst);
  return (OK);
}

/**
 *  Log service state information.
 *
 *  @param[in] type  State logging type.
 *  @param[in] svc   Service object.
 */
void log_service_state(unsigned int type, service* svc) {
  if (svc->host_name && svc->description) {
    char const* type_str(tab_initial_state[type]);
    char const* state("UNKNOWN");
    if ((svc->current_state >= 0)
        && ((unsigned int)svc->current_state
            < sizeof(tab_service_states) / sizeof(*tab_service_states)))
      state = tab_service_states[svc->current_state].str;
    char const* state_type(tab_state_type[svc->state_type]);
    char const* output(svc->plugin_output ? svc->plugin_output : "");
    logger(log_info_message, basic)
      << type_str << " SERVICE STATE: " << svc->host_name << ";"
      << svc->description << ";" << state << ";" << state_type
      << ";" << svc->current_attempt << ";" << output;
  }
  return ;
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
  for (service* svc(service_list); svc; svc = svc->next)
    log_service_state(type, svc);
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
  return (ERROR);
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
    <<  "LOG VERSION: " << LOG_VERSION_2;
  return (OK);
}

/**
 *  Do nothing.
 *  This function has been DEPRECATED.
 *
 *  @return Return true on success.
 */
int open_debug_log() {
  return (OK);
}

/**
 *  Do nothing.
 *  This function has been DEPRECATED.
 *
 *  @return Return true on success.
 */
int close_debug_log() {
  return (OK);
}
