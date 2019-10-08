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
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/logging/file.hh"

// using namespace com::centreon::engine;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

static char const* tab_initial_state[] = {
  "UNKNOWN",
  "INITIAL",
  "CURRENT"
};

/**
 *  Log host state information.
 *
 *  @param[in] type  State logging type.
 *  @param[in] hst   Host object.
 */
void log_host_state(unsigned int type, com::centreon::engine::host* hst) {
  char const* type_str(tab_initial_state[type]);
  char const* state("UP");
  if (hst->get_current_state() > 0
      && (unsigned int)hst->get_current_state() < host::tab_host_states.size())
    state = host::tab_host_states[hst->get_current_state()].second.c_str();
  std::string const& state_type{host::tab_state_type[hst->get_state_type()]};
  logger(log_info_message, basic)
    << type_str << " HOST STATE: " << hst->get_name() << ";" << state
    << ";" << state_type << ";" << hst->get_current_attempt() << ";"
    << hst->get_plugin_output();
}

/**
 *  Log service state information.
 *
 *  @param[in] type  State logging type.
 *  @param[in] svc   Service object.
 */
void log_service_state(unsigned int type, com::centreon::engine::service* svc) {
  char const* type_str(tab_initial_state[type]);
  char const* state("UNKNOWN");
  if (svc->get_current_state() >= 0
      && (unsigned int)svc->get_current_state() < service::tab_service_states.size())
    state = service::tab_service_states[svc->get_current_state()].second.c_str();
  std::string const& state_type(service::tab_state_type[svc->get_state_type()]);
  std::string const& output{svc->get_plugin_output()};
  logger(log_info_message, basic)
    << type_str << " SERVICE STATE: " << svc->get_hostname() << ";"
    << svc->get_description() << ";" << state << ";" << state_type
    << ";" << svc->get_current_attempt() << ";" << output;
}
