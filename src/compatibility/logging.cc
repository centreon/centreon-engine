/*
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

#include <cstdarg>
#include <cstdio>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/host.hh"
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
  { log_host_up,          "UP"          },
  { log_host_down,        "DOWN"        },
  { log_host_unreachable, "UNREACHABLE" }
};

static struct {
  unsigned long id;
  char const* str;
} tab_service_states[] = {
  { log_service_ok,       "OK"       },
  { log_service_warning,  "WARNING"  },
  { log_service_critical, "CRITICAL" }
};

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

  unsigned long log_options(log_service_unknown);
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
int log_host_event(host const* hst) {
  if (!hst || !hst->name)
    return (ERROR);

  unsigned long log_options(log_host_up);
  char const* state("UP");
  if (hst->current_state > 0
      && (unsigned int)hst->current_state
      < sizeof(tab_host_states) / sizeof(*tab_host_states)) {
    log_options = tab_host_states[hst->current_state].id;
    state = tab_host_states[hst->current_state].str;
  }
  char const* state_type(tab_state_type[hst->state_type]);
  char const* output(hst->plugin_output ? hst->plugin_output : "");

  logger(log_options, basic)
    << "HOST ALERT: " << hst->name << ";" << state << ";"
    << state_type << ";" << hst->current_attempt << ";" << output;

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

  if (type == INITIAL_STATES && !config->log_initial_states())
    return (OK);

  char const* type_str(tab_initial_state[type]);
  for (host* hst = host_list; hst; hst = hst->next) {
    if (!hst->name)
      continue;

    char const* state("UP");
    if (hst->current_state > 0
        && (unsigned int)hst->current_state
        < sizeof(tab_host_states) / sizeof(*tab_host_states))
      state = tab_host_states[hst->current_state].str;

    char const* state_type(tab_state_type[hst->state_type]);
    char const* output(hst->plugin_output ? hst->plugin_output : "");

    logger(log_info_message, basic)
      << type_str << " HOST STATE: " << hst->name << ";" << state
      << ";" << state_type << ";" << hst->current_attempt << ";"
      << output;
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

  if (type == INITIAL_STATES && !config->log_initial_states())
    return (OK);

  char const* type_str(tab_initial_state[type]);
  for (service* svc = service_list; svc; svc = svc->next) {
    if (!svc->host_ptr || !svc->host_name || !svc->description)
      continue;

    char const* state("UNKNOWN");
    if (svc->current_state >= 0
        && (unsigned int)svc->current_state
        < sizeof(tab_service_states) / sizeof(*tab_service_states))
      state = tab_service_states[svc->current_state].str;

    char const* state_type(tab_state_type[svc->state_type]);
    char const* output(svc->plugin_output ? svc->plugin_output : "");

    logger(log_info_message, basic)
      << type_str << " SERVICE STATE: " << svc->host_name << ";"
      << svc->description << ";" << state << ";" << state_type
      << ";" << svc->current_attempt << ";" << output;
  }
  return (OK);
}
